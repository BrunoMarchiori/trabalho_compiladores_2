; Testes de formas derivadas R4RS: if, cond, and, or, let, let*,
; letrec, named let, begin, delay/force e do.

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

(define (sum-do n)
  (do ((i 0 (+ i 1))
       (acc 0 (+ acc i)))
      ((> i n) acc)))

(check 'if "then" (if #t "then" "else"))

(check 'cond
       "small"
       (cond
         ((< 10 0) "negative")
         ((< 10 20) "small")
         (else "large")))

(check 'case
       "vowel"
       (case 'e
         ((a e i o u) "vowel")
         ((w y) "sometimes")
         (else "consonant")))

(check 'and 3 (and #t 1 2 3))
(check 'or "first" (or #f "first" "second"))

(check 'let 5
       (let ((x 2)
             (y 3))
         (+ x y)))

(check 'let-star 8
       (let* ((x 2)
              (y (+ x 3))
              (z (+ y 3)))
         z))

(check 'named-let 15
       (let loop ((i 5)
                  (acc 0))
         (if (= i 0)
             acc
             (loop (- i 1) (+ acc i)))))

(check 'letrec
       "even"
       (letrec ((even? (lambda (n)
                         (if (= n 0) #t (odd? (- n 1)))))
                (odd?  (lambda (n)
                         (if (= n 0) #f (even? (- n 1))))))
         (if (even? 10) "even" "odd")))

(check 'begin 99
       (begin
         (+ 1 2)
         99))

(check 'delay-force 42 (force (delay (+ 20 22))))
(check 'do 15 (sum-do 5))
