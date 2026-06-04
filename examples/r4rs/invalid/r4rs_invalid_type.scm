; Testes invalidos de tipo.
; Este arquivo e teste-alvo para a proxima fase de analise semantica.
; O compilador deve rejeitar estes usos quando a verificacao de tipos
; for reforcada.

(define text "hello")
(define number 42)
(define xs '(1 2 3))
(define v (vector 1 2 3))

(display (+ text 1))
(display (car number))
(display (string-ref number 0))
(display (vector-ref text 0))
(display (length number))
