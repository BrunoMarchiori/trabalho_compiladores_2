#include "ast.hpp"
#include <sstream>

static Node* alloc(NodeKind k, int line, int col) {
    Node* n  = new Node{};
    n->kind  = k;
    n->stype = ST_UNKNOWN;
    n->line  = line;
    n->col   = col;
    n->car   = n->cdr = nullptr;
    return n;
}

Node* makeInteger(long long v, int line, int col) {
    Node* n = alloc(NK_INTEGER, line, col);
    n->ival  = v;
    n->stype = ST_INTEGER;
    return n;
}

Node* makeFloat(double v, int line, int col) {
    Node* n = alloc(NK_FLOAT, line, col);
    n->dval  = v;
    n->stype = ST_FLOAT;
    return n;
}

Node* makeRational(long long num, long long den, int line, int col) {
    Node* n  = alloc(NK_RATIONAL, line, col);
    n->rnum  = num;
    n->rden  = den;
    n->stype = ST_RATIONAL;
    return n;
}

Node* makeBoolean(bool v, int line, int col) {
    Node* n  = alloc(NK_BOOLEAN, line, col);
    n->bval  = v;
    n->stype = ST_BOOLEAN;
    return n;
}

Node* makeString(const std::string& v, int line, int col) {
    Node* n  = alloc(NK_STRING, line, col);
    n->sval  = v;
    n->stype = ST_STRING;
    return n;
}

Node* makeChar(const std::string& v, int line, int col) {
    Node* n  = alloc(NK_CHAR, line, col);
    n->sval  = v;
    n->stype = ST_CHAR;
    return n;
}

Node* makeSymbol(const std::string& v, int line, int col) {
    Node* n  = alloc(NK_SYMBOL, line, col);
    n->sval  = v;
    n->stype = ST_SYMBOL;
    return n;
}

Node* makeNil(int line, int col) {
    Node* n  = alloc(NK_NIL, line, col);
    n->stype = ST_NIL;
    return n;
}

Node* makePair(Node* a, Node* d, int line, int col) {
    Node* n  = alloc(NK_PAIR, line, col);
    n->car   = a;
    n->cdr   = d;
    n->stype = ST_PAIR;
    return n;
}

Node* makeVector(Node* items, int line, int col) {
    Node* n  = alloc(NK_VECTOR, line, col);
    n->car   = items;
    n->stype = ST_VECTOR;
    return n;
}

Node* makeProgram(int line, int col) {
    return alloc(NK_PROGRAM, line, col);
}

int listLen(Node* n) {
    int count = 0;
    while (n && n->kind == NK_PAIR) { count++; n = n->cdr; }
    return count;
}

Node* listGet(Node* n, int i) {
    while (n && n->kind == NK_PAIR && i-- > 0) n = n->cdr;
    return (n && n->kind == NK_PAIR) ? n->car : nullptr;
}

bool isProperList(Node* n) {
    while (n && n->kind == NK_PAIR) n = n->cdr;
    return n && n->kind == NK_NIL;
}

bool isSymbol(Node* n, const std::string& s) {
    return n && n->kind == NK_SYMBOL && n->sval == s;
}

std::vector<Node*> listToVec(Node* n) {
    std::vector<Node*> v;
    while (n && n->kind == NK_PAIR) {
        v.push_back(n->car);
        n = n->cdr;
    }
    return v;
}

std::string nodeToString(Node* n) {
    if (!n) return "#<null>";
    switch (n->kind) {
        case NK_INTEGER:  return std::to_string(n->ival);
        case NK_FLOAT: {
            std::ostringstream oss;
            oss << n->dval;
            return oss.str();
        }
        case NK_RATIONAL:
            return std::to_string(n->rnum) + "/" + std::to_string(n->rden);
        case NK_BOOLEAN:  return n->bval ? "#t" : "#f";
        case NK_STRING:   return "\"" + n->sval + "\"";
        case NK_CHAR:     return "#\\" + n->sval;
        case NK_SYMBOL:   return n->sval;
        case NK_NIL:      return "()";
        case NK_PAIR: {
            std::string s = "(";
            Node* cur = n;
            bool first = true;
            while (cur && cur->kind == NK_PAIR) {
                if (!first) s += " ";
                s += nodeToString(cur->car);
                cur   = cur->cdr;
                first = false;
            }
            if (cur && cur->kind != NK_NIL)
                s += " . " + nodeToString(cur);
            return s + ")";
        }
        case NK_VECTOR: {
            std::string s = "#(";
            Node* cur = n->car;
            bool first = true;
            while (cur && cur->kind == NK_PAIR) {
                if (!first) s += " ";
                s += nodeToString(cur->car);
                cur   = cur->cdr;
                first = false;
            }
            return s + ")";
        }
        case NK_PROGRAM: {
            std::string s;
            for (auto* c : n->children) s += nodeToString(c) + "\n";
            return s;
        }
    }
    return "#<unknown>";
}
