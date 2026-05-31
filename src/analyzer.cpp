#include "analyzer.hpp"
#include <sstream>

Analyzer::Analyzer(SymTable& sym) : sym(sym) {}

void Analyzer::addError(const std::string& msg, int line, int col) {
    std::ostringstream oss;
    oss << "Erro [" << line << ":" << col << "]: " << msg;
    errors.push_back(oss.str());
}

void Analyzer::addWarning(const std::string& msg, int line, int col) {
    std::ostringstream oss;
    oss << "Aviso [" << line << ":" << col << "]: " << msg;
    warnings.push_back(oss.str());
}

bool Analyzer::analyze(Node* program) {
    if (!program) return false;
    for (auto* expr : program->children)
        analyzeExpr(expr);
    return errors.empty();
}

SchemeType Analyzer::analyzeExpr(Node* n) {
    if (!n) return ST_UNKNOWN;
    switch (n->kind) {
        case NK_INTEGER:  n->stype = ST_INTEGER;   return ST_INTEGER;
        case NK_FLOAT:    n->stype = ST_FLOAT;     return ST_FLOAT;
        case NK_RATIONAL: n->stype = ST_RATIONAL;  return ST_RATIONAL;
        case NK_BOOLEAN:  n->stype = ST_BOOLEAN;   return ST_BOOLEAN;
        case NK_STRING:   n->stype = ST_STRING;    return ST_STRING;
        case NK_CHAR:     n->stype = ST_CHAR;      return ST_CHAR;
        case NK_NIL:      n->stype = ST_NIL;       return ST_NIL;
        case NK_SYMBOL: {
            auto* e = sym.lookup(n->sval);
            if (!e) {
                addError("variável não definida: '" + n->sval + "'", n->line, n->col);
                n->stype = ST_UNKNOWN;
                return ST_UNKNOWN;
            }
            n->stype = e->type;
            return e->type;
        }
        case NK_PAIR:
            if (!n->car) {
                addError("lista vazia não é uma expressão", n->line, n->col);
                return ST_UNKNOWN;
            }
            return analyzeList(n);
        case NK_VECTOR: {
            Node* items = n->car;
            while (items && items->kind == NK_PAIR) {
                analyzeExpr(items->car);
                items = items->cdr;
            }
            n->stype = ST_VECTOR;
            return ST_VECTOR;
        }
        default:
            return ST_UNKNOWN;
    }
}

SchemeType Analyzer::analyzeList(Node* n) {
    if (n->car && n->car->kind == NK_SYMBOL) {
        const std::string& s = n->car->sval;
        if (s == "define")      return analyzeDefine(n);
        if (s == "lambda")      return analyzeLambda(n);
        if (s == "if")          return analyzeIf(n);
        if (s == "cond")        return analyzeCond(n);
        if (s == "case")        return analyzeCase(n);
        if (s == "let")         return analyzeLet(n, "let");
        if (s == "let*")        return analyzeLet(n, "let*");
        if (s == "letrec")      return analyzeLet(n, "letrec");
        if (s == "letrec*")     return analyzeLet(n, "letrec*");
        if (s == "begin")       return analyzeBegin(n);
        if (s == "set!")        return analyzeSetBang(n);
        if (s == "and")         return analyzeAnd(n);
        if (s == "or")          return analyzeOr(n);
        if (s == "when")        return analyzeWhen(n);
        if (s == "unless")      return analyzeUnless(n);
        if (s == "do")          return analyzeDo(n);
        if (s == "quote") {
            SchemeType t = quoteType(listGet(n, 1));
            n->stype = t; return t;
        }
        if (s == "quasiquote")  return analyzeQuasi(n);
        if (s == "unquote" || s == "unquote-splicing") {
            // should not appear outside quasiquote
            addError("unquote fora de quasiquote", n->line, n->col);
            n->stype = ST_ANY; return ST_ANY;
        }
        if (s == "delay") {
            if (n->cdr && n->cdr->kind == NK_PAIR) analyzeExpr(n->cdr->car);
            n->stype = ST_ANY; return ST_ANY;
        }
        if (s == "force") {
            if (n->cdr && n->cdr->kind == NK_PAIR) analyzeExpr(n->cdr->car);
            n->stype = ST_ANY; return ST_ANY;
        }
        if (s == "define-values" || s == "define-syntax" ||
            s == "let-syntax" || s == "letrec-syntax" || s == "syntax-rules") {
            addWarning("forma '" + s + "' não suportada nesta versão", n->line, n->col);
            n->stype = ST_VOID; return ST_VOID;
        }
    }
    return analyzeApp(n);
}

SchemeType Analyzer::analyzeDefine(Node* n) {
    auto items = listToVec(n);
    if (items.size() < 3) {
        addError("define: requer pelo menos dois operandos", n->line, n->col);
        return ST_VOID;
    }
    Node* target = items[1];
    if (target->kind == NK_SYMBOL) {
        // (define var expr)
        SchemeType t = analyzeExpr(items[2]);
        sym.define(target->sval, t);
    } else if (target->kind == NK_PAIR) {
        // (define (name . params) body...)
        Node* nameNode = target->car;
        if (!nameNode || nameNode->kind != NK_SYMBOL) {
            addError("define: nome de função inválido", n->line, n->col);
            return ST_VOID;
        }
        sym.define(nameNode->sval, ST_PROCEDURE, -1);
        sym.pushScope();
        Node* p = target->cdr;
        int arity = 0;
        bool variadic = false;
        while (p && p->kind == NK_PAIR) {
            if (p->car && p->car->kind == NK_SYMBOL)
                sym.define(p->car->sval, ST_ANY);
            arity++;
            p = p->cdr;
        }
        if (p && p->kind == NK_SYMBOL) {
            sym.define(p->sval, ST_LIST);
            variadic = true;
        }
        for (size_t i = 2; i < items.size(); i++) analyzeExpr(items[i]);
        sym.popScope();
        sym.define(nameNode->sval, ST_PROCEDURE, variadic ? -2 : arity);
    } else {
        addError("define: alvo inválido", n->line, n->col);
    }
    n->stype = ST_VOID;
    return ST_VOID;
}

SchemeType Analyzer::analyzeLambda(Node* n) {
    auto items = listToVec(n);
    if (items.size() < 3) {
        addError("lambda: requer parâmetros e corpo", n->line, n->col);
        n->stype = ST_PROCEDURE;
        return ST_PROCEDURE;
    }
    sym.pushScope();
    Node* params = items[1];
    if (params->kind == NK_SYMBOL) {
        sym.define(params->sval, ST_LIST);
    } else if (params->kind == NK_PAIR) {
        Node* p = params;
        while (p && p->kind == NK_PAIR) {
            if (p->car && p->car->kind == NK_SYMBOL)
                sym.define(p->car->sval, ST_ANY);
            p = p->cdr;
        }
        if (p && p->kind == NK_SYMBOL)
            sym.define(p->sval, ST_LIST);
    }
    for (size_t i = 2; i < items.size(); i++) analyzeExpr(items[i]);
    sym.popScope();
    n->stype = ST_PROCEDURE;
    return ST_PROCEDURE;
}

SchemeType Analyzer::analyzeIf(Node* n) {
    auto items = listToVec(n);
    if (items.size() < 3 || items.size() > 4) {
        addError("if: (if teste então [senão])", n->line, n->col);
        n->stype = ST_ANY; return ST_ANY;
    }
    analyzeExpr(items[1]);
    SchemeType t1 = analyzeExpr(items[2]);
    SchemeType t2 = (items.size() == 4) ? analyzeExpr(items[3]) : ST_VOID;
    n->stype = (t1 == t2) ? t1 : ST_ANY;
    return n->stype;
}

SchemeType Analyzer::analyzeCond(Node* n) {
    Node* clauses = n->cdr;
    while (clauses && clauses->kind == NK_PAIR) {
        Node* clause = clauses->car;
        if (!clause || clause->kind != NK_PAIR) {
            addError("cond: cláusula inválida", n->line, n->col);
        } else {
            auto cv = listToVec(clause);
            if (cv.empty()) {
                addError("cond: cláusula vazia", n->line, n->col);
            } else {
                if (!isSymbol(cv[0], "else")) analyzeExpr(cv[0]);
                // cláusula => proc
                if (cv.size() == 3 && isSymbol(cv[1], "=>")) {
                    analyzeExpr(cv[2]);
                } else {
                    for (size_t i = 1; i < cv.size(); i++) analyzeExpr(cv[i]);
                }
            }
        }
        clauses = clauses->cdr;
    }
    n->stype = ST_ANY;
    return ST_ANY;
}

SchemeType Analyzer::analyzeCase(Node* n) {
    auto items = listToVec(n);
    if (items.size() < 3) {
        addError("case: sintaxe inválida", n->line, n->col);
        n->stype = ST_ANY; return ST_ANY;
    }
    analyzeExpr(items[1]);
    for (size_t i = 2; i < items.size(); i++) {
        auto cv = listToVec(items[i]);
        for (size_t j = 1; j < cv.size(); j++) analyzeExpr(cv[j]);
    }
    n->stype = ST_ANY;
    return ST_ANY;
}

SchemeType Analyzer::analyzeLet(Node* n, const std::string& kind) {
    auto items = listToVec(n);
    // named let: (let name bindings body...)
    if ((kind == "let") && items.size() >= 4 && items[1]->kind == NK_SYMBOL)
        return analyzeNamedLet(n);

    if (items.size() < 3) {
        addError(kind + ": sintaxe inválida", n->line, n->col);
        n->stype = ST_ANY; return ST_ANY;
    }
    Node* bindings = items[1];

    if (kind == "letrec" || kind == "letrec*") {
        sym.pushScope();
        // registra todos os nomes antes de analisar valores
        Node* b = bindings;
        while (b && b->kind == NK_PAIR) {
            auto bv = listToVec(b->car);
            if (!bv.empty() && bv[0]->kind == NK_SYMBOL)
                sym.define(bv[0]->sval, ST_UNKNOWN);
            b = b->cdr;
        }
        b = bindings;
        while (b && b->kind == NK_PAIR) {
            auto bv = listToVec(b->car);
            if (bv.size() >= 2 && bv[0]->kind == NK_SYMBOL) {
                SchemeType t = analyzeExpr(bv[1]);
                sym.define(bv[0]->sval, t);
            }
            b = b->cdr;
        }
    } else if (kind == "let*") {
        sym.pushScope();
        Node* b = bindings;
        while (b && b->kind == NK_PAIR) {
            auto bv = listToVec(b->car);
            if (bv.size() >= 2 && bv[0]->kind == NK_SYMBOL) {
                SchemeType t = analyzeExpr(bv[1]);
                sym.define(bv[0]->sval, t);
            }
            b = b->cdr;
        }
    } else {
        // let: avalia valores no escopo atual
        std::vector<std::pair<std::string, SchemeType>> binds;
        Node* b = bindings;
        while (b && b->kind == NK_PAIR) {
            auto bv = listToVec(b->car);
            if (bv.size() >= 2 && bv[0]->kind == NK_SYMBOL) {
                SchemeType t = analyzeExpr(bv[1]);
                binds.push_back({bv[0]->sval, t});
            }
            b = b->cdr;
        }
        sym.pushScope();
        for (auto& [nm, t] : binds) sym.define(nm, t);
    }

    SchemeType bt = ST_VOID;
    for (size_t i = 2; i < items.size(); i++) bt = analyzeExpr(items[i]);
    sym.popScope();
    n->stype = bt;
    return bt;
}

SchemeType Analyzer::analyzeNamedLet(Node* n) {
    auto items = listToVec(n);
    // items[0]=let, items[1]=name, items[2]=bindings, items[3..]=body
    std::string loopName = items[1]->sval;
    Node* bindings = items[2];
    sym.pushScope();
    sym.define(loopName, ST_PROCEDURE);
    Node* b = bindings;
    while (b && b->kind == NK_PAIR) {
        auto bv = listToVec(b->car);
        if (bv.size() >= 2 && bv[0]->kind == NK_SYMBOL) {
            SchemeType t = analyzeExpr(bv[1]);
            sym.define(bv[0]->sval, t);
        }
        b = b->cdr;
    }
    SchemeType bt = ST_VOID;
    for (size_t i = 3; i < items.size(); i++) bt = analyzeExpr(items[i]);
    sym.popScope();
    n->stype = bt;
    return bt;
}

SchemeType Analyzer::analyzeBegin(Node* n) {
    SchemeType t = ST_VOID;
    Node* exprs = n->cdr;
    while (exprs && exprs->kind == NK_PAIR) {
        t = analyzeExpr(exprs->car);
        exprs = exprs->cdr;
    }
    n->stype = t;
    return t;
}

SchemeType Analyzer::analyzeSetBang(Node* n) {
    auto items = listToVec(n);
    if (items.size() != 3) {
        addError("set!: (set! var expr)", n->line, n->col);
        n->stype = ST_VOID; return ST_VOID;
    }
    if (items[1]->kind != NK_SYMBOL) {
        addError("set!: primeiro argumento deve ser símbolo", n->line, n->col);
        n->stype = ST_VOID; return ST_VOID;
    }
    if (!sym.isDefined(items[1]->sval))
        addError("set!: variável não definida: '" + items[1]->sval + "'", n->line, n->col);
    analyzeExpr(items[2]);
    n->stype = ST_VOID;
    return ST_VOID;
}

SchemeType Analyzer::analyzeAnd(Node* n) {
    Node* exprs = n->cdr;
    while (exprs && exprs->kind == NK_PAIR) {
        analyzeExpr(exprs->car);
        exprs = exprs->cdr;
    }
    n->stype = ST_ANY;
    return ST_ANY;
}

SchemeType Analyzer::analyzeOr(Node* n) {
    Node* exprs = n->cdr;
    while (exprs && exprs->kind == NK_PAIR) {
        analyzeExpr(exprs->car);
        exprs = exprs->cdr;
    }
    n->stype = ST_ANY;
    return ST_ANY;
}

SchemeType Analyzer::analyzeWhen(Node* n) {
    auto items = listToVec(n);
    if (items.size() < 2) {
        addError("when: requer condição", n->line, n->col);
        n->stype = ST_VOID; return ST_VOID;
    }
    analyzeExpr(items[1]);
    for (size_t i = 2; i < items.size(); i++) analyzeExpr(items[i]);
    n->stype = ST_VOID;
    return ST_VOID;
}

SchemeType Analyzer::analyzeUnless(Node* n) {
    auto items = listToVec(n);
    if (items.size() < 2) {
        addError("unless: requer condição", n->line, n->col);
        n->stype = ST_VOID; return ST_VOID;
    }
    analyzeExpr(items[1]);
    for (size_t i = 2; i < items.size(); i++) analyzeExpr(items[i]);
    n->stype = ST_VOID;
    return ST_VOID;
}

SchemeType Analyzer::analyzeDo(Node* n) {
    auto items = listToVec(n);
    if (items.size() < 3) {
        addError("do: sintaxe inválida", n->line, n->col);
        n->stype = ST_VOID; return ST_VOID;
    }
    sym.pushScope();
    Node* varSpecs = items[1];
    while (varSpecs && varSpecs->kind == NK_PAIR) {
        auto sv = listToVec(varSpecs->car);
        if (!sv.empty() && sv[0]->kind == NK_SYMBOL) {
            SchemeType t = (sv.size() >= 2) ? analyzeExpr(sv[1]) : ST_UNKNOWN;
            sym.define(sv[0]->sval, t);
            if (sv.size() >= 3) analyzeExpr(sv[2]);
        }
        varSpecs = varSpecs->cdr;
    }
    auto testClause = listToVec(items[2]);
    if (!testClause.empty()) {
        analyzeExpr(testClause[0]);
        for (size_t i = 1; i < testClause.size(); i++) analyzeExpr(testClause[i]);
    }
    for (size_t i = 3; i < items.size(); i++) analyzeExpr(items[i]);
    sym.popScope();
    n->stype = ST_ANY;
    return ST_ANY;
}

SchemeType Analyzer::analyzeQuasi(Node* n) {
    // analysamos recursivamente mas aceitamos qualquer tipo
    if (n->cdr && n->cdr->kind == NK_PAIR)
        analyzeExpr(n->cdr->car);
    n->stype = ST_ANY;
    return ST_ANY;
}

SchemeType Analyzer::analyzeApp(Node* n) {
    SchemeType procType = analyzeExpr(n->car);
    if (procType != ST_PROCEDURE && procType != ST_ANY && procType != ST_UNKNOWN) {
        addError("chamada: a expressão na posição de operador não é um procedimento",
                 n->line, n->col);
    }
    // arity check when known
    SymEntry* entry = nullptr;
    if (n->car && n->car->kind == NK_SYMBOL)
        entry = sym.lookup(n->car->sval);

    int argCount = listLen(n->cdr);
    if (entry && entry->arity >= 0 && entry->arity != argCount) {
        std::ostringstream oss;
        oss << "'" << n->car->sval << "' espera " << entry->arity
            << " argumento(s), recebeu " << argCount;
        addError(oss.str(), n->line, n->col);
    }

    Node* args = n->cdr;
    while (args && args->kind == NK_PAIR) {
        analyzeExpr(args->car);
        args = args->cdr;
    }
    n->stype = (entry && entry->type != ST_UNKNOWN) ? entry->type : ST_ANY;
    return n->stype;
}

SchemeType Analyzer::quoteType(Node* n) {
    if (!n) return ST_NIL;
    switch (n->kind) {
        case NK_INTEGER:  return ST_INTEGER;
        case NK_FLOAT:    return ST_FLOAT;
        case NK_RATIONAL: return ST_RATIONAL;
        case NK_BOOLEAN:  return ST_BOOLEAN;
        case NK_STRING:   return ST_STRING;
        case NK_CHAR:     return ST_CHAR;
        case NK_SYMBOL:   return ST_SYMBOL;
        case NK_NIL:      return ST_NIL;
        case NK_PAIR:     return ST_LIST;
        case NK_VECTOR:   return ST_VECTOR;
        default:          return ST_ANY;
    }
}
