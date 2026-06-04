; Testes de I/O basico e controle suportados pelo runtime Python.

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

(display "io-smoke")
(newline)
(write '(1 2 3))
(newline)

(check 'procedure-predicate #t (procedure? check))
(check 'values-single 7 (values 7))
(check 'call-with-values 9
       (call-with-values
         (lambda () (values 4 5))
         (lambda (a b) (+ a b))))
(check 'number->string "ff" (number->string 255 16))
(check 'string->number 255 (string->number "ff" 16))

