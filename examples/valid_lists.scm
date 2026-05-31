; Operações com listas
(define lst (list 1 2 3 4 5))

(display (length lst))          (newline)  ; 5
(display (reverse lst))         (newline)  ; (5 4 3 2 1)
(display (map (lambda (x) (* x x)) lst)) (newline)  ; (1 4 9 16 25)
(display (filter odd? lst))     (newline)  ; (1 3 5)

; Acumulação com fold
(define (sum lst)
  (if (null? lst)
      0
      (+ (car lst) (sum (cdr lst)))))

(display (sum lst)) (newline)  ; 15

; Flatten usando append
(define (flatten lst)
  (cond
    ((null? lst) '())
    ((pair? (car lst)) (append (flatten (car lst)) (flatten (cdr lst))))
    (else (cons (car lst) (flatten (cdr lst))))))

(display (flatten '(1 (2 3) (4 (5 6))))) (newline)  ; (1 2 3 4 5 6)

; association list
(define env '((x . 10) (y . 20) (z . 30)))
(display (assoc 'y env)) (newline)  ; (y . 20)
