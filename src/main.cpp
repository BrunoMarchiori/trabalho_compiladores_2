#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>

#include "ast.hpp"
#include "symtable.hpp"
#include "analyzer.hpp"
#include "codegen.hpp"

/* exportados pelo parser Bison e scanner Flex */
extern int   yyparse();
extern FILE* yyin;
extern Node* parseResult;
extern int   parseErrors;

static void usage(const char* prog) {
    std::cerr << "Uso: " << prog << " <entrada.scm> [saida.py]\n";
    std::cerr << "  Se saida.py for omitido, imprime na stdout.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) { usage(argv[0]); return 1; }

    const std::string inputPath = argv[1];
    const std::string outputPath = (argc >= 3) ? argv[2] : "";

    /* ── 1. Parsing ───────────────────────────────────────── */
    yyin = fopen(inputPath.c_str(), "r");
    if (!yyin) {
        std::cerr << "Erro: não foi possível abrir '" << inputPath << "'\n";
        return 1;
    }
    yyparse();
    fclose(yyin);

    if (parseErrors > 0 || !parseResult) {
        std::cerr << "Compilação interrompida: " << parseErrors
                  << " erro(s) sintático(s).\n";
        return 1;
    }

    /* ── 2. Análise semântica ─────────────────────────────── */
    SymTable symTable;
    Analyzer analyzer(symTable);
    bool ok = analyzer.analyze(parseResult);

    for (const auto& w : analyzer.getWarnings())
        std::cout << "[AVISO] " << w << "\n";

    if (!ok) {
        for (const auto& e : analyzer.getErrors())
            std::cerr << e << "\n";
        std::cerr << "Compilação interrompida: "
                  << analyzer.getErrors().size() << " erro(s) semântico(s).\n";
        return 1;
    }

    /* ── 3. Geração de código Python ──────────────────────── */
    CodeGen codegen;
    std::string pythonCode = codegen.generate(parseResult);

    /* ── 4. Saída ─────────────────────────────────────────── */
    if (outputPath.empty()) {
        std::cout << pythonCode;
    } else {
        std::ofstream out(outputPath);
        if (!out) {
            std::cerr << "Erro: não foi possível criar '" << outputPath << "'\n";
            return 1;
        }
        out << pythonCode;
        std::cout << "Código gerado em: " << outputPath << "\n";
    }

    return 0;
}
