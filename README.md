# Compilador Scheme para Python

Trabalho da segunda entrega da disciplina de Compiladores.

O projeto implementa um compilador para um subconjunto da linguagem Scheme. A entrada e um programa `.scm`, que passa por scanner, parser bottom-up, construção de AST, verificação semântica e geração de código Python.

## Objetivo

A entrega atende ao requisito de implementar:

- scanner usando Flex;
- parser bottom-up usando Bison;
- verificacao de tipos e contexto de identificadores;
- geração de código Python a partir de programas Scheme;
- exemplos válidos e inválidos;
- Makefile com regras de compilação e execução.

## Tecnologias

- C++
- Flex
- Bison
- Python 3
- Make

## Dependências

Em sistemas Linux, instale:

```bash
sudo apt update
sudo apt install g++ make flex bison python3
```

## Estrutura do Projeto

```text
.
├── Makefile
├── scheme_runtime.py
├── examples/
│   ├── valid_factorial.scm
│   ├── valid_fibonacci.scm
│   ├── valid_lists.scm
│   ├── valid_closures.scm
│   ├── valid_higher_order.scm
│   ├── invalid_scope.scm
│   └── invalid_type.scm
└── src/
    ├── scanner.l
    ├── parser.y
    ├── ast.hpp / ast.cpp
    ├── symtable.hpp / symtable.cpp
    ├── analyzer.hpp / analyzer.cpp
    ├── codegen.hpp / codegen.cpp
    └── main.cpp
```

Principais arquivos:

- `src/scanner.l`: definição dos tokens da linguagem Scheme usando Flex.
- `src/parser.y`: gramática bottom-up usando Bison.
- `src/ast.*`: estruturas da árvore sintática abstrata.
- `src/symtable.*`: tabela de símbolos e controle de escopos.
- `src/analyzer.*`: verificação semântica, tipos, aridade e identificadores.
- `src/codegen.*`: geracao de código Python.
- `scheme_runtime.py`: funções auxiliares usadas pelo Python gerado.
- `examples/`: programas de teste válidos e inválidos.

## Compilação

Para compilar o projeto:

```bash
make
```

Isso gera o executável:

```text
compilador_scheme
```

Para limpar arquivos gerados:

```bash
make clean
```

## Execução

O compilador recebe um arquivo Scheme de entrada e, opcionalmente, um arquivo Python de saída:

```bash
./compilador_scheme entrada.scm saida.py
```

Exemplo:

```bash
./compilador_scheme examples/valid_factorial.scm output.py
python3 output.py
```

Se o arquivo de saída for omitido, o código Python gerado e impresso na saída padrão:
```bash
./compilador_scheme examples/valid_factorial.scm
```

Também existem alvos auxiliares no `Makefile`.

Para compilar, gerar Python e executar:

```bash
make run FILE=examples/valid_factorial.scm
```

Para apenas mostrar o Python gerado:

```bash
make show FILE=examples/valid_factorial.scm
```

## Exemplos

Exemplos válidos:

```bash
make run FILE=examples/valid_factorial.scm
make run FILE=examples/valid_fibonacci.scm
make run FILE=examples/valid_lists.scm
make run FILE=examples/valid_closures.scm
make run FILE=examples/valid_higher_order.scm
```

Exemplos inválidos:

```bash
make show FILE=examples/invalid_scope.scm
make show FILE=examples/invalid_type.scm
```

Os exemplos inválidos foram criados para exercitar mensagens de erro relacionadas a identificadores não definidos, erros de aridade e usos inválidos de tipos.

## Subconjunto de Scheme Suportado

O scanner reconhece:

- inteiros, floats e racionais;
- booleanos `#t`, `#f`, `#true`, `#false`;
- strings com escapes;
- caracteres;
- símbolos;
- listas;
- pares pontuados;
- vetores;
- abreviações como quote, quasiquote, unquote e unquote-splicing;
- comentários de linha e comentários de bloco.

O parser e o gerador tratam formas e operações como:

- `define`;
- `lambda`;
- `if`;
- `cond`;
- `case`;
- `let`, `let*`, `letrec` e named let;
- `begin`;
- `set!`;
- `and`, `or`;
- `when`, `unless`;
- `do`;
- `quote` e `quasiquote`;
- chamadas de funções;
- operações aritméticas, booleanas, listas, strings, caracteres, vetores e entrada/saída básica por meio do runtime Python.

## Analise Semântica

A etapa semântica usa uma tabela de símbolos com escopos para verificar:

- uso de variáveis antes de sua definição;
- definições locais e globais;
- parâmetros de funções e lambdas;
- atribuições com `set!`;
- aridade de funções quando conhecida;
- tipos básicos associados aos literais e expressões.

Quando erros semânticos sao encontrados, a compilação e interrompida antes da geração de código Python final.

## Geração de Código

A saída gerada e código Python 3.

O arquivo Python gerado importa `scheme_runtime.py`, que implementa funções auxiliares para aproximar a semântica de Scheme em Python, por exemplo:

- operações aritméticas n-arias;
- listas e pares;
- predicados;
- comparações;
- funções de ordem superior;
- vetores;
- entrada e saída simples.

Por isso, para executar o Python gerado, o arquivo `scheme_runtime.py` deve estar no mesmo diretorio ou acessivel pelo `PYTHONPATH`.

## Limitações Conhecidas

Este compilador implementa um subconjunto de Scheme, não a linguagem completa especificada pelas referencias R4RS/R5RS.

Limitações importantes:

- macros completas de Scheme não são implementadas;
- `define-syntax`, `let-syntax`, `letrec-syntax` e `syntax-rules` não são suportados completamente;
- continuações e recursos avançados são tratados de forma limitada pelo runtime;
- alguns comportamentos dinâmicos de Scheme são aproximados por estruturas Python;
- a verificação de tipos e estatica e conservadora, pois Scheme e uma linguagem dinamicamente tipada;
- alguns casos complexos de `letrec`, `set!` em expressões e closures podem depender das limitações naturais da traducao para Python.

## Referências

- https://www.scheme.org/
- https://www.cs.cmu.edu/Groups/AI/html/r4rs/r4rs_9.html

## Integrantes

```text
Bruno Adji
Bruno Marchiori
Mateus Maia
Thiago Arruda
```
