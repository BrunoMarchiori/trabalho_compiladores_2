#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>

enum NodeKind {
    NK_INTEGER,
    NK_FLOAT,
    NK_RATIONAL,
    NK_BOOLEAN,
    NK_STRING,
    NK_CHAR,
    NK_SYMBOL,
    NK_NIL,
    NK_PAIR,
    NK_VECTOR,
    NK_PROGRAM,
};

enum SchemeType {
    ST_UNKNOWN = 0,
    ST_ANY,
    ST_INTEGER,
    ST_FLOAT,
    ST_RATIONAL,
    ST_NUMBER,
    ST_BOOLEAN,
    ST_STRING,
    ST_CHAR,
    ST_SYMBOL,
    ST_LIST,
    ST_VECTOR,
    ST_PAIR,
    ST_PROCEDURE,
    ST_VOID,
    ST_NIL,
    ST_PORT,
};

struct Node {
    NodeKind   kind;
    SchemeType stype;
    int        line, col;

    long long  ival;
    double     dval;
    long long  rnum, rden;
    bool       bval;
    std::string sval;

    Node* car;
    Node* cdr;

    std::vector<Node*> children; // NK_PROGRAM only
};

Node* makeInteger (long long v,  int line = 0, int col = 0);
Node* makeFloat   (double v,     int line = 0, int col = 0);
Node* makeRational(long long n, long long d, int line = 0, int col = 0);
Node* makeBoolean (bool v,       int line = 0, int col = 0);
Node* makeString  (const std::string& v, int line = 0, int col = 0);
Node* makeChar    (const std::string& v, int line = 0, int col = 0);
Node* makeSymbol  (const std::string& v, int line = 0, int col = 0);
Node* makeNil     (int line = 0, int col = 0);
Node* makePair    (Node* a, Node* d, int line = 0, int col = 0);
Node* makeVector  (Node* items,  int line = 0, int col = 0);
Node* makeProgram (int line = 0, int col = 0);

int                 listLen     (Node* n);
Node*               listGet     (Node* n, int i);
bool                isProperList(Node* n);
bool                isSymbol    (Node* n, const std::string& s);
std::vector<Node*>  listToVec   (Node* n);

std::string nodeToString(Node* n);

#endif
