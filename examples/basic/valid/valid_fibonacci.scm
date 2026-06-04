; Fibonacci com acumulador (tail-recursive via named let)
(define (fib n)
  (let loop ((i n) (a 0) (b 1))
    (if (= i 0)
        a
        (loop (- i 1) b (+ a b)))))

(display (fib 0))  (newline)
(display (fib 1))  (newline)
(display (fib 10)) (newline)
(display (fib 20)) (newline)
