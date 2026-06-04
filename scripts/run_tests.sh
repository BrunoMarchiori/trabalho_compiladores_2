#!/usr/bin/env bash
set -u

COMPILER="./compilador_scheme"
PYTHON_BIN="${PYTHON:-python3}"

valid_cases=(
  "examples/basic/valid/valid_factorial.scm"
  "examples/basic/valid/valid_fibonacci.scm"
  "examples/basic/valid/valid_lists.scm"
  "examples/basic/valid/valid_closures.scm"
  "examples/basic/valid/valid_higher_order.scm"
  "examples/r4rs/valid/r4rs_lexical.scm"
  "examples/r4rs/valid/r4rs_numbers.scm"
  "examples/r4rs/valid/r4rs_lists.scm"
  "examples/r4rs/valid/r4rs_lambda_scope.scm"
  "examples/r4rs/valid/r4rs_strings_chars_vectors.scm"
  "examples/r4rs/valid/r4rs_derived_forms.scm"
  "examples/r4rs/valid/r4rs_quasiquote.scm"
  "examples/r4rs/valid/r4rs_io_control.scm"
)

invalid_cases=(
  "examples/basic/invalid/invalid_scope.scm"
  "examples/basic/invalid/invalid_type.scm"
  "examples/r4rs/invalid/r4rs_invalid_scope.scm"
  "examples/r4rs/invalid/r4rs_invalid_type.scm"
)

failures=0

echo "== Testes validos =="
for file in "${valid_cases[@]}"; do
  out="/tmp/$(basename "$file" .scm).py"
  if "$COMPILER" "$file" "$out" >/tmp/compiler_test.out 2>/tmp/compiler_test.err; then
    if PYTHONPATH=. "$PYTHON_BIN" "$out" >/tmp/python_test.out 2>/tmp/python_test.err; then
      echo "PASS $file"
    else
      echo "FAIL $file"
      cat /tmp/python_test.err
      failures=$((failures + 1))
    fi
  else
    echo "FAIL $file"
    cat /tmp/compiler_test.err
    failures=$((failures + 1))
  fi
done

echo "== Testes invalidos =="
for file in "${invalid_cases[@]}"; do
  out="/tmp/$(basename "$file" .scm).py"
  if "$COMPILER" "$file" "$out" >/tmp/compiler_test.out 2>/tmp/compiler_test.err; then
    echo "FAIL $file deveria falhar"
    failures=$((failures + 1))
  else
    echo "PASS $file falhou como esperado"
  fi
done

if [ "$failures" -ne 0 ]; then
  echo "Falhas: $failures"
  exit 1
fi

echo "Todos os testes passaram."
