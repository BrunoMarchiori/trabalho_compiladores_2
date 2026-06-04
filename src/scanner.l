%{
#include "ast.hpp"
#include "parser.tab.hpp"
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cctype>
#include <sstream>
#include <string>

/*
 * yylval e a variavel global criada pelo Bison para transportar o valor
 * semantico do token reconhecido pelo scanner ate o parser. Cada regra Flex
 * preenche o campo correto antes de retornar um TOK_*.
 */
extern YYSTYPE yylval;

/*
 * Remove as aspas externas de uma string Scheme e converte escapes conhecidos
 * para os caracteres reais que serao armazenados na AST.
 *
 * Exemplo:
 *   "a\nb"  ->  a, quebra de linha, b
 *
 * Observacao: alguns escapes aqui sao extensoes praticas do projeto; o R4RS
 * formaliza um conjunto menor.
 */
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

/*
 * Normaliza texto ASCII para minusculas. Scheme/R4RS trata identificadores de
 * forma case-insensitive, entao simbolos como FOO e foo devem representar o
 * mesmo identificador interno.
 */
static std::string lowerAscii(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (unsigned char c : in)
        out += (char)std::tolower(c);
    return out;
}

/*
 * Duplica uma string normalizada para minusculas em memoria C.
 *
 * O parser usa char* nos tokens vindos do Flex/Bison; por isso retornamos uma
 * copia alocada com strdup, que depois e liberada nas acoes do parser.
 */
static char* dupLower(const char* raw) {
    std::string s = lowerAscii(raw);
    return strdup(s.c_str());
}

/*
 * Extrai o valor de um literal de caractere Scheme.
 *
 * Entrada do scanner:
 *   #\a       -> "a"
 *   #\space   -> "space"
 *   #\newline -> "newline"
 *
 * Nomes padronizados sao normalizados para minusculas; caracteres simples
 * preservam o valor lido.
 */
static char* dupCharToken(const char* raw) {
    std::string value = raw + 2;
    std::string lower = lowerAscii(value);
    if (lower == "space" || lower == "newline")
        return strdup(lower.c_str());
    return strdup(value.c_str());
}

/*
 * Remove prefixos numericos R4RS de radix/exatidao e informa a base.
 *
 * Exemplos:
 *   #b1010   -> corpo "1010", base 2
 *   #e#x0a   -> corpo "0a", base 16
 *   #i10     -> corpo "10", base 10
 *
 * Prefixos de exatidao (#e/#i) sao reconhecidos, mas a AST atual ainda guarda
 * somente o valor numerico convertido.
 */
static std::string stripNumberPrefixes(const char* raw, int& base) {
    std::string s = lowerAscii(raw);
    base = 10;
    size_t pos = 0;

    while (pos + 1 < s.size() && s[pos] == '#') {
        char p = s[pos + 1];
        if (p == 'b')      { base = 2;  pos += 2; }
        else if (p == 'o') { base = 8;  pos += 2; }
        else if (p == 'd') { base = 10; pos += 2; }
        else if (p == 'x') { base = 16; pos += 2; }
        else if (p == 'e' || p == 'i') { pos += 2; }
        else break;
    }

    return s.substr(pos);
}

/*
 * Converte um inteiro possivelmente prefixado para long long.
 *
 * A base e definida por stripNumberPrefixes. Assim a mesma funcao serve para
 * literais binarios, octais, decimais e hexadecimais.
 */
static long long parsePrefixedInteger(const char* raw) {
    int base = 10;
    std::string body = stripNumberPrefixes(raw, base);
    return (long long)strtoll(body.c_str(), nullptr, base);
}

/*
 * Converte um racional possivelmente prefixado para a forma decimal "n/d".
 *
 * O parser ja espera receber o racional como string "num/den"; aqui fazemos a
 * conversao quando o literal veio em outra base, como #xA/#x2.
 */
static char* dupPrefixedRational(const char* raw) {
    int base = 10;
    std::string body = stripNumberPrefixes(raw, base);
    size_t slash = body.find('/');
    long long n = 0, d = 1;
    if (slash != std::string::npos) {
        n = (long long)strtoll(body.substr(0, slash).c_str(), nullptr, base);
        d = (long long)strtoll(body.substr(slash + 1).c_str(), nullptr, base);
    }
    std::ostringstream oss;
    oss << n << "/" << d;
    return strdup(oss.str().c_str());
}

/*
 * Converte um decimal inexato para double.
 *
 * O R4RS aceita marcadores de expoente alem de e/E, como s, f, d e l. Para
 * reaproveitar atof, esses marcadores sao normalizados para 'e'.
 */
static double parsePrefixedFloat(const char* raw) {
    int base = 10;
    std::string body = stripNumberPrefixes(raw, base);
    for (char& c : body) {
        if (c == 's' || c == 'f' || c == 'd' || c == 'l')
            c = 'e';
    }
    return atof(body.c_str());
}
%}

/*
 * Opcoes do Flex:
 * - noyywrap: nao chama yywrap ao fim do arquivo.
 * - yylineno: atualiza yylineno automaticamente.
 * - nounput/noinput: evita gerar funcoes nao usadas.
 * - caseless: deixa o reconhecimento dos padroes case-insensitive.
 */
%option noyywrap
%option yylineno
%option nounput
%option noinput
%option caseless

/* Estado exclusivo usado enquanto o scanner esta dentro de um comentario #| |#. */
%x BLOCK_COMMENT

/*
 * Definicoes auxiliares de regex.
 *
 * Flex permite nomear pedacos de expressoes regulares para tornar as regras
 * principais mais legiveis. Esses nomes nao geram tokens sozinhos; eles apenas
 * sao reutilizados nas regras abaixo.
 */
DIGIT        [0-9]
BINDIGIT     [01]
OCTDIGIT     [0-7]
HEXDIGIT     [0-9a-fA-F]
SIGN         [+-]
LETTER       [a-zA-Z]
SPECIAL_INI  [!$%&*/:<=>?^_~]
INITIAL      ({LETTER}|{SPECIAL_INI})
SUBSEQ       ({INITIAL}|{DIGIT}|[+\-.])
EXACTNESS    "#"[ei]
RADIX2       "#"[b]
RADIX8       "#"[o]
RADIX10      "#"[d]
RADIX16      "#"[x]
PREFIX2      (({EXACTNESS}{RADIX2})|({RADIX2}{EXACTNESS})|{RADIX2})
PREFIX8      (({EXACTNESS}{RADIX8})|({RADIX8}{EXACTNESS})|{RADIX8})
PREFIX10     (({EXACTNESS}{RADIX10})|({RADIX10}{EXACTNESS})|{RADIX10}|{EXACTNESS})
PREFIX16     (({EXACTNESS}{RADIX16})|({RADIX16}{EXACTNESS})|{RADIX16})
EXP_MARK     [esfdl]

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
    yylval.sval = dupCharToken(yytext);
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

 /* ── Números com prefixos R4RS de base/exatidão ─────── */
 /*
  * As regras numericas ficam antes de simbolos para que sequencias como #x10,
  * 1/2 e 1.5 sejam classificadas corretamente.
  *
  * A ordem tambem importa: racionais e floats precisam vir antes de inteiros,
  * pois seus prefixos iniciais tambem poderiam casar com uma regra menor.
  */
 /* Racionais em base 2: #b101/10, com #e/#i opcional via PREFIX2. */
{PREFIX2}{SIGN}?{BINDIGIT}+"/"{BINDIGIT}+        {
    yylval.sval = dupPrefixedRational(yytext);
    return TOK_RATIONAL;
}
 /* Racionais em base 8. */
{PREFIX8}{SIGN}?{OCTDIGIT}+"/"{OCTDIGIT}+        {
    yylval.sval = dupPrefixedRational(yytext);
    return TOK_RATIONAL;
}
 /* Racionais em base 16. */
{PREFIX16}{SIGN}?{HEXDIGIT}+"/"{HEXDIGIT}+       {
    yylval.sval = dupPrefixedRational(yytext);
    return TOK_RATIONAL;
}
 /* Racionais decimais, com prefixo decimal/exatidao opcional. */
({PREFIX10})?{SIGN}?{DIGIT}+"/"[1-9]{DIGIT}*     {
    yylval.sval = dupPrefixedRational(yytext);
    return TOK_RATIONAL;
}
 /* Inteiros em base 2. */
{PREFIX2}{SIGN}?{BINDIGIT}+                      {
    yylval.ival = parsePrefixedInteger(yytext);
    return TOK_INTEGER;
}
 /* Inteiros em base 8. */
{PREFIX8}{SIGN}?{OCTDIGIT}+                      {
    yylval.ival = parsePrefixedInteger(yytext);
    return TOK_INTEGER;
}
 /* Inteiros em base 16. */
{PREFIX16}{SIGN}?{HEXDIGIT}+                     {
    yylval.ival = parsePrefixedInteger(yytext);
    return TOK_INTEGER;
}
 /* Decimais com ponto, como 1.0 ou #i1.0e2. */
({PREFIX10})?{SIGN}?{DIGIT}+"."{DIGIT}*({EXP_MARK}{SIGN}?{DIGIT}+)?  {
    yylval.dval = parsePrefixedFloat(yytext);
    return TOK_FLOAT;
}
 /* Decimais iniciados por ponto, como .5. */
({PREFIX10})?{SIGN}?"."{DIGIT}+({EXP_MARK}{SIGN}?{DIGIT}+)?           {
    yylval.dval = parsePrefixedFloat(yytext);
    return TOK_FLOAT;
}
 /* Decimais com expoente e sem ponto, como 1e3, 1s3, 1d3. */
({PREFIX10})?{SIGN}?{DIGIT}+{EXP_MARK}{SIGN}?{DIGIT}+                 {
    yylval.dval = parsePrefixedFloat(yytext);
    return TOK_FLOAT;
}
 /* Inteiros decimais comuns ou com prefixo #d/#e/#i. */
({PREFIX10})?{SIGN}?{DIGIT}+                                           {
    yylval.ival = parsePrefixedInteger(yytext);
    return TOK_INTEGER;
}

 /* ── Identificadores / símbolos ──────────────────────── */
{INITIAL}{SUBSEQ}*           {
    yylval.sval = dupLower(yytext);
    return TOK_SYMBOL;
}
"->"({SUBSEQ})*              {
    yylval.sval = dupLower(yytext);
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
