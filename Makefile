CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wno-unused-function -Wno-register -g

FLEX  = flex
BISON = bison

SRC      = src
SRCS_CPP = $(SRC)/ast.cpp $(SRC)/symtable.cpp \
           $(SRC)/analyzer.cpp $(SRC)/codegen.cpp $(SRC)/main.cpp
SRCS_GEN = $(SRC)/lex.yy.cpp $(SRC)/parser.tab.cpp
ALL_SRCS = $(SRCS_CPP) $(SRCS_GEN)
OBJS     = $(ALL_SRCS:.cpp=.o)

TARGET = compilador_scheme

# ── Regras principais ──────────────────────────────────────────

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# ── Geração Bison/Flex ─────────────────────────────────────────

$(SRC)/parser.tab.cpp $(SRC)/parser.tab.hpp: $(SRC)/parser.y
	$(BISON) -d --defines=$(SRC)/parser.tab.hpp -o $(SRC)/parser.tab.cpp $<

$(SRC)/lex.yy.cpp: $(SRC)/scanner.l $(SRC)/parser.tab.hpp
	$(FLEX) -o $(SRC)/lex.yy.cpp $<

# ── Compilação ─────────────────────────────────────────────────

$(SRC)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(SRC) -c $< -o $@

# ── Limpeza ────────────────────────────────────────────────────

clean:
	rm -f $(TARGET) \
	      $(SRC)/lex.yy.cpp \
	      $(SRC)/parser.tab.cpp \
	      $(SRC)/parser.tab.hpp \
	      $(SRC)/*.o

# ── Utilitários ────────────────────────────────────────────────

# Compila e executa um exemplo:   make run FILE=examples/valid_factorial.scm
run: $(TARGET)
	./$(TARGET) $(FILE) output.py
	@echo "─── Python gerado ───────────────────────────────────────"
	@cat output.py
	@echo "─── Resultado da execução ───────────────────────────────"
	@python3 output.py

# Apenas exibe o Python sem executar
show: $(TARGET)
	./$(TARGET) $(FILE)

.PHONY: all clean run show
