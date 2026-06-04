; Testes lexicos R4RS: comentarios, case-insensitivity, caracteres,
; strings, simbolos e abreviacao de quote.

#|
  Comentario de bloco: deve ser ignorado pelo scanner.
|#

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

(define Mixed-Value 42)

(check 'case-fold 42 mixed-value)
(check 'char-space 32 (char->integer #\space))
(check 'char-newline 10 (char->integer #\newline))
(check 'quote-fold "hello" 'HELLO)
(check 'string-quote "a\"b" "a\"b")
(check 'string-backslash "a\\b" "a\\b")

