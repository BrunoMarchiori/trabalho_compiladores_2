#include "codegen.hpp"
#include <sstream>
#include <algorithm>

// Mapeamento de builtins Scheme → Python (funções do scheme_runtime.py)
const std::map<std::string, std::string> CodeGen::BUILTINS = {
    {"+","_add"},{"-","_sub"},{"*","_mul"},{"/","_div"},
    {"=","_num_eq"},{"<","_lt"},{"<=","_le"},{">","_gt"},{">=","_ge"},
    {"eq?","_eq"},{"eqv?","_eqv"},{"equal?","_equal"},{"not","_not"},
    {"cons","_cons"},{"car","_car"},{"cdr","_cdr"},{"list","_list"},
    {"null?","_is_null"},{"pair?","_is_pair"},{"list?","_is_list"},
    {"length","_length"},{"append","_append"},{"reverse","_reverse"},
    {"list-ref","_list_ref"},{"list-tail","_list_tail"},
    {"member","_member"},{"memq","_memq"},{"memv","_memv"},
    {"assoc","_assoc"},{"assq","_assq"},{"assv","_assv"},
    {"caar","_caar"},{"cadr","_cadr"},{"cdar","_cdar"},{"cddr","_cddr"},
    {"caddr","_caddr"},{"cadddr","_cadddr"},{"caaar","_caaar"},{"caadr","_caadr"},
    {"list-copy","_list_copy"},
    {"map","_map"},{"for-each","_for_each"},{"apply","_apply"},
    {"filter","_filter"},{"fold-left","_fold_left"},{"fold-right","_fold_right"},
    {"display","_display"},{"newline","_newline"},{"write","_write"},
    {"read","_read"},{"read-char","_read_char"},{"write-char","_write_char"},
    {"error","_error"},{"void","_void"},
    {"number->string","_number_to_string"},{"string->number","_string_to_number"},
    {"string-length","_string_length"},{"string-ref","_string_ref"},
    {"string-append","_string_append"},{"substring","_substring"},
    {"string=?","_string_eq"},{"string<?","_string_lt"},{"string>?","_string_gt"},
    {"string<=?","_string_le"},{"string>=?","_string_ge"},
    {"string-ci=?","_string_ci_eq"},{"string-ci<?","_string_ci_lt"},
    {"string-ci>?","_string_ci_gt"},{"string-ci<=?","_string_ci_le"},
    {"string-ci>=?","_string_ci_ge"},
    {"string->symbol","_string_to_symbol"},{"symbol->string","_symbol_to_string"},
    {"string->list","_string_to_list"},{"list->string","_list_to_string"},
    {"string-upcase","_string_upcase"},{"string-downcase","_string_downcase"},
    {"string-copy","_string_copy"},{"string","_mk_string"},{"make-string","_make_string"},
    {"char->integer","_char_to_int"},{"integer->char","_int_to_char"},
    {"char=?","_char_eq"},{"char<?","_char_lt"},{"char>?","_char_gt"},
    {"char<=?","_char_le"},{"char>=?","_char_ge"},
    {"char-ci=?","_char_ci_eq"},{"char-ci<?","_char_ci_lt"},
    {"char-ci>?","_char_ci_gt"},{"char-ci<=?","_char_ci_le"},
    {"char-ci>=?","_char_ci_ge"},
    {"char-alphabetic?","_char_alpha"},
    {"char-numeric?","_char_num"},{"char-whitespace?","_char_ws"},
    {"char-upcase","_char_up"},{"char-downcase","_char_dn"},
    {"number?","_is_number"},{"integer?","_is_integer"},{"real?","_is_real"},
    {"rational?","_is_rational"},{"complex?","_is_complex"},
    {"exact?","_is_exact"},{"inexact?","_is_inexact"},
    {"numerator","_numerator"},{"denominator","_denominator"},
    {"exact->inexact","_exact_to_inexact"},{"inexact->exact","_inexact_to_exact"},
    {"string?","_is_string"},{"symbol?","_is_symbol"},{"boolean?","_is_boolean"},
    {"char?","_is_char"},{"procedure?","_is_procedure"},{"vector?","_is_vector"},
    {"port?","_is_port"},{"input-port?","_is_input_port"},{"output-port?","_is_output_port"},
    {"zero?","_is_zero"},{"positive?","_is_positive"},{"negative?","_is_negative"},
    {"odd?","_is_odd"},{"even?","_is_even"},
    {"abs","_abs"},{"max","_max"},{"min","_min"},
    {"floor","_floor"},{"ceiling","_ceiling"},{"truncate","_trunc"},{"round","_round"},
    {"sqrt","_sqrt"},{"expt","_expt"},{"modulo","_modulo"},
    {"remainder","_remainder"},{"quotient","_quotient"},{"gcd","_gcd"},{"lcm","_lcm"},
    {"make-vector","_make_vector"},{"vector","_vector"},
    {"vector-ref","_vector_ref"},{"vector-set!","_vector_set"},
    {"vector-length","_vector_length"},{"vector->list","_vector_to_list"},
    {"list->vector","_list_to_vector"},{"vector-fill!","_vector_fill"},
    {"force","_force"},{"values","_values"},
    {"call-with-values","_call_with_values"},
    {"call/cc","_call_cc"},{"call-with-current-continuation","_call_cc"},
    {"dynamic-wind","_dynamic_wind"},{"raise","_raise"},
    {"with-exception-handler","_with_exc_handler"},
    {"open-input-file","_open_in"},{"open-output-file","_open_out"},
    {"close-input-port","_close_in"},{"close-output-port","_close_out"},
    {"current-input-port","_cur_in"},{"current-output-port","_cur_out"},
    {"eof-object?","_is_eof"},
};

static const std::set<std::string> PY_KEYWORDS = {
    "False","None","True","and","as","assert","async","await",
    "break","class","continue","def","del","elif","else","except",
    "finally","for","from","global","if","import","in","is",
    "lambda","nonlocal","not","or","pass","raise","return",
    "try","while","with","yield"
};

std::string CodeGen::newTemp()   { return "_t"  + std::to_string(tempCount++);   }
std::string CodeGen::newLambda() { return "_lam_" + std::to_string(lambdaCount++); }

std::string CodeGen::pyId(const std::string& s) {
    auto it = BUILTINS.find(s);
    if (it != BUILTINS.end()) return it->second;

    std::string r;
    for (char c : s) {
        switch (c) {
            case '-': case '?': case '!': case '>':
            case '<': case '/': case '=': case ':':
            case '^': case '~': case '%': case '&':
            case '$': case '@': case '*': r += '_'; break;
            case '+':                    r += '_'; break;
            default:                     r += c;   break;
        }
    }
    while (!r.empty() && r.back() == '_') r.pop_back();
    while (!r.empty() && r.front() == '_') r.erase(r.begin());
    if (r.empty()) r = "_sym";
    if (PY_KEYWORDS.count(r)) r = "_" + r + "_";
    return r;
}

bool CodeGen::lookupBoxedAlias(const std::string& name, std::string& box) const {
    for (int i = (int)boxedAliases.size() - 1; i >= 0; i--) {
        auto it = boxedAliases[i].find(name);
        if (it != boxedAliases[i].end()) {
            box = it->second;
            return !box.empty();
        }
    }
    return false;
}

std::string CodeGen::ind(int level) {
    return std::string(level * 4, ' ');
}

void CodeGen::collectMutations(Node* n, std::set<std::string>& out) {
    if (!n) return;
    if (n->kind != NK_PAIR) return;
    if (isSymbol(n->car, "set!")) {
        auto items = listToVec(n);
        if (items.size() >= 2 && items[1]->kind == NK_SYMBOL)
            out.insert(items[1]->sval);
        return;
    }
    // Não descemos em lambdas/defines aninhados
    if (isSymbol(n->car, "lambda") || isSymbol(n->car, "define")) return;
    collectMutations(n->car, out);
    collectMutations(n->cdr, out);
}

void CodeGen::collectLocalDefs(const std::vector<Node*>& body, std::set<std::string>& out) {
    for (Node* stmt : body) {
        if (!stmt || stmt->kind != NK_PAIR) continue;
        if (!isSymbol(stmt->car, "define")) continue;
        auto items = listToVec(stmt);
        if (items.size() < 2) continue;
        if (items[1]->kind == NK_SYMBOL)
            out.insert(items[1]->sval);
        else if (items[1]->kind == NK_PAIR && items[1]->car &&
                 items[1]->car->kind == NK_SYMBOL)
            out.insert(items[1]->car->sval);
    }
}

std::set<std::string> CodeGen::needsNonlocal(
        const std::vector<Node*>& body,
        const std::set<std::string>& params,
        const std::set<std::string>& locals) {
    std::set<std::string> mutations;
    for (Node* n : body) collectMutations(n, mutations);
    std::set<std::string> nl;
    for (const auto& name : mutations)
        if (!params.count(name) && !locals.count(name))
            nl.insert(name);
    return nl;
}

bool CodeGen::containsSetBang(Node* n) {
    if (!n) return false;
    if (n->kind == NK_PAIR) {
        if (isSymbol(n->car, "set!")) return true;
        if (isSymbol(n->car, "lambda") || isSymbol(n->car, "define")) return false;
        return containsSetBang(n->car) || containsSetBang(n->cdr);
    }
    return false;
}

static void collectSetTargetsDeep(Node* n, std::set<std::string>& out,
                                  std::set<Node*>& seen) {
    if (!n || seen.count(n)) return;
    seen.insert(n);

    if (n->kind == NK_PAIR) {
        if (isSymbol(n->car, "set!")) {
            auto items = listToVec(n);
            if (items.size() >= 2 && items[1]->kind == NK_SYMBOL)
                out.insert(items[1]->sval);
        }
        collectSetTargetsDeep(n->car, out, seen);
        collectSetTargetsDeep(n->cdr, out, seen);
    } else if (n->kind == NK_VECTOR) {
        collectSetTargetsDeep(n->car, out, seen);
    } else if (n->kind == NK_PROGRAM) {
        for (Node* child : n->children)
            collectSetTargetsDeep(child, out, seen);
    }
}

static void collectSetTargetsDeep(Node* n, std::set<std::string>& out) {
    std::set<Node*> seen;
    collectSetTargetsDeep(n, out, seen);
}

// ─────────────── genExpr ───────────────

std::string CodeGen::genExpr(Node* n) {
    if (!n) return "None";
    switch (n->kind) {
        case NK_INTEGER:
            return std::to_string(n->ival);
        case NK_FLOAT: {
            std::ostringstream oss; oss << n->dval;
            std::string s = oss.str();
            if (s.find('.') == std::string::npos && s.find('e') == std::string::npos)
                s += ".0";
            return s;
        }
        case NK_RATIONAL:
            return "Fraction(" + std::to_string(n->rnum) + "," + std::to_string(n->rden) + ")";
        case NK_BOOLEAN:
            return n->bval ? "True" : "False";
        case NK_STRING: {
            std::string s = "\"";
            for (char c : n->sval) {
                if      (c == '"')  s += "\\\"";
                else if (c == '\\') s += "\\\\";
                else if (c == '\n') s += "\\n";
                else if (c == '\t') s += "\\t";
                else if (c == '\r') s += "\\r";
                else                s += c;
            }
            return s + "\"";
        }
        case NK_CHAR: {
            const std::string& cv = n->sval;
            if (cv == "space")              return "' '";
            if (cv == "newline")            return "'\\n'";
            if (cv == "tab")                return "'\\t'";
            if (cv == "return")             return "'\\r'";
            if (cv == "null" || cv == "nul") return "'\\x00'";
            if (cv == "escape" || cv == "esc") return "'\\x1b'";
            if (cv == "delete" || cv == "del") return "'\\x7f'";
            if (cv == "backspace")          return "'\\b'";
            if (cv == "alarm")              return "'\\a'";
            if (cv.size() == 1)             return "'" + cv + "'";
            if (!cv.empty() && cv[0] == 'x') return "chr(0x" + cv.substr(1) + ")";
            return "'" + cv + "'";
        }
        case NK_SYMBOL:
        {
            std::string box;
            if (lookupBoxedAlias(n->sval, box))
                return box + "[0]";
            return pyId(n->sval);
        }
        case NK_NIL:
            return "[]";
        case NK_PAIR: {
            if (!n->car) return "None";
            if (n->car->kind == NK_SYMBOL) {
                const std::string& s = n->car->sval;
                if (s == "if")          return genIfExpr(n);
                if (s == "cond")        return genCondExpr(n);
                if (s == "case")        return genCaseExpr(n);
                if (s == "and")         return genAndExpr(n);
                if (s == "or")          return genOrExpr(n);
                if (s == "let")         return genLetExpr(n, "let");
                if (s == "let*")        return genLetExpr(n, "let*");
                if (s == "letrec")      return genLetExpr(n, "letrec");
                if (s == "letrec*")     return genLetExpr(n, "letrec");
                if (s == "begin") {
                    auto v = listToVec(n); v.erase(v.begin());
                    return genBeginExpr(v);
                }
                if (s == "lambda")      return genLambdaExpr(n);
                if (s == "quote")       return genQuoteExpr(listGet(n, 1));
                if (s == "quasiquote")  return genQuasiExpr(listGet(n, 1));
                if (s == "when") {
                    auto items = listToVec(n);
                    if (items.size() < 2) return "None";
                    std::vector<Node*> body(items.begin()+2, items.end());
                    return "(" + genBeginExpr(body) +
                           " if _truthy(" + genExpr(items[1]) + ") else None)";
                }
                if (s == "unless") {
                    auto items = listToVec(n);
                    if (items.size() < 2) return "None";
                    std::vector<Node*> body(items.begin()+2, items.end());
                    return "(" + genBeginExpr(body) +
                           " if not _truthy(" + genExpr(items[1]) + ") else None)";
                }
                if (s == "do")    return genDoExpr(n);
                if (s == "delay") {
                    auto items = listToVec(n);
                    return "_delay(lambda: " +
                           (items.size() >= 2 ? genExpr(items[1]) : "None") + ")";
                }
                if (s == "set!") {
                    // set! em contexto de expressão: retorna None como efeito colateral
                    auto items = listToVec(n);
                    if (items.size() >= 3 && items[1]->kind == NK_SYMBOL) {
                        std::string box;
                        if (lookupBoxedAlias(items[1]->sval, box))
                            return box + ".__setitem__(0, " + genExpr(items[2]) + ")";
                    }
                    return "None";
                }
                if (s == "define") return "None";
            }
            return genAppExpr(n);
        }
        case NK_VECTOR: {
            auto items = listToVec(n->car);
            std::string s = "[";
            for (size_t i = 0; i < items.size(); i++) {
                if (i) s += ", ";
                s += genExpr(items[i]);
            }
            return s + "]";
        }
        default:
            return "None";
    }
}

// ─────────────── expressões especiais ───────────────

std::string CodeGen::genIfExpr(Node* n) {
    auto items = listToVec(n);
    if (items.size() < 3) return "None";
    std::string cond = genExpr(items[1]);
    std::string then = genExpr(items[2]);
    std::string els  = (items.size() >= 4) ? genExpr(items[3]) : "None";
    return "(" + then + " if _truthy(" + cond + ") else " + els + ")";
}

std::string CodeGen::genCondExpr(Node* n) {
    auto clauses = listToVec(n); clauses.erase(clauses.begin());
    if (clauses.empty()) return "None";
    // constrói cadeia de ternários da direita para a esquerda
    std::string result = "None";
    for (int i = (int)clauses.size()-1; i >= 0; i--) {
        auto cv = listToVec(clauses[i]);
        if (cv.empty()) continue;
        if (isSymbol(cv[0], "else")) {
            std::vector<Node*> body(cv.begin()+1, cv.end());
            result = genBeginExpr(body);
        } else if (cv.size() == 3 && isSymbol(cv[1], "=>")) {
            // (test => proc)
            std::string t   = newTemp();
            std::string cnd = genExpr(cv[0]);
            std::string prc = genExpr(cv[2]);
            result = "(_truthy(" + t + " := " + cnd + ") and " +
                     prc + "(" + t + ") or " + result + ")";
        } else {
            std::string cnd = genExpr(cv[0]);
            std::vector<Node*> body(cv.begin()+1, cv.end());
            std::string val = body.empty() ? cnd : genBeginExpr(body);
            result = "(" + val + " if _truthy(" + cnd + ") else " + result + ")";
        }
    }
    return result;
}

std::string CodeGen::genCaseExpr(Node* n) {
    auto items = listToVec(n);
    if (items.size() < 3) return "None";

    std::string key = newTemp();
    std::string result = "None";
    for (int i = (int)items.size() - 1; i >= 2; i--) {
        auto clause = listToVec(items[i]);
        if (clause.empty()) continue;
        std::vector<Node*> body(clause.begin() + 1, clause.end());
        std::string val = body.empty() ? "None" : genBeginExpr(body);

        if (isSymbol(clause[0], "else")) {
            result = val;
            continue;
        }

        auto datums = listToVec(clause[0]);
        std::string choices = "[";
        for (size_t j = 0; j < datums.size(); j++) {
            if (j) choices += ", ";
            choices += genQuoteExpr(datums[j]);
        }
        choices += "]";
        result = "(" + val + " if _truthy(_memv(" + key + ", " +
                 choices + ")) else " + result + ")";
    }
    return "(lambda " + key + ": " + result + ")(" + genExpr(items[1]) + ")";
}

std::string CodeGen::genAndExpr(Node* n) {
    auto items = listToVec(n); items.erase(items.begin());
    if (items.empty()) return "True";
    // short-circuit correto
    std::string result = genExpr(items.back());
    for (int i = (int)items.size()-2; i >= 0; i--) {
        std::string t = newTemp();
        std::string e = genExpr(items[i]);
        result = "((" + t + " := " + e + "), " + t +
                 " if not _truthy(" + t + ") else " + result + ")[-1]";
    }
    return result;
}

std::string CodeGen::genOrExpr(Node* n) {
    auto items = listToVec(n); items.erase(items.begin());
    if (items.empty()) return "False";
    std::string result = genExpr(items.back());
    for (int i = (int)items.size()-2; i >= 0; i--) {
        std::string t = newTemp();
        std::string e = genExpr(items[i]);
        result = "((" + t + " := " + e + "), " + t +
                 " if _truthy(" + t + ") else " + result + ")[-1]";
    }
    return result;
}

std::string CodeGen::genBeginExpr(const std::vector<Node*>& body) {
    if (body.empty())  return "None";
    if (body.size() == 1) return genExpr(body[0]);
    // Se algum elemento contém set!, usamos uma lambda auxiliar
    bool hasSet = false;
    for (Node* n : body) if (containsSetBang(n)) { hasSet = true; break; }
    if (hasSet) {
        // gera como def + chamada imediata
        std::string fname = newLambda();
        // não é possível gerar def inline como expressão pura,
        // mas podemos usar (lambda: [...])()
        // Limitação documentada: set! dentro de begin-expr pode não propagar corretamente
        std::string s = "(lambda: [";
        for (size_t i = 0; i < body.size(); i++) {
            if (i) s += ", ";
            s += genExpr(body[i]);
        }
        return s + "])()[-1]";
    }
    std::string s = "[";
    for (size_t i = 0; i < body.size(); i++) {
        if (i) s += ", ";
        s += genExpr(body[i]);
    }
    return s + "][-1]";
}

std::string CodeGen::genLambdaExpr(Node* n) {
    auto items = listToVec(n);
    if (items.size() < 3) return "lambda: None";
    Node* params = items[1];
    std::vector<Node*> body(items.begin()+2, items.end());

    bool hasSet = false;
    for (Node* b : body) if (containsSetBang(b)) { hasSet = true; break; }

    std::string paramStr = fmtParams(params);

    if (!hasSet && body.size() == 1) {
        return "lambda " + paramStr + ": " + genExpr(body[0]);
    }
    // corpo com múltiplas expressões ou set!: usar lista trick
    // (lambda params: [e1, e2, ..., en][-1])
    std::string s = "lambda " + paramStr + ": [";
    for (size_t i = 0; i < body.size(); i++) {
        if (i) s += ", ";
        s += genExpr(body[i]);
    }
    return s + "][-1]";
}

std::string CodeGen::genLetExpr(Node* n, const std::string& kind) {
    auto items = listToVec(n);
    // named let
    if (kind == "let" && items.size() >= 4 && items[1]->kind == NK_SYMBOL)
        return genNamedLetExpr(n);

    if (items.size() < 3) return "None";
    Node* bindings = items[1];
    std::vector<Node*> body(items.begin()+2, items.end());

    if (kind == "let") {
        // (lambda (vars) body)(vals)
        std::string params, vals;
        auto bv = listToVec(bindings);
        std::set<std::string> mutated;
        for (Node* b : body) collectSetTargetsDeep(b, mutated);
        std::map<std::string, std::string> aliasScope;

        for (size_t i = 0; i < bv.size(); i++) {
            auto bnd = listToVec(bv[i]);
            if (bnd.size() < 2) continue;
            if (i) { params += ", "; vals += ", "; }
            std::string name = bnd[0]->sval;
            if (mutated.count(name)) {
                std::string box = "_box_" + pyId(name);
                params += box;
                vals   += "[" + genExpr(bnd[1]) + "]";
                aliasScope[name] = box;
            } else {
                params += pyId(name);
                vals   += genExpr(bnd[1]);
                aliasScope[name] = "";
            }
        }
        boxedAliases.push_back(aliasScope);
        std::string bodyExpr = genBeginExpr(body);
        boxedAliases.pop_back();
        return "(lambda " + params + ": " + bodyExpr + ")(" + vals + ")";
    }
    if (kind == "let*") {
        // encadeamento de lambdas
        auto bv = listToVec(bindings);
        std::string result = genBeginExpr(body);
        for (int i = (int)bv.size()-1; i >= 0; i--) {
            auto bnd = listToVec(bv[i]);
            if (bnd.size() < 2) continue;
            std::string var = pyId(bnd[0]->sval);
            std::string val = genExpr(bnd[1]);
            result = "(lambda " + var + ": " + result + ")(" + val + ")";
        }
        return result;
    }
    // letrec: mutação necessária
    auto bv = listToVec(bindings);
    std::string exprs;
    for (size_t i = 0; i < bv.size(); i++) {
        auto bnd = listToVec(bv[i]);
        if (bnd.size() < 2) continue;
        if (!exprs.empty()) exprs += ", ";
        exprs += "(" + pyId(bnd[0]->sval) + " := " + genExpr(bnd[1]) + ")";
    }
    std::string bodyStr = genBeginExpr(body);
    if (!exprs.empty()) exprs += ", ";
    exprs += bodyStr;
    return "(lambda: [" + exprs + "][-1])()";
}

std::string CodeGen::genNamedLetExpr(Node* n) {
    // (let name ((x v)...) body...)
    auto items = listToVec(n);
    std::string fname = pyId(items[1]->sval);
    auto bv = listToVec(items[2]);
    std::string params, initVals;
    for (size_t i = 0; i < bv.size(); i++) {
        auto bnd = listToVec(bv[i]);
        if (bnd.size() < 2) continue;
        if (i) { params += ", "; initVals += ", "; }
        params   += pyId(bnd[0]->sval);
        initVals += genExpr(bnd[1]);
    }
    std::vector<Node*> body(items.begin()+3, items.end());
    std::string bodyStr = genBeginExpr(body);
    return "(lambda: [(" + fname + " := (lambda " + params + ": " +
           bodyStr + ")), " + fname + "(" + initVals + ")][-1])()";
}

std::string CodeGen::genDoExpr(Node* n) {
    // do em contexto de expressão: gera uma IIFE
    return "(lambda: " + genDoStmt(n, 0) + ")()  # do-expr";
}

std::string CodeGen::genQuoteExpr(Node* data) {
    if (!data) return "[]";
    switch (data->kind) {
        case NK_INTEGER:  return std::to_string(data->ival);
        case NK_FLOAT: {
            std::ostringstream oss; oss << data->dval;
            return oss.str();
        }
        case NK_RATIONAL:
            return "Fraction(" + std::to_string(data->rnum) +
                   "," + std::to_string(data->rden) + ")";
        case NK_BOOLEAN:  return data->bval ? "True" : "False";
        case NK_STRING:   return genExpr(data);
        case NK_CHAR:     return genExpr(data);
        case NK_SYMBOL:   return "\"" + data->sval + "\"";
        case NK_NIL:      return "[]";
        case NK_PAIR: {
            // quoted list → Python list
            std::string s = "[";
            Node* cur = data;
            bool first = true;
            while (cur && cur->kind == NK_PAIR) {
                if (!first) s += ", ";
                s += genQuoteExpr(cur->car);
                cur   = cur->cdr;
                first = false;
            }
            if (cur && cur->kind != NK_NIL)
                s += ", " + genQuoteExpr(cur); // dotted pair tail
            return s + "]";
        }
        case NK_VECTOR: {
            std::string s = "[";
            Node* cur = data->car;
            bool first = true;
            while (cur && cur->kind == NK_PAIR) {
                if (!first) s += ", ";
                s += genQuoteExpr(cur->car);
                cur   = cur->cdr;
                first = false;
            }
            return s + "]";
        }
        default: return "None";
    }
}

std::string CodeGen::genQuasiExpr(Node* data) {
    if (!data) return "[]";
    if (data->kind != NK_PAIR) return genQuoteExpr(data);
    // (unquote x) → genExpr(x)
    if (isSymbol(data->car, "unquote"))
        return genExpr(listGet(data, 1));
    // lista com possíveis unquotes
    std::string s = "_cons_list([";
    Node* cur = data;
    bool first = true;
    while (cur && cur->kind == NK_PAIR) {
        Node* elem = cur->car;
        if (!first) s += ", ";
        if (elem && elem->kind == NK_PAIR && isSymbol(elem->car, "unquote-splicing")) {
            s += "('splice', " + genExpr(listGet(elem, 1)) + ")";
        } else {
            s += genQuasiExpr(elem);
        }
        cur   = cur->cdr;
        first = false;
    }
    return s + "])";
}

std::string CodeGen::genAppExpr(Node* n) {
    std::string func = genExpr(n->car);
    if (n->car && n->car->kind == NK_PAIR)
        func = "(" + func + ")";
    std::string args;
    Node* a = n->cdr;
    bool first = true;
    while (a && a->kind == NK_PAIR) {
        if (!first) args += ", ";
        args += genExpr(a->car);
        a     = a->cdr;
        first = false;
    }
    return func + "(" + args + ")";
}

// ─────────────── formatação de parâmetros ───────────────

std::string CodeGen::fmtParams(Node* params) {
    if (!params || params->kind == NK_NIL) return "";
    if (params->kind == NK_SYMBOL) return "*" + pyId(params->sval);
    std::string s;
    Node* p = params;
    bool first = true;
    while (p && p->kind == NK_PAIR) {
        if (!first) s += ", ";
        if (p->car) s += pyId(p->car->sval);
        p     = p->cdr;
        first = false;
    }
    if (p && p->kind == NK_SYMBOL)
        s += ", *" + pyId(p->sval);
    return s;
}

// ─────────────── genStmt ───────────────

std::string CodeGen::genStmt(Node* n, int il) {
    if (!n) return ind(il) + "pass\n";
    if (n->kind != NK_PAIR) return ind(il) + genExpr(n) + "\n";
    if (n->car && n->car->kind == NK_SYMBOL) {
        const std::string& s = n->car->sval;
        if (s == "define")   return genDefineStmt(n, il);
        if (s == "set!")     return genSetStmt(n, il);
        if (s == "if")       return genIfStmt(n, il);
        if (s == "cond")     return genCondStmt(n, il);
        if (s == "case")     return genCaseStmt(n, il);
        if (s == "when")     return genWhenStmt(n, il);
        if (s == "unless")   return genUnlessStmt(n, il);
        if (s == "let")      return genLetStmt(n, "let", il);
        if (s == "let*")     return genLetStmt(n, "let*", il);
        if (s == "letrec")   return genLetStmt(n, "letrec", il);
        if (s == "letrec*")  return genLetStmt(n, "letrec", il);
        if (s == "begin") {
            auto v = listToVec(n); v.erase(v.begin());
            return genBeginStmt(v, il);
        }
        if (s == "do")       return genDoStmt(n, il);
        if (s == "lambda" || s == "quote" || s == "and" || s == "or") {
            // expressão pura usada como statement
            return ind(il) + genExpr(n) + "\n";
        }
    }
    return ind(il) + genExpr(n) + "\n";
}

// ─────────────── statements especiais ───────────────

std::string CodeGen::genDefineStmt(Node* n, int il) {
    auto items = listToVec(n);
    if (items.size() < 3) return ind(il) + "pass  # define inválido\n";
    Node* target = items[1];

    if (target->kind == NK_SYMBOL) {
        std::string var = pyId(target->sval);
        std::string val = genExpr(items[2]);
        return ind(il) + var + " = " + val + "\n";
    }
    if (target->kind == NK_PAIR) {
        Node* nameNode = target->car;
        if (!nameNode || nameNode->kind != NK_SYMBOL)
            return ind(il) + "pass  # define inválido\n";
        std::string fname = pyId(nameNode->sval);
        std::string pstr  = fmtParams(target->cdr);
        std::vector<Node*> body(items.begin()+2, items.end());

        std::set<std::string> params;
        Node* p = target->cdr;
        while (p && p->kind == NK_PAIR) {
            if (p->car && p->car->kind == NK_SYMBOL) params.insert(p->car->sval);
            p = p->cdr;
        }
        if (p && p->kind == NK_SYMBOL) params.insert(p->sval);

        std::string out = ind(il) + "def " + fname + "(" + pstr + "):\n";
        out += genFuncBody(body, il+1, params);
        return out;
    }
    return ind(il) + "pass  # define inválido\n";
}

std::string CodeGen::genSetStmt(Node* n, int il) {
    auto items = listToVec(n);
    if (items.size() < 3 || items[1]->kind != NK_SYMBOL) return ind(il) + "pass\n";
    std::string box;
    std::string var = pyId(items[1]->sval);
    std::string val = genExpr(items[2]);
    if (lookupBoxedAlias(items[1]->sval, box))
        return ind(il) + box + "[0] = " + val + "\n";

    // Determina se precisa de nonlocal/global
    std::string decl;
    if (!scopes.empty()) {
        bool foundLocal = false;
        for (int i = (int)scopes.size()-1; i >= 0; i--) {
            if (scopes[i].params.count(items[1]->sval) ||
                scopes[i].locals.count(items[1]->sval)) {
                foundLocal = true;
                break;
            }
            if (scopes[i].isTopLevel) break;
        }
        if (!foundLocal && scopes.size() > 1) {
            bool topLevel = scopes[0].isTopLevel;
            decl = ind(il) + (topLevel ? "global " : "nonlocal ") + var + "\n";
        }
    }
    return decl + ind(il) + var + " = " + val + "\n";
}

std::string CodeGen::genIfStmt(Node* n, int il) {
    auto items = listToVec(n);
    if (items.size() < 3) return ind(il) + "pass\n";
    std::string cond = genExpr(items[1]);
    std::string out  = ind(il) + "if _truthy(" + cond + "):\n";
    out += genStmt(items[2], il+1);
    if (items.size() >= 4) {
        out += ind(il) + "else:\n";
        out += genStmt(items[3], il+1);
    }
    return out;
}

std::string CodeGen::genCondStmt(Node* n, int il) {
    auto clauses = listToVec(n); clauses.erase(clauses.begin());
    if (clauses.empty()) return ind(il) + "pass\n";
    std::string out;
    bool first = true;
    for (Node* clause : clauses) {
        auto cv = listToVec(clause);
        if (cv.empty()) continue;
        std::vector<Node*> body(cv.begin()+1, cv.end());
        if (isSymbol(cv[0], "else")) {
            out += ind(il) + "else:\n";
            if (body.empty()) out += ind(il+1) + "pass\n";
            else out += genBeginStmt(body, il+1);
        } else {
            std::string kw = first ? "if" : "elif";
            // cond => proc
            if (cv.size() == 3 && isSymbol(cv[1], "=>")) {
                std::string t = newTemp();
                out += ind(il) + t + " = " + genExpr(cv[0]) + "\n";
                out += ind(il) + kw + " _truthy(" + t + "):\n";
                out += ind(il+1) + genExpr(cv[2]) + "(" + t + ")\n";
            } else {
                out += ind(il) + kw + " _truthy(" + genExpr(cv[0]) + "):\n";
                if (body.empty()) out += ind(il+1) + "pass\n";
                else out += genBeginStmt(body, il+1);
            }
            first = false;
        }
    }
    return out;
}

std::string CodeGen::genCaseStmt(Node* n, int il) {
    auto items = listToVec(n);
    if (items.size() < 3) return ind(il) + "pass\n";

    std::string key = newTemp();
    std::string out = ind(il) + key + " = " + genExpr(items[1]) + "\n";
    bool first = true;

    for (size_t i = 2; i < items.size(); i++) {
        auto clause = listToVec(items[i]);
        if (clause.empty()) continue;
        std::vector<Node*> body(clause.begin() + 1, clause.end());

        if (isSymbol(clause[0], "else")) {
            out += ind(il) + std::string(first ? "if True" : "else") + ":\n";
            first = false;
        } else {
            auto datums = listToVec(clause[0]);
            std::string choices = "[";
            for (size_t j = 0; j < datums.size(); j++) {
                if (j) choices += ", ";
                choices += genQuoteExpr(datums[j]);
            }
            choices += "]";
            out += ind(il) + std::string(first ? "if" : "elif") +
                   " _truthy(_memv(" + key + ", " + choices + ")):\n";
            first = false;
        }

        if (body.empty()) out += ind(il+1) + "pass\n";
        else out += genBeginStmt(body, il+1);
    }
    return out;
}

std::string CodeGen::genWhenStmt(Node* n, int il) {
    auto items = listToVec(n);
    if (items.size() < 2) return ind(il) + "pass\n";
    std::vector<Node*> body(items.begin()+2, items.end());
    std::string out = ind(il) + "if _truthy(" + genExpr(items[1]) + "):\n";
    if (body.empty()) out += ind(il+1) + "pass\n";
    else out += genBeginStmt(body, il+1);
    return out;
}

std::string CodeGen::genUnlessStmt(Node* n, int il) {
    auto items = listToVec(n);
    if (items.size() < 2) return ind(il) + "pass\n";
    std::vector<Node*> body(items.begin()+2, items.end());
    std::string out = ind(il) + "if not _truthy(" + genExpr(items[1]) + "):\n";
    if (body.empty()) out += ind(il+1) + "pass\n";
    else out += genBeginStmt(body, il+1);
    return out;
}

std::string CodeGen::genLetStmt(Node* n, const std::string& kind, int il) {
    auto items = listToVec(n);
    if (kind == "let" && items.size() >= 4 && items[1]->kind == NK_SYMBOL)
        return genNamedLetStmt(n, il);
    if (items.size() < 3) return ind(il) + "pass\n";
    Node* bindings = items[1];
    std::vector<Node*> body(items.begin()+2, items.end());

    if (kind == "let") {
        // avalia todos os valores primeiro, depois atribui
        auto bv = listToVec(bindings);
        std::vector<std::pair<std::string,std::string>> assigns;
        for (auto* b : bv) {
            auto bnd = listToVec(b);
            if (bnd.size() < 2) continue;
            assigns.push_back({pyId(bnd[0]->sval), genExpr(bnd[1])});
        }
        std::string out;
        std::vector<std::string> tmps;
        for (auto& [var, val] : assigns) {
            std::string t = newTemp();
            out += ind(il) + t + " = " + val + "\n";
            tmps.push_back(t);
        }
        for (size_t i = 0; i < assigns.size(); i++)
            out += ind(il) + assigns[i].first + " = " + tmps[i] + "\n";
        out += genBeginStmt(body, il);
        return out;
    }
    if (kind == "let*") {
        std::string out;
        Node* b = bindings;
        while (b && b->kind == NK_PAIR) {
            auto bnd = listToVec(b->car);
            if (bnd.size() >= 2 && bnd[0]->kind == NK_SYMBOL)
                out += ind(il) + pyId(bnd[0]->sval) + " = " + genExpr(bnd[1]) + "\n";
            b = b->cdr;
        }
        out += genBeginStmt(body, il);
        return out;
    }
    // letrec: define todas, depois inicializa
    std::string out;
    Node* b = bindings;
    while (b && b->kind == NK_PAIR) {
        auto bnd = listToVec(b->car);
        if (!bnd.empty() && bnd[0]->kind == NK_SYMBOL)
            out += ind(il) + pyId(bnd[0]->sval) + " = None\n";
        b = b->cdr;
    }
    b = bindings;
    while (b && b->kind == NK_PAIR) {
        auto bnd = listToVec(b->car);
        if (bnd.size() >= 2 && bnd[0]->kind == NK_SYMBOL)
            out += ind(il) + pyId(bnd[0]->sval) + " = " + genExpr(bnd[1]) + "\n";
        b = b->cdr;
    }
    out += genBeginStmt(body, il);
    return out;
}

std::string CodeGen::genNamedLetStmt(Node* n, int il) {
    // (let name ((x v)...) body...)
    auto items = listToVec(n);
    std::string fname = pyId(items[1]->sval);
    auto bv = listToVec(items[2]);
    std::string params, initVals;
    std::set<std::string> paramSet;
    for (size_t i = 0; i < bv.size(); i++) {
        auto bnd = listToVec(bv[i]);
        if (bnd.size() < 2) continue;
        if (i) { params += ", "; initVals += ", "; }
        std::string pn = pyId(bnd[0]->sval);
        params   += pn;
        initVals += genExpr(bnd[1]);
        paramSet.insert(bnd[0]->sval);
    }
    std::vector<Node*> body(items.begin()+3, items.end());
    std::string out = ind(il) + "def " + fname + "(" + params + "):\n";
    out += genFuncBody(body, il+1, paramSet);
    out += ind(il) + fname + "(" + initVals + ")\n";
    return out;
}

std::string CodeGen::genDoStmt(Node* n, int il, bool withReturn) {
    // (do ((var init step)...) (test result...) body...)
    auto items = listToVec(n);
    if (items.size() < 3) return ind(il) + "pass  # do inválido\n";

    std::string out;
    // inicializações
    auto varSpecs = listToVec(items[1]);
    for (auto* vs : varSpecs) {
        auto sv = listToVec(vs);
        if (sv.empty()) continue;
        std::string var = pyId(sv[0]->sval);
        std::string init = (sv.size() >= 2) ? genExpr(sv[1]) : "None";
        out += ind(il) + var + " = " + init + "\n";
    }
    // loop while
    auto testClause = listToVec(items[2]);
    if (testClause.empty()) return out + ind(il) + "pass  # do sem teste\n";
    std::string testExpr = genExpr(testClause[0]);
    std::vector<Node*> result(testClause.begin()+1, testClause.end());
    std::vector<Node*> body(items.begin()+3, items.end());

    out += ind(il) + "while not _truthy(" + testExpr + "):\n";
    // corpo do do
    if (body.empty()) out += ind(il+1) + "pass\n";
    else out += genBeginStmt(body, il+1);
    // atualização das variáveis (com temps para evitar conflito)
    std::vector<std::pair<std::string, std::string>> updates;
    for (auto* vs : varSpecs) {
        auto sv = listToVec(vs);
        if (sv.size() < 3) continue;
        std::string t   = newTemp();
        std::string var = pyId(sv[0]->sval);
        out += ind(il+1) + t + " = " + genExpr(sv[2]) + "\n";
        updates.push_back({var, t});
    }
    for (auto& [var, t] : updates)
        out += ind(il+1) + var + " = " + t + "\n";
    // resultado
    if (!result.empty()) {
        out += genBeginStmt(result, il, withReturn);
    } else if (withReturn) {
        out += ind(il) + "return None\n";
    }
    return out;
}

std::string CodeGen::genBeginStmt(const std::vector<Node*>& body, int il, bool withReturn) {
    if (body.empty()) return ind(il) + "pass\n";
    std::string out;
    for (size_t i = 0; i < body.size(); i++) {
        if (withReturn && i == body.size()-1) {
            if (body[i] && body[i]->kind == NK_PAIR && isSymbol(body[i]->car, "do"))
                out += genDoStmt(body[i], il, true);
            else
                out += ind(il) + "return " + genExpr(body[i]) + "\n";
        } else {
            out += genStmt(body[i], il);
        }
    }
    return out;
}

std::string CodeGen::genFuncBody(const std::vector<Node*>& body, int il,
                                   const std::set<std::string>& params) {
    if (body.empty()) return ind(il) + "pass\n";

    std::set<std::string> locals;
    collectLocalDefs(body, locals);

    // nonlocal / global declarations
    std::set<std::string> nl = needsNonlocal(body, params, locals);
    std::string out;
    if (!scopes.empty()) {
        std::set<std::string> globals, nonlocals;
        for (const auto& name : nl) {
            if (scopes[0].isTopLevel)
                globals.insert(pyId(name));
            else
                nonlocals.insert(pyId(name));
        }
        for (const auto& g : globals)   out += ind(il) + "global " + g + "\n";
        for (const auto& nv : nonlocals) out += ind(il) + "nonlocal " + nv + "\n";
    }

    // empurra escopo
    Scope sc;
    sc.params     = params;
    sc.locals     = locals;
    sc.isTopLevel = false;
    scopes.push_back(sc);

    out += genBeginStmt(body, il, true);

    scopes.pop_back();
    return out;
}

// ─────────────── generate (entry point) ───────────────

std::string CodeGen::generate(Node* program) {
    if (!program) return "# programa vazio\n";

    Scope topScope;
    topScope.isTopLevel = true;
    scopes.push_back(topScope);

    std::string out;
    out += "from fractions import Fraction\n";
    out += "from scheme_runtime import *\n\n";

    for (Node* expr : program->children) {
        out += genStmt(expr, 0);
    }

    scopes.pop_back();
    return out;
}
