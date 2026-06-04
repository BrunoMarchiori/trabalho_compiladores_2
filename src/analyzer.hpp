#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "ast.hpp"
#include "symtable.hpp"
#include <string>
#include <vector>

class Analyzer {
    SymTable&                sym;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    void addError  (const std::string& msg, int line, int col);
    void addWarning(const std::string& msg, int line, int col);

    SchemeType analyzeExpr    (Node* n);
    SchemeType analyzeList    (Node* n);
    SchemeType analyzeDefine  (Node* n);
    SchemeType analyzeLambda  (Node* n);
    SchemeType analyzeIf      (Node* n);
    SchemeType analyzeCond    (Node* n);
    SchemeType analyzeCase    (Node* n);
    SchemeType analyzeLet     (Node* n, const std::string& kind);
    SchemeType analyzeNamedLet(Node* n);
    SchemeType analyzeBegin   (Node* n);
    SchemeType analyzeSetBang (Node* n);
    SchemeType analyzeAnd     (Node* n);
    SchemeType analyzeOr      (Node* n);
    SchemeType analyzeWhen    (Node* n);
    SchemeType analyzeUnless  (Node* n);
    SchemeType analyzeDo      (Node* n);
    SchemeType analyzeQuasi   (Node* n);
    void       analyzeQuasiDatum(Node* n, int depth);
    SchemeType analyzeApp     (Node* n);
    SchemeType quoteType      (Node* n);

    void analyzeBodyList(const std::vector<Node*>& body);
    void predeclareBody(const std::vector<Node*>& body);
    void checkBuiltinTypes(const std::string& name,
                           const std::vector<SchemeType>& argTypes,
                           Node* call);
    void checkArg(const std::string& proc, int argIndex, SchemeType actual,
                  const std::string& expected, bool ok, Node* call);

public:
    explicit Analyzer(SymTable& sym);

    bool analyze(Node* program);

    const std::vector<std::string>& getErrors()   const { return errors;   }
    const std::vector<std::string>& getWarnings()  const { return warnings; }
};

#endif
