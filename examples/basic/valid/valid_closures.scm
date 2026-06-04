; Closures com set! (contador encapsulado)
(define (make-counter)
  (let ((count 0))
    (lambda ()
      (set! count (+ count 1))
      count)))

(define c1 (make-counter))
(define c2 (make-counter))

(display (c1)) (newline)   ; 1
(display (c1)) (newline)   ; 2
(display (c2)) (newline)   ; 1  (independente)
(display (c1)) (newline)   ; 3

; Adder factory
(define (make-adder n)
  (lambda (x) (+ x n)))

(define add5  (make-adder 5))
(define add10 (make-adder 10))

(display (add5 3))  (newline)  ; 8
(display (add10 3)) (newline)  ; 13
