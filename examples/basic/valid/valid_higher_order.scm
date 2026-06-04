; Funções de ordem superior e formas especiais diversas

; compose
(define (compose f g)
  (lambda (x) (f (g x))))

(define inc    (lambda (x) (+ x 1)))
(define double (lambda (x) (* x 2)))
(define inc-then-double (compose double inc))

(display (inc-then-double 3)) (newline)   ; 8 = (3+1)*2

; curry
(define (curry f)
  (lambda (x) (lambda (y) (f x y))))

(define add (curry +))
(display ((add 3) 4)) (newline)   ; 7

; cond com =>
(define (classify n)
  (cond
    ((< n 0)  "negativo")
    ((= n 0)  "zero")
    ((< n 10) "pequeno")
    ((< n 100) "médio")
    (else      "grande")))

(display (classify -5))  (newline)
(display (classify 0))   (newline)
(display (classify 7))   (newline)
(display (classify 42))  (newline)
(display (classify 999)) (newline)

; do loop: soma de 1 a n
(define (sum-to n)
  (do ((i 1 (+ i 1))
       (acc 0 (+ acc i)))
      ((> i n) acc)))

(display (sum-to 10)) (newline)   ; 55

; let* encadeado
(define resultado
  (let* ((x 2)
         (y (* x 3))
         (z (+ x y)))
    (* z z)))
(display resultado) (newline)   ; 64

; letrec para funções mutuamente recursivas
(define (par-ou-impar n)
  (letrec ((my-even? (lambda (n)
                        (if (= n 0) #t (my-odd? (- n 1)))))
           (my-odd?  (lambda (n)
                        (if (= n 0) #f (my-even? (- n 1))))))
    (if (my-even? n) "par" "ímpar")))

(display (par-ou-impar 4)) (newline)
(display (par-ou-impar 7)) (newline)
