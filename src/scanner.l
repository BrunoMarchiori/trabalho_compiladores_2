%{
#include "ast.hpp"
#include "parser.tab.hpp"
#include <cstdlib>
#include <cstring>
#include <cerrno>

/* Prototipo exigido pelo Bison */
extern YYSTYPE yylval;

/* processa escapes dentro de strings Scheme */
static std::string processString(const char* raw) {
    std::string s;
    const char* p = raw + 1;               /* pula aspas de abertura */
    while (*p && !(*p == '"' && *(p-1) != '\\')) {
        if (*p == '\\') {
            p++;
            switch (*p) {
                case 'n':  s += '\n'; break;
                case 't':  s += '\t'; break;
                case 'r':  s += '\r'; break;
                case '"':  s += '"';  break;
                case '\\': s += '\\'; break;
                case '0':  s += '\0'; break;
                case 'a':  s += '\a'; break;
                case 'b':  s += '\b'; break;
                default:   s += *p;   break;
            }
        } else {
            s += *p;
        }
        p++;
    }
    return s;
}
%}

%option noyywrap
%option yylineno
%option nounput
%option noinput

/* estados para comentários de bloco */
%x BLOCK_COMMENT

/* ── Definições auxiliares ─────────────────────────────── */
DIGIT        [0-9]
HEXDIGIT     [0-9a-fA-F]
SIGN         [+-]
LETTER       [a-zA-Z]
SPECIAL_INI  [!$%&*/:<=>?^_~]
INITIAL      ({LETTER}|{SPECIAL_INI})
SUBSEQ       ({INITIAL}|{DIGIT}|[+\-.@])

%%

 /* ── Comentários ─────────────────────────────────────── */
";"[^\n]*                   { /* comentário de linha: ignora */ }
"#|"                        { BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>"|#"         { BEGIN(INITIAL); }
<BLOCK_COMMENT>\n           { /* yylineno já atualiza */ }
<BLOCK_COMMENT>.            { /* ignora conteúdo do bloco */ }
"#;"[^\n]*\n                { /* datum comment simplificado: ignora linha */ }

 /* ── Espaço em branco ───────────────────────────────── */
[ \t\r\n]+                  { /* ignora */ }

 /* ── Booleanos (antes de identificadores) ──────────── */
"#true"|"#t"                { yylval.bval = 1; return TOK_BOOLEAN; }
"#false"|"#f"               { yylval.bval = 0; return TOK_BOOLEAN; }

 /* ── Vetores ────────────────────────────────────────── */
"#("                        { return TOK_HASH_LPAREN; }

 /* ── Caracteres ─────────────────────────────────────── */
"#\\"[a-zA-Z][a-zA-Z0-9]*  {
    /* caractere nomeado: #\space, #\newline, etc. */
    yylval.sval = strdup(yytext + 2);
    return TOK_CHAR;
}
"#\\".                      {
    /* caractere simples: #\a */
    char buf[2] = { yytext[2], '\0' };
    yylval.sval = strdup(buf);
    return TOK_CHAR;
}

 /* ── Strings ─────────────────────────────────────────── */
\"([^\"\\]|\\.)*\"          {
    std::string s = processString(yytext);
    yylval.sval = strdup(s.c_str());
    return TOK_STRING;
}

 /* ── Números com prefixo de base ─────────────────────── */
"#x"{HEXDIGIT}+             {
    yylval.ival = (long long)strtoll(yytext+2, nullptr, 16);
    return TOK_INTEGER;
}
"#o"[0-7]+                  {
    yylval.ival = (long long)strtoll(yytext+2, nullptr, 8);
    return TOK_INTEGER;
}
"#b"[01]+                   {
    yylval.ival = (long long)strtoll(yytext+2, nullptr, 2);
    return TOK_INTEGER;
}
"#d"{SIGN}?{DIGIT}+         {
    yylval.ival = (long long)strtoll(yytext+2, nullptr, 10);
    return TOK_INTEGER;
}
"#e"|"#i"                   { /* prefixo de exactidão: ignora por ora */ }

 /* ── Números (ordem importa: float > racional > inteiro) */
{SIGN}?{DIGIT}+"."{DIGIT}*([eE]{SIGN}{DIGIT}+)?  {
    yylval.dval = atof(yytext);
    return TOK_FLOAT;
}
{SIGN}?"."{DIGIT}+([eE]{SIGN}{DIGIT}+)?           {
    yylval.dval = atof(yytext);
    return TOK_FLOAT;
}
{SIGN}?{DIGIT}+[eE]{SIGN}?{DIGIT}+               {
    yylval.dval = atof(yytext);
    return TOK_FLOAT;
}
{SIGN}?{DIGIT}+"/"[1-9]{DIGIT}*                  {
    yylval.sval = strdup(yytext);   /* "num/den" como string */
    return TOK_RATIONAL;
}
{SIGN}?{DIGIT}+                                   {
    yylval.ival = (long long)strtoll(yytext, nullptr, 10);
    return TOK_INTEGER;
}

 /* ── Identificadores / símbolos ──────────────────────── */
{INITIAL}{SUBSEQ}*           {
    yylval.sval = strdup(yytext);
    return TOK_SYMBOL;
}
"->"({SUBSEQ})*              {
    yylval.sval = strdup(yytext);
    return TOK_SYMBOL;
}
"..."                        {
    yylval.sval = strdup("...");
    return TOK_SYMBOL;
}
"+"                          {
    yylval.sval = strdup("+");
    return TOK_SYMBOL;
}
"-"                          {
    yylval.sval = strdup("-");
    return TOK_SYMBOL;
}

 /* ── Pontuação ───────────────────────────────────────── */
"("                          { return TOK_LPAREN;     }
")"                          { return TOK_RPAREN;     }
"["                          { return TOK_LBRACKET;   }
"]"                          { return TOK_RBRACKET;   }
",@"                         { return TOK_UNQUOTE_SPL; }
","                          { return TOK_UNQUOTE;    }
"`"                          { return TOK_QUASIQUOTE; }
"'"                          { return TOK_QUOTE;      }
"."                          { return TOK_DOT;        }

 /* ── Erro ────────────────────────────────────────────── */
.                            {
    fprintf(stderr, "Erro léxico [linha %d]: caractere inesperado '%s'\n",
            yylineno, yytext);
}

%%
