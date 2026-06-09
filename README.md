# Compilador Scheme para Python

Projeto da segunda entrega da disciplina de Compiladores.

O projeto implementa um compilador para um subconjunto da linguagem Scheme. O programa de entrada, escrito em um arquivo `.scm`, passa por análise léxica, análise sintática bottom-up, construção de AST, verificação semântica de tipos e de contexto de identificadores, e geração de código Python.

## Objetivo da entrega

A entrega atende ao enunciado por implementar:

- scanner com Flex;
- parser bottom-up com Bison;
- verificação de tipos básicos;
- verificação de contexto de identificadores com tabela de símbolos e escopos;
- geração de código Python a partir de programas Scheme;
- arquivos de exemplo válidos e inválidos;
- Makefile com regras de compilação e execução;
- relatório com limitações conhecidas e atuação dos integrantes.

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

## Estrutura do projeto

```text
.
├── Makefile
├── README.md
├── RELATORIO.md
├── scheme_runtime.py
├── examples/
│   ├── basic/
│   │   ├── valid/
│   │   └── invalid/
│   └── r4rs/
│       ├── valid/
│       └── invalid/
├── scripts/
│   └── run_tests.sh
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
- `src/analyzer.*`: verificação semântica, incluindo tipos, aridade e identificadores.
- `src/codegen.*`: geração de código Python.
- `src/main.cpp`: driver principal do compilador.
- `scheme_runtime.py`: funções auxiliares usadas pelo Python gerado.
- `examples/`: programas de teste organizados por categoria.
- `scripts/run_tests.sh`: script de testes automatizados.
- `RELATORIO.md`: relatório da entrega, com limitações, aderência ao R4RS e atuação dos membros.

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
./compilador_scheme examples/basic/valid/valid_factorial.scm output.py
python3 output.py
```

Se o arquivo de saída for omitido, o código Python gerado é exibido no terminal:

```bash
./compilador_scheme examples/basic/valid/valid_factorial.scm
```

Também existem alvos auxiliares no `Makefile`.

Para compilar, gerar Python e executar:

```bash
make run FILE=examples/basic/valid/valid_factorial.scm
```

Para apenas mostrar o Python gerado:

```bash
make show FILE=examples/basic/valid/valid_factorial.scm
```

## Exemplos e testes

Exemplos válidos:

```bash
make run FILE=examples/basic/valid/valid_factorial.scm
make run FILE=examples/basic/valid/valid_fibonacci.scm
make run FILE=examples/basic/valid/valid_lists.scm
make run FILE=examples/basic/valid/valid_closures.scm
make run FILE=examples/basic/valid/valid_higher_order.scm
```

Exemplos inválidos:

```bash
make show FILE=examples/basic/invalid/invalid_scope.scm
make show FILE=examples/basic/invalid/invalid_type.scm
```

Os exemplos inválidos foram criados para exercitar mensagens de erro relacionadas a identificadores não definidos, erros de aridade e usos inválidos de tipos.

Para rodar todos os exemplos e garantir se eles estão rodando da forma correta:

```bash
make
./scripts/run_tests.sh
```

Os testes inspirados no R4RS ficam em:

```text
examples/r4rs/valid/
examples/r4rs/invalid/
```

## Subconjunto de Scheme suportado

O scanner reconhece:

- inteiros, floats e racionais;
- prefixos numéricos como `#b`, `#o`, `#d` e `#x`;
- marcadores de exatidão como `#e` e `#i`;
- booleanos `#t`, `#f`, `#true` e `#false`;
- strings com escapes;
- caracteres;
- símbolos;
- listas;
- pares pontuados;
- vetores;
- abreviações como quote, quasiquote, unquote e unquote-splicing;
- comentários de linha e comentários de bloco.

O parser, o analisador semântico e o gerador tratam formas e operações como:

- `define`;
- `lambda`;
- `if`;
- `cond`;
- `case`;
- `let`, `let*`, `letrec`, `letrec*` e `named let`;
- `begin`;
- `set!`;
- `and` e `or`;
- `when` e `unless`;
- `do`;
- `delay` e `force`;
- `quote` e `quasiquote`;
- chamadas de funções;
- operações aritméticas, booleanas, de listas, strings, caracteres, vetores e entrada/saída básica por meio do runtime Python.

## Análise semântica

A etapa semântica usa uma tabela de símbolos com escopos para verificar:

- uso de variáveis antes de sua definição;
- definições locais e globais;
- parâmetros de funções e lambdas;
- atribuições com `set!`;
- aridade de funções quando conhecida;
- tipos básicos associados a literais e expressões;
- usos estaticamente inválidos de alguns built-ins numéricos, de listas, strings, caracteres e vetores.

Quando erros semânticos são encontrados, a compilação é interrompida antes da geração do código Python final.

## Geração de código

A saída gerada é código Python 3.

O arquivo Python gerado importa `scheme_runtime.py`, que implementa funções auxiliares para aproximar a semântica de Scheme em Python, por exemplo:

- operações aritméticas n-árias;
- listas e pares;
- predicados;
- comparações;
- funções de ordem superior;
- vetores;
- entrada e saída simples.

Para executar o Python gerado, o arquivo `scheme_runtime.py` deve estar no mesmo diretório do programa Python gerado ou acessível pelo `PYTHONPATH`.

## Limitações conhecidas

Este compilador implementa um subconjunto de Scheme, não a linguagem completa especificada pelas referências R4RS/R5RS.

Limitações importantes:

- macros completas de Scheme não são implementadas;
- `define-syntax`, `let-syntax`, `letrec-syntax` e `syntax-rules` não são suportados completamente;
- recursão de cauda adequada não é garantida, pois o código final usa chamadas Python comuns;
- continuações e `call/cc` são tratados de forma limitada;
- números complexos não são suportados;
- alguns comportamentos dinâmicos de Scheme são aproximados por estruturas Python;
- a verificação de tipos é estática e conservadora, pois Scheme é uma linguagem dinamicamente tipada;
- alguns casos complexos de `letrec`, `set!`, closures e quasiquote podem depender das limitações naturais da tradução para Python.

Consulte `RELATORIO.md` para a matriz de aderência ao R4RS, as limitações detalhadas e a atuação de cada integrante.

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