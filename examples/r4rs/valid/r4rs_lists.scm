; Testes de pares, listas, quote e procedimentos de lista.

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

(define xs '(1 2 3))

(check 'car 1 (car xs))
(check 'cdr '(2 3) (cdr xs))
(check 'cons '(0 1 2 3) (cons 0 xs))
(check 'length 3 (length xs))
(check 'append '(1 2 3 4) (append '(1 2) '(3 4)))
(check 'reverse '(3 2 1) (reverse xs))
(check 'list-ref 2 (list-ref xs 1))
(check 'list-tail '(2 3) (list-tail xs 1))
(check 'null-true #t (null? '()))
(check 'pair-true #t (pair? xs))
(check 'member '(2 3) (member 2 xs))

