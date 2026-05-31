#ifndef SYMTABLE_HPP
#define SYMTABLE_HPP

#include "ast.hpp"
#include <map>
#include <string>
#include <vector>

struct SymEntry {
    std::string name;
    SchemeType  type;
    int         arity;    // -1=desconhecido, -2=variádico, >=0=fixo
    bool        isBuiltin;
};

class SymTable {
    std::vector<std::map<std::string, SymEntry>> scopes;

    void defBuiltin(const std::string& name, SchemeType type, int arity);

public:
    SymTable();

    void pushScope();
    void popScope();
    void define(const std::string& name, SchemeType type = ST_ANY,
                int arity = -1, bool builtin = false);

    SymEntry* lookup   (const std::string& name);
    bool      isDefined(const std::string& name);
    int       depth()  const { return (int)scopes.size(); }
};

#endif
