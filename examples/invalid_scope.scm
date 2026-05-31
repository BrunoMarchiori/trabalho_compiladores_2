; Exemplo com erros de escopo

; Erro: variável não definida
(display undefined-variable)

; Erro: set! em variável não definida
(set! also-undefined 42)

; Erro: aridade errada
(define (f x y) (+ x y))
(display (f 1))          ; falta um argumento
(display (f 1 2 3))      ; argumento a mais
