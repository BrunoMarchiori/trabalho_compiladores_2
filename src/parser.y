%{
#include "ast.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* declarações externas do scanner */
extern int  yylex();
extern int  yylineno;
void yyerror(const char* msg);

/* raiz da AST; acessada pelo main.cpp */
Node* parseResult = nullptr;

/* contador de erros de parsing */
int parseErrors = 0;
%}

/* ── Tipos do union ────────────────────────────────────── */
%union {
    long long  ival;
    double     dval;
    int        bval;
    char*      sval;   /* symbol, string, char, rational-as-string */
    Node*      node;
}

/* ── Tokens ─────────────────────────────────────────────── */
%token <ival>  TOK_INTEGER
%token <dval>  TOK_FLOAT
%token <sval>  TOK_RATIONAL
%token <bval>  TOK_BOOLEAN
%token <sval>  TOK_STRING
%token <sval>  TOK_SYMBOL
%token <sval>  TOK_CHAR

%token TOK_LPAREN TOK_RPAREN
%token TOK_LBRACKET TOK_RBRACKET
%token TOK_HASH_LPAREN
%token TOK_QUOTE TOK_QUASIQUOTE
%token TOK_UNQUOTE TOK_UNQUOTE_SPL
%token TOK_DOT

/* ── Tipos dos não-terminais ────────────────────────────── */
%type <node> program datum datum_list atom list vector abbreviation

%%

/* ── Gramática ──────────────────────────────────────────── */

program
    : datum_list
        {
            Node* prog = makeProgram(1, 1);
            /* converte a lista ligada de datums em vetor de filhos */
            Node* cur = $1;
            while (cur && cur->kind == NK_PAIR) {
                prog->children.push_back(cur->car);
                cur = cur->cdr;
            }
            parseResult = prog;
        }
    ;

datum_list
    : /* vazio */        { $$ = makeNil(yylineno, 0); }
    | datum_list datum   { $$ = makePair($2, $1, yylineno, 0); /* inverte ao final */ }
    ;

datum
    : atom
    | list
    | vector
    | abbreviation
    ;

atom
    : TOK_INTEGER  { $$ = makeInteger($1,  yylineno, 0); }
    | TOK_FLOAT    { $$ = makeFloat($1,    yylineno, 0); }
    | TOK_RATIONAL {
        /* "num/den" → separa */
        long long n = 0, d = 1;
        char* slash = strchr($1, '/');
        if (slash) {
            *slash = '\0';
            n = atoll($1);
            d = atoll(slash+1);
        }
        free($1);
        $$ = makeRational(n, d, yylineno, 0);
    }
    | TOK_BOOLEAN  { $$ = makeBoolean($1 != 0, yylineno, 0); }
    | TOK_STRING   { $$ = makeString($1, yylineno, 0); free($1); }
    | TOK_CHAR     { $$ = makeChar($1,   yylineno, 0); free($1); }
    | TOK_SYMBOL   { $$ = makeSymbol($1, yylineno, 0); free($1); }
    ;

list
    : TOK_LPAREN datum_list TOK_RPAREN
        {
            /* datum_list está na ordem inversa; revertemos */
            Node* rev = makeNil(yylineno, 0);
            Node* cur = $2;
            while (cur && cur->kind == NK_PAIR) {
                rev = makePair(cur->car, rev, yylineno, 0);
                cur = cur->cdr;
            }
            $$ = rev;
        }
    | TOK_LPAREN datum_list TOK_DOT datum TOK_RPAREN
        {
            /* par dotado: (a b . c) */
            Node* rev = $4;
            Node* cur = $2;
            while (cur && cur->kind == NK_PAIR) {
                rev = makePair(cur->car, rev, yylineno, 0);
                cur = cur->cdr;
            }
            $$ = rev;
        }
    | TOK_LBRACKET datum_list TOK_RBRACKET
        {
            /* colchetes equivalentes a parênteses em Scheme */
            Node* rev = makeNil(yylineno, 0);
            Node* cur = $2;
            while (cur && cur->kind == NK_PAIR) {
                rev = makePair(cur->car, rev, yylineno, 0);
                cur = cur->cdr;
            }
            $$ = rev;
        }
    ;

vector
    : TOK_HASH_LPAREN datum_list TOK_RPAREN
        {
            Node* rev = makeNil(yylineno, 0);
            Node* cur = $2;
            while (cur && cur->kind == NK_PAIR) {
                rev = makePair(cur->car, rev, yylineno, 0);
                cur = cur->cdr;
            }
            $$ = makeVector(rev, yylineno, 0);
        }
    ;

abbreviation
    : TOK_QUOTE datum
        {
            /* 'x → (quote x) */
            Node* nil  = makeNil(yylineno, 0);
            Node* tail = makePair($2, nil, yylineno, 0);
            $$ = makePair(makeSymbol("quote", yylineno, 0), tail, yylineno, 0);
        }
    | TOK_QUASIQUOTE datum
        {
            Node* nil  = makeNil(yylineno, 0);
            Node* tail = makePair($2, nil, yylineno, 0);
            $$ = makePair(makeSymbol("quasiquote", yylineno, 0), tail, yylineno, 0);
        }
    | TOK_UNQUOTE datum
        {
            Node* nil  = makeNil(yylineno, 0);
            Node* tail = makePair($2, nil, yylineno, 0);
            $$ = makePair(makeSymbol("unquote", yylineno, 0), tail, yylineno, 0);
        }
    | TOK_UNQUOTE_SPL datum
        {
            Node* nil  = makeNil(yylineno, 0);
            Node* tail = makePair($2, nil, yylineno, 0);
            $$ = makePair(makeSymbol("unquote-splicing", yylineno, 0), tail, yylineno, 0);
        }
    ;

%%

void yyerror(const char* msg) {
    fprintf(stderr, "Erro sintático [linha %d]: %s\n", yylineno, msg);
    parseErrors++;
}
