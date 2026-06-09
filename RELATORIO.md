# Relatório da segunda entrega

## Visão geral

Este relatório descreve a segunda entrega do projeto de Compiladores: um compilador de um subconjunto de Scheme para Python.

O compilador foi desenvolvido em C++ e usa Flex para o scanner e Bison para o parser bottom-up. Depois da análise léxica e sintática, o projeto constrói uma AST, executa verificação semântica de tipos e de contexto de identificadores, e gera código Python 3. O Python gerado depende do arquivo `scheme_runtime.py`, que implementa funções auxiliares para aproximar operações de Scheme em Python.

A entrega inclui:

- código-fonte do compilador;
- Makefile com instruções de compilação e execução;
- arquivos de exemplo válidos e inválidos;
- relatório com limitações conhecidas;
- descrição da atuação de cada membro do grupo.

## Atuação dos membros

### Bruno Adji: scanner, parser e AST

Responsável pela análise léxica e sintática do compilador.

Implementou o scanner Flex (`scanner.l`) com suporte a prefixos numéricos do R4RS (`#b`, `#o`, `#d`, `#x`) combinados com marcadores de exatidão (`#e`, `#i`), processamento de strings com escapes, literais de caractere nomeados (`#\space`, `#\newline`), comentários de bloco (`#| |#`) e normalização de símbolos para minúsculas por meio de `%option caseless`. Também implementou funções auxiliares de conversão numérica no bloco `%{...%}` do scanner.

Implementou o parser Bison (`parser.y`) com gramática de datum, suporte a pares pontuados, vetores e abreviações de citação. Corrigiu a ordem de acumulação dos datums para preservar a ordem original dos formulários do programa.

Definiu a estrutura de nós da AST (`ast.hpp` e `ast.cpp`), os enums `NodeKind` e `SchemeType`, e funções auxiliares de manipulação de listas encadeadas, como `listLen`, `listGet`, `listToVec` e `nodeToString`.

### Bruno Marchiori: tabela de símbolos e análise semântica

Responsável pela verificação de tipos e de contexto de identificadores.

Implementou a tabela de símbolos (`symtable.hpp` e `symtable.cpp`) com pilha de escopos léxicos e registro inicial de procedimentos built-in com tipos de retorno e aridades.

Implementou o analisador semântico (`analyzer.hpp` e `analyzer.cpp`), cobrindo formas especiais como `define`, `lambda`, `if`, `cond`, `case`, `let`, `let*`, `letrec`, `letrec*`, `begin`, `set!`, `and`, `or`, `when`, `unless`, `do`, `quote`, `quasiquote`, `delay` e `force`. Também implementou a pré-declaração de definições internas (`predeclareBody`) para permitir recursão mútua local e a verificação de tipos de argumentos para built-ins numéricos, de listas, strings, caracteres e vetores.

### Mateus Maia: geração de código

Responsável pela tradução da AST para Python.

Implementou o gerador de código (`codegen.hpp` e `codegen.cpp`) com rotinas para gerar expressões e comandos Python a partir das formas Scheme suportadas. Resolveu casos de closures com `set!` usando boxing: variáveis mutadas passam a ser representadas como listas de um elemento (`[valor]`) e o acesso ocorre por `box[0]`. Implementou `and` e `or` com curto-circuito, evitando dupla avaliação dos operandos intermediários.

Também implementou a geração de `let`, `let*`, `letrec`, named let, `do`, aplicação imediata de lambda e `case`, além do mapeamento de identificadores Scheme para identificadores Python (`pyId`) e da tabela de built-ins. Implementou o driver principal do compilador (`main.cpp`).

### Thiago Arruda: runtime Python, testes e documentação

Responsável pela biblioteca de suporte ao código gerado, pelos exemplos e pela documentação.

Implementou `scheme_runtime.py`, cobrindo aritmética n-ária com suporte a `Fraction`, comparações n-árias, predicados de tipo, operações de lista, strings, caracteres, vetores, entrada/saída básica, controle de fluxo e uma implementação limitada de continuações por exceção.

Criou arquivos de exemplo organizados em `examples/basic/` e `examples/r4rs/`, separados entre casos válidos e inválidos, além do script de testes automatizados `scripts/run_tests.sh`.

Também escreveu e revisou o `README.md` com instruções de compilação e execução, e este `RELATORIO.md` com a matriz de aderência ao R4RS, limitações conhecidas e atuação dos membros.

## Referências usadas

- https://www.scheme.org/
- https://www.cs.cmu.edu/Groups/AI/html/r4rs/r4rs_9.html

## Escopo da implementação

O objetivo prático do projeto é implementar um subconjunto expressivo de Scheme, deixando explícitos os casos não cobertos. A entrega não afirma compatibilidade total com R4RS; ela apresenta uma cobertura testada para scanner, parser, análise semântica e geração de código Python.

Os status usados neste relatório são:

- **Suportado**: implementado, testado e executando nos exemplos.
- **Parcial**: implementado para os casos comuns ou para um subconjunto da regra R4RS.
- **Não coberto**: fora do escopo atual ou não implementado.

## Matriz de aderência ao R4RS

| Área R4RS | Status | Implementação atual | Pendências |
| --- | --- | --- | --- |
| Espaços e comentários de linha | Suportado | Scanner ignora espaços, quebras de linha e comentários iniciados por `;`. | Nenhuma pendência relevante. |
| Identificadores | Parcial | Suporta letras, caracteres especiais R4RS, `+`, `-`, `...` e normaliza símbolos para minúsculas. | Validar terminação obrigatória por delimitador em todos os casos. |
| Case-insensitivity | Parcial | Scanner reconhece tokens sem diferenciar maiúsculas e minúsculas; símbolos são normalizados para minúsculas. | Strings preservam o conteúdo literal, como esperado. Casos Unicode não são tratados. |
| Booleanos | Suportado | Reconhece `#t`, `#f` e as variantes extras `#true`, `#false`. | R4RS exige apenas `#t` e `#f`; as variantes extras são extensões do projeto. |
| Caracteres | Parcial | Reconhece caracteres simples e nomes `#\space`, `#\newline`; runtime cobre conversões e comparações sensíveis e insensíveis a caixa. | Rejeitar nomes de caracteres não previstos pelo R4RS, em vez de aceitá-los como extensão. |
| Strings | Parcial | Reconhece strings e escapes comuns. | R4RS formaliza apenas `\"` e `\\`; escapes extras são extensões do projeto. |
| Números inteiros | Parcial | Reconhece inteiros decimais e inteiros com prefixos `#b`, `#o`, `#d`, `#x`, inclusive combinados com `#e` e `#i`. | Implementar marcadores `#` em dígitos e validar delimitadores estritamente. |
| Números racionais | Parcial | Reconhece racionais decimais e racionais com radix; runtime cobre `numerator`, `denominator`, `exact?`, `inexact?`, `exact->inexact` e `inexact->exact`. | Validar denominador zero e casos com marcadores `#`. |
| Números inexatos/decimais | Parcial | Reconhece floats decimais, prefixos de exatidão e expoentes `e`, `s`, `f`, `d`, `l`. | Revisar todos os formatos formais de decimal do R4RS. |
| Números complexos | Não coberto | Não há tipo de AST nem runtime específico para complexos Scheme. | Adicionar scanner, parser, AST, análise semântica, geração e runtime. |
| Datums simples | Parcial | Booleanos, números, caracteres, strings e símbolos são representados na AST. | Completar números complexos e validações léxicas. |
| Listas e pares pontuados | Parcial | Parser reconhece listas próprias e pares pontuados. | Validar todos os casos de lista imprópria segundo a gramática formal. |
| Vetores | Parcial | Parser reconhece `#(...)` e gerador usa listas Python. | Diferenciar semanticamente vetor de lista quando necessário. |
| Quote e abreviações | Parcial | Suporta `'`, `` ` ``, `,` e `,@` como expansões para formas simbólicas. | Completar regras de quasiquote com níveis formais de aninhamento. |
| Variáveis | Parcial | Tabela de símbolos valida identificadores definidos em escopos globais e locais. | Melhorar suporte a definições internas em todos os contextos de corpo do R4RS. |
| Chamadas de procedimento | Parcial | Chamadas são analisadas; built-ins são tratados como procedimentos; aplicação imediata de lambda é aceita pelo gerador. | Aprofundar chamadas dinâmicas de procedimentos de primeira classe. |
| Lambda | Parcial | Suporta parâmetros fixos, variádicos, corpo com múltiplas expressões e definições internas simples. | Completar todos os casos formais de corpo R4RS. |
| If | Parcial | Gera expressão ou comando Python respeitando verdade Scheme por meio de `_truthy`. | Revisar resultado não especificado quando não há alternativa. |
| Set! | Parcial | Verifica se a variável existe e gera atribuição. | Revisar interação com closures, `nonlocal`, `global` e expressões. |
| Cond | Parcial | Implementado no analisador e no gerador, incluindo `=>`. | Validar expansão exatamente como regra derivada R4RS. |
| Case | Parcial | Analisador reconhece e gerador emite cadeia Python com `_memv` e `else`. | Ampliar testes de datums compostos. |
| And/Or | Parcial | Implementado com curto-circuito e sem dupla avaliação dos operandos intermediários. | Ampliar testes com efeitos colaterais. |
| Let, let*, letrec e letrec* | Parcial | Suportados no analisador e gerador. | Revisar `letrec` conforme regra de variáveis temporariamente indefinidas. |
| Named let | Parcial | Reconhecido e gerado. | Validar recursão e valor de retorno em contexto de expressão. |
| Begin | Parcial | Implementado para sequências. | Completar regra de `begin` contendo definições em todos os contextos formais. |
| Do | Parcial | Implementado como loop Python; passos são avaliados antes das atualizações. | Ampliar testes com múltiplas variáveis e resultado vazio. |
| Delay/force | Parcial | Runtime possui promessa simples e gerador/analisador reconhecem os casos básicos. | Validar cache e semântica completa de promessa. |
| Quasiquote | Parcial | Implementação com `unquote` e `unquote-splicing` para casos simples. | Implementar profundidade formal de aninhamento. |
| Definições | Parcial | Suporta `define` de variável, função e definições internas simples em corpos. | Completar `(begin <definition>*)` em todos os contextos formais. |
| Procedimentos padrão | Parcial | Vários built-ins estão mapeados no runtime e na tabela de símbolos: números, listas, símbolos, strings, caracteres, vetores, controle e I/O básico. | Fazer checklist completo do capítulo 6 do R4RS e completar mutações de pares/strings. |
| Macros | Não coberto | Formas de macro geram aviso ou não são implementadas. | Implementar macros ou declarar formalmente fora de escopo. |
| Proper tail recursion | Não coberto | Python gerado usa chamadas Python comuns. | Implementar transformação específica ou documentar a limitação. |
| Continuations | Parcial | Há nomes no runtime, mas sem equivalência completa a `call/cc`. | Implementação completa é complexa em Python e está fora do escopo atual. |

## Cobertura demonstrável por testes

Os testes por categoria ficam em `examples/r4rs/valid/` e `examples/r4rs/invalid/`.

| Arquivo | Categoria | Status esperado |
| --- | --- | --- |
| `examples/r4rs/valid/r4rs_lexical.scm` | Léxico, comentários, case-insensitivity, caracteres, strings e quote | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_numbers.scm` | Números, radix, racionais, exatidão e procedimentos numéricos básicos | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_lists.scm` | Pares, listas, quote e procedimentos de lista | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_lambda_scope.scm` | Lambda, escopo léxico, variádicos, closures e definições internas | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_strings_chars_vectors.scm` | Strings, caracteres, comparações e vetores | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_derived_forms.scm` | `if`, `cond`, `case`, `and`, `or`, `let`, `let*`, `letrec`, named let, `begin`, `delay`, `force` e `do` | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_quasiquote.scm` | Quote/quasiquote simples, `unquote` e `unquote-splicing` | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_io_control.scm` | I/O básico, `values`, `call-with-values` e conversões numéricas | Deve compilar e executar |
| `examples/r4rs/invalid/r4rs_invalid_scope.scm` | Erros de escopo e aridade | Deve falhar na análise semântica |
| `examples/r4rs/invalid/r4rs_invalid_type.scm` | Erros estáticos de tipo em built-ins | Deve falhar na análise semântica |

## Limitações conhecidas

- macros higiênicas completas;
- `syntax-rules`;
- `define-syntax`;
- `let-syntax`;
- `letrec-syntax`;
- recursão de cauda adequada garantida;
- `call/cc` completo e continuações reais;
- números complexos completos;
- torre numérica completa do R4RS;
- marcadores `#` em dígitos numéricos;
- todos os detalhes formais de exatidão/inexatidão;
- quasiquote com profundidade formal de aninhamento;
- mutação de pares com `set-car!` e `set-cdr!`;
- mutação completa de strings, como `string-set!` e `string-fill!`;
- vetores diferenciados de listas no nível de representação interna Python;
- portas e I/O completo do R4RS;
- `load`, `eval`, ambientes formais e interação completa;
- semântica formal completa do R4RS.

## Conclusão

O projeto cumpre a proposta da segunda entrega ao implementar, com Flex e Bison, um scanner e um parser bottom-up para um subconjunto de Scheme, além de análise semântica com verificação de tipos e contexto de identificadores e geração de código Python.

O compilador não busca compatibilidade total com R4RS. As limitações conhecidas foram documentadas para deixar claro o escopo da entrega e os pontos que exigiriam evolução futura.