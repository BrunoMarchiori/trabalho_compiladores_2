; Testes invalidos de escopo e aridade.
; Este arquivo deve falhar na analise semantica.

(display missing-name)

(set! another-missing-name 10)

(define (two-args x y)
  (+ x y))

(display (two-args 1))
(display (two-args 1 2 3))

