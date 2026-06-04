; Testes de quote/quasiquote R4RS em casos simples.
; A profundidade formal completa de quasiquote aninhado permanece documentada
; como cobertura parcial.

(define (check label expected actual)
  (if (equal? expected actual)
      (begin
        (display "ok ")
        (display label)
        (newline))
      (begin
        (display "FAIL ")
        (display label)
        (display " expected=")
        (write expected)
        (display " actual=")
        (write actual)
        (newline)
        (error "check failed"))))

(define x 10)
(define ys '(2 3))

(check 'quote-list '(1 2 3) '(1 2 3))
(check 'quote-symbol "alpha" 'alpha)
(check 'quasiquote-unquote '(1 10 3) `(1 ,x 3))
(check 'quasiquote-splicing '(1 2 3 4) `(1 ,@ys 4))

