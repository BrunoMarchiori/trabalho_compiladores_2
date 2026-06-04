# Relatorio de Aderencia ao R4RS

Este relatorio registra a aderencia do compilador Scheme para Python em relacao ao R4RS, usando as referencias listadas no README:

- https://www.scheme.org/
- https://www.cs.cmu.edu/Groups/AI/html/r4rs/r4rs_9.html

O objetivo desta etapa foi auditar a cobertura atual, corrigir bugs-base que impediam exemplos validos e aproximar o scanner das regras lexicas formais do R4RS.

## Visao Geral

O R4RS define uma linguagem dinamicamente tipada, com escopo lexico, poucos tipos primitivos de expressao, varias expressoes derivadas e um conjunto de procedimentos padrao.

Este projeto implementa um compilador escrito em C++ que usa:

- Flex para analise lexica;
- Bison para parser bottom-up;
- AST propria;
- tabela de simbolos com escopos;
- analise semantica;
- geracao de codigo Python;
- runtime Python auxiliar para aproximar procedimentos padrao de Scheme.

## Escopo Declarado

O objetivo pratico deste trabalho e implementar o maximo possivel de um subconjunto expressivo do R4RS, deixando explicitos os casos nao cobertos. Assim, a entrega nao afirma compatibilidade total com R4RS; ela apresenta uma cobertura testada para scanner, parser, analise semantica e geracao de Python.

Os status usados neste relatorio sao:

- **Suportado**: implementado, testado e executando nos exemplos.
- **Parcial**: implementado para os casos comuns ou para um subconjunto da regra R4RS.
- **Nao coberto**: fora do escopo atual ou nao implementado.

## Matriz de Aderencia R4RS

| Area R4RS | Status | Implementacao atual | Pendencias |
| --- | --- | --- | --- |
| Espacos e comentarios de linha | Suportado | Scanner ignora espacos, quebras de linha e comentarios iniciados por `;`. | Nenhuma pendencia relevante. |
| Identificadores | Parcial | Suporta letras, caracteres especiais R4RS, `+`, `-`, `...` e normaliza simbolos para minusculas. | Validar terminacao obrigatoria por delimitador em todos os casos. |
| Case-insensitivity | Suportado parcialmente | Scanner reconhece tokens sem diferenca entre maiusculas e minusculas; simbolos sao normalizados para minusculas. | Strings preservam conteudo literal, como esperado. Casos Unicode nao sao tratados. |
| Booleanos | Suportado | Reconhece `#t`, `#f` e tambem variantes extras `#true`, `#false`. | R4RS exige apenas `#t` e `#f`; as variantes extras devem ser documentadas como extensao. |
| Caracteres | Parcial | Reconhece caracteres simples e nomes `#\space`, `#\newline`; runtime cobre conversoes e comparacoes sensiveis/insensiveis a caixa. | Rejeitar nomes de caracteres nao previstos pelo R4RS em vez de aceita-los como extensao. |
| Strings | Parcial | Reconhece strings e escapes comuns. | R4RS formaliza apenas `\"` e `\\`; escapes extras sao extensoes do projeto. |
| Numeros inteiros | Parcial | Reconhece inteiros decimais e inteiros com prefixos `#b`, `#o`, `#d`, `#x`, inclusive combinados com `#e`/`#i`. | Implementar marcadores `#` em digitos e validar delimitadores estritamente. |
| Numeros racionais | Parcial | Reconhece racionais decimais e racionais com radix; runtime cobre `numerator`, `denominator`, `exact?`, `inexact?`, `exact->inexact`, `inexact->exact`. | Validar denominador zero e casos com marcadores `#`. |
| Numeros inexatos/decimais | Parcial | Reconhece floats decimais, prefixos de exatidao e expoentes `e`, `s`, `f`, `d`, `l`. | Revisar todos os formatos formais de decimal do R4RS. |
| Numeros complexos | Nao suportado | Nao ha tipo de AST nem runtime especifico para complexos Scheme. | Adicionar tipo de AST, scanner, parser, analise e runtime. |
| Datums simples | Parcial | Booleanos, numeros, caracteres, strings e simbolos sao representados na AST. | Completar numeros complexos e validacoes lexicas. |
| Listas e pares pontuados | Suportado parcialmente | Parser reconhece listas proprias e pares pontuados. | Validar todos os casos de lista impropria segundo a gramatica formal. |
| Vetores | Parcial | Parser reconhece `#(...)` e gerador usa listas Python. | Diferenciar semanticamente vetor de lista no runtime quando necessario. |
| Quote e abreviacoes | Parcial | Suporta `'`, `` ` ``, `,` e `,@` como expansoes para formas simbolicas. | Completar regras de quasiquote com niveis de aninhamento. |
| Variaveis | Parcial | Tabela de simbolos valida identificadores definidos. | Melhorar suporte a definicoes internas e contexto de corpo R4RS. |
| Literais | Parcial | Literais basicos sao aceitos. | Completar numeros complexos e semantica de vetores. |
| Chamadas de procedimento | Parcial | Chamadas sao analisadas; built-ins sao tratados como procedimentos; aplicacao imediata de lambda e aceita pelo gerador. | Aprofundar chamadas dinamicas de procedimentos de primeira classe. |
| Lambda | Parcial | Suporta parametros fixos, variadicos, corpo com multiplas expressoes e definicoes internas simples. | Completar todos os casos formais de corpo R4RS. |
| If | Suportado parcialmente | Gera expressao/statement Python respeitando verdade Scheme via `_truthy`. | Revisar resultado nao especificado quando nao ha alternativo. |
| Set! | Parcial | Verifica se variavel existe e gera atribuicao. | Revisar interacao com closures, `nonlocal`, `global` e expressoes. |
| Cond | Parcial | Implementado no analisador e no gerador, incluindo `=>`. | Validar expansao exatamente como regra derivada R4RS. |
| Case | Parcial | Analisador reconhece e gerador emite cadeia Python com `_memv` e `else`. | Ampliar testes de datums compostos. |
| And/Or | Parcial | Implementado com curto-circuito e sem dupla avaliacao dos operandos intermediarios. | Ampliar testes com efeitos colaterais. |
| Let, let*, letrec | Parcial | Suportados no analisador e gerador. | Revisar `letrec` conforme regra de variaveis indefinidas temporarias. |
| Named let | Parcial | Reconhecido e gerado. | Validar recursao e valor de retorno em contexto de expressao. |
| Begin | Parcial | Implementado para sequencias. | Completar regra de `begin` contendo definicoes. |
| Do | Parcial | Implementado como loop Python; passos sao avaliados antes das atualizacoes, como no R4RS. | Ampliar testes com multiplas variaveis e resultado vazio. |
| Delay/force | Parcial | Runtime possui promessa simples. | Validar cache e semantica completa de promessa. |
| Quasiquote | Parcial | Implementacao com `unquote` e `unquote-splicing` para casos simples; analisador reconhece unquote dentro de quasiquote. | Implementar profundidade formal de aninhamento. |
| Definicoes | Parcial | Suporta `define` de variavel, funcao e definicoes internas simples em corpos. | Completar `(begin <definition>*)` em todos os contextos formais. |
| Procedimentos padrao | Parcial | Muitos built-ins estao mapeados no runtime e na tabela de simbolos: numeros, listas, simbolos, strings, chars, vetores, controle e I/O basico. | Fazer checklist completo do capitulo 6 do R4RS e completar mutacoes de pares/strings. |
| Macros | Nao suportado | Formas de macro geram aviso ou nao sao implementadas. | Implementar ou declarar fora de escopo. |
| Proper tail recursion | Nao garantido | Python gerado usa chamadas Python comuns. | Documentar limitacao ou implementar transformacoes especificas. |
| Continuations | Parcial/limitado | Ha nomes no runtime, mas sem equivalencia completa a `call/cc`. | Implementacao completa e complexa em Python. |

## Cobertura Demonstravel por Testes

Os testes por categoria ficam em `examples/r4rs/valid/` e `examples/r4rs/invalid/`.

| Arquivo | Categoria | Status esperado |
| --- | --- | --- |
| `examples/r4rs/valid/r4rs_lexical.scm` | Lexico, comentarios, case-insensitivity, caracteres, strings e quote | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_numbers.scm` | Numeros, radix, racionais, exatidao e procedimentos numericos basicos | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_lists.scm` | Pares, listas, quote e procedimentos de lista | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_lambda_scope.scm` | Lambda, escopo lexico, variadicos, closures, definicoes internas | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_strings_chars_vectors.scm` | Strings, chars, comparacoes e vetores | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_derived_forms.scm` | `if`, `cond`, `case`, `and`, `or`, `let`, `let*`, `letrec`, named let, `begin`, `delay`, `do` | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_quasiquote.scm` | Quote/quasiquote simples, `unquote`, `unquote-splicing` | Deve compilar e executar |
| `examples/r4rs/valid/r4rs_io_control.scm` | I/O basico, `values`, `call-with-values`, conversoes numericas | Deve compilar e executar |
| `examples/r4rs/invalid/r4rs_invalid_scope.scm` | Erros de escopo e aridade | Deve falhar na analise semantica |
| `examples/r4rs/invalid/r4rs_invalid_type.scm` | Erros estaticos de tipo em built-ins | Deve falhar na analise semantica |

## Casos Nao Cobertos ou Limitados

Estes pontos devem ser declarados explicitamente na entrega:

- macros higienicas completas;
- `syntax-rules`;
- `define-syntax`;
- `let-syntax`;
- `letrec-syntax`;
- proper tail recursion garantida;
- `call/cc` completo e continuations reais;
- numeros complexos completos;
- torre numerica completa do R4RS;
- marcadores `#` em digitos numericos;
- todos os detalhes formais de exatidao/inexatidao;
- quasiquote com profundidade formal de aninhamento;
- mutacao de pares com `set-car!` e `set-cdr!`;
- mutacao completa de strings, como `string-set!` e `string-fill!`;
- vetores diferenciados de listas no nivel de representacao interna Python;
- portas e I/O completo do R4RS;
- `load`, `eval`, ambientes formais e interacao completa;
- semantica formal completa do R4RS.

## Correcoes Realizadas Nesta Etapa

1. O parser agora preserva a ordem original dos formularios do programa. Antes, os datums de topo eram acumulados em ordem inversa, fazendo usos aparecerem antes de seus `define`.
2. Built-ins passaram a ser tratados como procedimentos na analise semantica. Antes, chamadas como `(+ 1 2)` e `(display x)` podiam ser rejeitadas porque a tabela guardava o tipo de retorno do built-in.
3. O runtime Python agora exporta explicitamente os nomes iniciados por `_`, pois `from scheme_runtime import *` nao importa esses nomes por padrao.
4. A geracao de named `let` em contexto de expressao foi corrigida para permitir recursao.
5. A geracao de `do` em posicao de retorno agora emite um bloco Python com `return`, em vez de tentar embutir `while` como expressao.
6. A geracao de `letrec` em contexto de expressao agora suporta casos de recursao mutua usando escopo local Python.
7. `let` com variaveis mutadas por `set!` dentro de lambdas agora usa celulas mutaveis, permitindo closures como contadores encapsulados.
8. O scanner foi ajustado para ficar mais proximo do lexico R4RS:
   - reconhecimento case-insensitive;
   - normalizacao de simbolos para minusculas;
   - prefixos de radix `#b`, `#o`, `#d`, `#x`;
   - prefixos de exatidao `#e`, `#i`, inclusive combinados com radix;
   - racionais com radix;
   - expoentes `e`, `s`, `f`, `d`, `l` em decimais;
   - caracteres nomeados `#\space` e `#\newline`.
9. A analise semantica agora rejeita usos estaticamente invalidos em built-ins numericos, listas, strings, chars e vetores quando os tipos sao conhecidos.
10. A geracao de aplicacao imediata de lambda foi corrigida, por exemplo `((lambda (x) ...) 10)`.
11. A geracao de `case` foi adicionada em contexto de expressao e statement.
12. `and` e `or` agora evitam dupla avaliacao dos operandos intermediarios no Python gerado.
13. A ordem de atualizacao do `do` foi corrigida: todas as expressoes de passo sao avaliadas antes de qualquer atribuicao.
14. Definicoes internas simples sao predeclaradas na analise semantica de corpos, permitindo recursao mutua local.
15. Foram adicionados testes por categoria para quasiquote e I/O/controle basico.
16. Foram adicionadas comparacoes case-insensitive de strings e caracteres (`string-ci*`, `char-ci*`) e comparacoes complementares de caracteres.
17. Foram adicionados procedimentos numericos de exatidao e racionalidade: `exact?`, `inexact?`, `numerator`, `denominator`, `exact->inexact`, `inexact->exact`.
18. O analisador agora aceita `unquote` e `unquote-splicing` dentro do contexto de `quasiquote`.

## Limitacoes Apos Esta Etapa

As fases 1 a 3 nao completam toda a implementacao R4RS. Elas deixam o projeto melhor documentado, corrigem erros de base e expandem o scanner.

Ainda faltam, para uma aderencia mais forte:

- parser separado para expressoes/definicoes conforme gramatica formal;
- validacoes semanticas ainda mais precisas de tipos e aridade em procedimentos dinamicos;
- suporte completo a numeros complexos;
- quasiquote com profundidade;
- runtime conferido procedimento por procedimento contra o capitulo 6 do R4RS;
- macros higienicas ou documentacao formal de exclusao;
- proper tail recursion e continuations completas.
