#include "symtable.hpp"
#include <initializer_list>

void SymTable::defBuiltin(const std::string& name, SchemeType type, int arity) {
    scopes.back()[name] = {name, type, arity, true};
}

SymTable::SymTable() {
    scopes.push_back({});

    // aritmética (n-ário)
    for (const char* n : {"+", "-", "*", "/", "modulo", "remainder", "quotient",
                          "expt", "abs", "max", "min", "floor", "ceiling",
                          "truncate", "round", "sqrt", "gcd", "lcm"})
        defBuiltin(n, ST_NUMBER, -2);

    // comparação numérica (n-ário)
    for (const char* n : {"=", "<", ">", "<=", ">="})
        defBuiltin(n, ST_BOOLEAN, -2);

    // predicados numéricos (1 arg)
    for (const char* n : {"zero?", "positive?", "negative?", "odd?", "even?",
                          "number?", "integer?", "real?", "rational?", "complex?"})
        defBuiltin(n, ST_BOOLEAN, 1);
    defBuiltin("exact?",   ST_BOOLEAN, 1);
    defBuiltin("inexact?", ST_BOOLEAN, 1);
    defBuiltin("numerator",   ST_NUMBER, 1);
    defBuiltin("denominator", ST_NUMBER, 1);
    defBuiltin("exact->inexact", ST_FLOAT, 1);
    defBuiltin("inexact->exact", ST_RATIONAL, 1);

    // igualdade
    for (const char* n : {"eq?", "eqv?", "equal?"})
        defBuiltin(n, ST_BOOLEAN, 2);

    // booleano
    defBuiltin("not", ST_BOOLEAN, 1);
    defBuiltin("boolean?", ST_BOOLEAN, 1);

    // listas
    defBuiltin("cons",      ST_PAIR,    2);
    defBuiltin("car",       ST_ANY,     1);
    defBuiltin("cdr",       ST_ANY,     1);
    defBuiltin("list",      ST_LIST,   -2);
    defBuiltin("null?",     ST_BOOLEAN, 1);
    defBuiltin("pair?",     ST_BOOLEAN, 1);
    defBuiltin("list?",     ST_BOOLEAN, 1);
    defBuiltin("length",    ST_INTEGER, 1);
    defBuiltin("append",    ST_LIST,   -2);
    defBuiltin("reverse",   ST_LIST,    1);
    defBuiltin("list-ref",  ST_ANY,     2);
    defBuiltin("list-tail", ST_LIST,    2);
    defBuiltin("member",    ST_ANY,     2);
    defBuiltin("memq",      ST_ANY,     2);
    defBuiltin("memv",      ST_ANY,     2);
    defBuiltin("assoc",     ST_ANY,     2);
    defBuiltin("assq",      ST_ANY,     2);
    defBuiltin("assv",      ST_ANY,     2);
    defBuiltin("caar",      ST_ANY,     1);
    defBuiltin("cadr",      ST_ANY,     1);
    defBuiltin("cdar",      ST_ANY,     1);
    defBuiltin("cddr",      ST_ANY,     1);
    defBuiltin("caaar",     ST_ANY,     1);
    defBuiltin("caadr",     ST_ANY,     1);
    defBuiltin("caddr",     ST_ANY,     1);
    defBuiltin("cadddr",    ST_ANY,     1);
    defBuiltin("list-copy", ST_LIST,    1);

    // map / apply
    defBuiltin("map",       ST_LIST,   -2);
    defBuiltin("for-each",  ST_VOID,   -2);
    defBuiltin("apply",     ST_ANY,    -2);
    defBuiltin("filter",    ST_LIST,    2);
    defBuiltin("fold-left", ST_ANY,     3);
    defBuiltin("fold-right",ST_ANY,     3);
    defBuiltin("reduce",    ST_ANY,     3);

    // strings
    defBuiltin("string-length",   ST_INTEGER, 1);
    defBuiltin("string-ref",      ST_CHAR,    2);
    defBuiltin("string-append",   ST_STRING, -2);
    defBuiltin("substring",       ST_STRING,  3);
    defBuiltin("string->number",  ST_ANY,    -2);
    defBuiltin("number->string",  ST_STRING, -2);
    defBuiltin("string=?",        ST_BOOLEAN, 2);
    defBuiltin("string<?",        ST_BOOLEAN, 2);
    defBuiltin("string>?",        ST_BOOLEAN, 2);
    defBuiltin("string<=?",       ST_BOOLEAN, 2);
    defBuiltin("string>=?",       ST_BOOLEAN, 2);
    defBuiltin("string-ci=?",     ST_BOOLEAN, 2);
    defBuiltin("string-ci<?",     ST_BOOLEAN, 2);
    defBuiltin("string-ci>?",     ST_BOOLEAN, 2);
    defBuiltin("string-ci<=?",    ST_BOOLEAN, 2);
    defBuiltin("string-ci>=?",    ST_BOOLEAN, 2);
    defBuiltin("string->symbol",  ST_SYMBOL,  1);
    defBuiltin("symbol->string",  ST_STRING,  1);
    defBuiltin("string->list",    ST_LIST,    1);
    defBuiltin("list->string",    ST_STRING,  1);
    defBuiltin("string-upcase",   ST_STRING,  1);
    defBuiltin("string-downcase", ST_STRING,  1);
    defBuiltin("string-copy",     ST_STRING,  1);
    defBuiltin("string",          ST_STRING, -2);
    defBuiltin("make-string",     ST_STRING, -2);
    defBuiltin("string-contains", ST_ANY,     2);
    defBuiltin("string-split",    ST_LIST,    2);
    defBuiltin("string?",         ST_BOOLEAN, 1);
    defBuiltin("symbol?",         ST_BOOLEAN, 1);

    // chars
    defBuiltin("char->integer",    ST_INTEGER, 1);
    defBuiltin("integer->char",    ST_CHAR,    1);
    defBuiltin("char=?",           ST_BOOLEAN, 2);
    defBuiltin("char<?",           ST_BOOLEAN, 2);
    defBuiltin("char>?",           ST_BOOLEAN, 2);
    defBuiltin("char<=?",          ST_BOOLEAN, 2);
    defBuiltin("char>=?",          ST_BOOLEAN, 2);
    defBuiltin("char-ci=?",        ST_BOOLEAN, 2);
    defBuiltin("char-ci<?",        ST_BOOLEAN, 2);
    defBuiltin("char-ci>?",        ST_BOOLEAN, 2);
    defBuiltin("char-ci<=?",       ST_BOOLEAN, 2);
    defBuiltin("char-ci>=?",       ST_BOOLEAN, 2);
    defBuiltin("char-alphabetic?", ST_BOOLEAN, 1);
    defBuiltin("char-numeric?",    ST_BOOLEAN, 1);
    defBuiltin("char-whitespace?", ST_BOOLEAN, 1);
    defBuiltin("char-upcase",      ST_CHAR,    1);
    defBuiltin("char-downcase",    ST_CHAR,    1);
    defBuiltin("char?",            ST_BOOLEAN, 1);

    // vetores
    defBuiltin("make-vector",  ST_VECTOR,  -2);
    defBuiltin("vector",       ST_VECTOR,  -2);
    defBuiltin("vector-ref",   ST_ANY,      2);
    defBuiltin("vector-set!",  ST_VOID,     3);
    defBuiltin("vector-length",ST_INTEGER,  1);
    defBuiltin("vector->list", ST_LIST,     1);
    defBuiltin("list->vector", ST_VECTOR,   1);
    defBuiltin("vector-fill!", ST_VOID,     2);
    defBuiltin("vector?",      ST_BOOLEAN,  1);

    // I/O
    defBuiltin("display",           ST_VOID,  -2);
    defBuiltin("newline",           ST_VOID,   0);
    defBuiltin("write",             ST_VOID,  -2);
    defBuiltin("read",              ST_ANY,    0);
    defBuiltin("read-char",         ST_CHAR,   0);
    defBuiltin("write-char",        ST_VOID,   1);
    defBuiltin("open-input-file",   ST_PORT,   1);
    defBuiltin("open-output-file",  ST_PORT,   1);
    defBuiltin("close-input-port",  ST_VOID,   1);
    defBuiltin("close-output-port", ST_VOID,   1);
    defBuiltin("current-input-port",ST_PORT,   0);
    defBuiltin("current-output-port",ST_PORT,  0);
    defBuiltin("eof-object?",       ST_BOOLEAN,1);
    defBuiltin("port?",             ST_BOOLEAN,1);
    defBuiltin("input-port?",       ST_BOOLEAN,1);
    defBuiltin("output-port?",      ST_BOOLEAN,1);

    // controle
    defBuiltin("error",           ST_VOID,  -2);
    defBuiltin("void",            ST_VOID,   0);
    defBuiltin("values",          ST_ANY,   -2);
    defBuiltin("call-with-values",ST_ANY,    2);
    defBuiltin("call/cc",         ST_ANY,    1);
    defBuiltin("call-with-current-continuation", ST_ANY, 1);
    defBuiltin("dynamic-wind",    ST_ANY,    3);
    defBuiltin("force",           ST_ANY,    1);
    defBuiltin("raise",           ST_VOID,   1);
    defBuiltin("with-exception-handler", ST_ANY, 2);

    // procedure?
    defBuiltin("procedure?",      ST_BOOLEAN,1);
}

void SymTable::pushScope() {
    scopes.push_back({});
}

void SymTable::popScope() {
    if (scopes.size() > 1) scopes.pop_back();
}

void SymTable::define(const std::string& name, SchemeType type, int arity, bool builtin) {
    scopes.back()[name] = {name, type, arity, builtin};
}

SymEntry* SymTable::lookup(const std::string& name) {
    for (int i = (int)scopes.size() - 1; i >= 0; i--) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return &it->second;
    }
    return nullptr;
}

bool SymTable::isDefined(const std::string& name) {
    return lookup(name) != nullptr;
}
