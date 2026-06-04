; Testes de lambda, escopo lexico, variadicos e closure com set!.

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

(define (make-adder n)
  (lambda (x) (+ x n)))

(define add7 (make-adder 7))

(define (shadow x)
  ((lambda (x) (+ x 1)) 10))

(define count-args
  (lambda args (length args)))

(define (make-counter)
  (let ((count 0))
    (lambda ()
      (set! count (+ count 1))
      count)))

(define (internal-parity n)
  (define (local-even? n)
    (if (= n 0) #t (local-odd? (- n 1))))
  (define (local-odd? n)
    (if (= n 0) #f (local-even? (- n 1))))
  (if (local-even? n) "even" "odd"))

(define counter (make-counter))

(check 'closure 12 (add7 5))
(check 'shadowing 11 (shadow 99))
(check 'variadic 4 (count-args 1 2 3 4))
(check 'counter-1 1 (counter))
(check 'counter-2 2 (counter))
(check 'internal-defs "odd" (internal-parity 7))
