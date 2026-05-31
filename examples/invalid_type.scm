; Exemplo com erros de tipo detectáveis estaticamente

; Erro: + aplicado a string (detectável em análise de tipo)
(define x "hello")
(display (+ x 1))

; Erro: car em não-par
(define y 42)
(display (car y))
