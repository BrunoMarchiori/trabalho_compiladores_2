#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "ast.hpp"
#include <string>
#include <set>
#include <vector>
#include <map>

class CodeGen {
    int tempCount   = 0;
    int lambdaCount = 0;

    // rastreia escopos para nonlocal/global
    struct Scope {
        std::set<std::string> params;
        std::set<std::string> locals;
        bool                  isTopLevel;
    };
    std::vector<Scope> scopes;
    std::vector<std::map<std::string, std::string>> boxedAliases;

    std::string newTemp();
    std::string newLambda();

    // converte identificador Scheme para Python
    std::string pyId(const std::string& s);
    bool lookupBoxedAlias(const std::string& name, std::string& box) const;

    // coleta targets de set! (sem descer em lambdas internas)
    void collectMutations(Node* n, std::set<std::string>& out);
    // coleta nomes definidos via (define ...) no nível imediato do corpo
    void collectLocalDefs(const std::vector<Node*>& body, std::set<std::string>& out);
    // retorna vars que precisam de nonlocal/global
    std::set<std::string> needsNonlocal(const std::vector<Node*>& body,
                                         const std::set<std::string>& params,
                                         const std::set<std::string>& locals);
    bool containsSetBang(Node* n);

    std::string ind(int level);

    // geração de expressão Python
    std::string genExpr(Node* n);

    // geração de statement(s) Python
    std::string genStmt(Node* n, int ind_level);

    // expressões especiais
    std::string genIfExpr    (Node* n);
    std::string genCondExpr  (Node* n);
    std::string genCaseExpr  (Node* n);
    std::string genAndExpr   (Node* n);
    std::string genOrExpr    (Node* n);
    std::string genLetExpr   (Node* n, const std::string& kind);
    std::string genNamedLetExpr(Node* n);
    std::string genBeginExpr (const std::vector<Node*>& body);
    std::string genLambdaExpr(Node* n);
    std::string genDoExpr    (Node* n);
    std::string genQuoteExpr (Node* data);
    std::string genQuasiExpr (Node* n);
    std::string genAppExpr   (Node* n);

    // statements especiais
    std::string genDefineStmt   (Node* n, int il);
    std::string genSetStmt      (Node* n, int il);
    std::string genIfStmt       (Node* n, int il);
    std::string genCondStmt     (Node* n, int il);
    std::string genCaseStmt     (Node* n, int il);
    std::string genWhenStmt     (Node* n, int il);
    std::string genUnlessStmt   (Node* n, int il);
    std::string genLetStmt      (Node* n, const std::string& kind, int il);
    std::string genNamedLetStmt (Node* n, int il);
    std::string genDoStmt       (Node* n, int il, bool withReturn = false);
    std::string genBeginStmt    (const std::vector<Node*>& body, int il, bool withReturn = false);
    std::string genFuncBody     (const std::vector<Node*>& body, int il,
                                  const std::set<std::string>& params);

    // helper: formata lista de parâmetros
    std::string fmtParams(Node* params);

    static const std::map<std::string, std::string> BUILTINS;

public:
    std::string generate(Node* program);
};

#endif
