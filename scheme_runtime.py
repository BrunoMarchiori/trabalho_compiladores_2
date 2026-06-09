"""
scheme_runtime.py
Runtime Python para programas compilados a partir de Scheme.
Todas as funções têm prefixo _ para evitar colisão com nomes do usuário.
"""

from fractions import Fraction
import math
import sys

# ── Semântica booleana de Scheme ─────────────────────────────
# Em Scheme, somente #f é falso; tudo mais é verdadeiro.

def _truthy(x):
    return x is not False

def _not(x):
    return x is False

# ── Aritmética n-ária ────────────────────────────────────────

def _add(*args):
    if not args: return 0
    r = args[0]
    for a in args[1:]: r += a
    return r

def _sub(*args):
    if not args: return 0
    if len(args) == 1: return -args[0]
    r = args[0]
    for a in args[1:]: r -= a
    return r

def _mul(*args):
    if not args: return 1
    r = args[0]
    for a in args[1:]: r *= a
    return r

def _div(*args):
    if not args: return 1
    if len(args) == 1:
        if isinstance(args[0], int): return Fraction(1, args[0])
        return 1.0 / args[0]
    r = args[0]
    for a in args[1:]:
        if isinstance(r, int) and isinstance(a, int) and r % a == 0:
            r = r // a
        elif isinstance(r, int) and isinstance(a, int):
            r = Fraction(r, a)
        else:
            r = r / a
    return r

def _modulo(a, b):
    r = a % b
    if r != 0 and (r < 0) != (b < 0): r += b
    return r

def _remainder(a, b):
    r = a % b
    if r != 0 and (r < 0) != (a < 0): r -= b
    return r

def _quotient(a, b):
    return int(a / b)

def _expt(a, b):
    if isinstance(b, int) and b >= 0 and isinstance(a, int):
        return a ** b
    return a ** b

def _abs(x):    return abs(x)
def _max(*args): return max(args)
def _min(*args): return min(args)

def _floor(x):    return math.floor(x)
def _ceiling(x):  return math.ceil(x)
def _trunc(x):    return math.trunc(x)
def _round(x):    return round(x)
def _sqrt(x):
    r = math.sqrt(x)
    return int(r) if r == int(r) else r

def _gcd(*args):
    if not args: return 0
    r = args[0]
    for a in args[1:]: r = math.gcd(r, a)
    return r

def _lcm(*args):
    if not args: return 1
    r = args[0]
    for a in args[1:]: r = r * a // math.gcd(r, a)
    return r

# ── Comparação numérica n-ária ───────────────────────────────

def _num_eq(*args):
    return all(args[i] == args[i+1] for i in range(len(args)-1))

def _lt(*args):
    return all(args[i] < args[i+1] for i in range(len(args)-1))

def _le(*args):
    return all(args[i] <= args[i+1] for i in range(len(args)-1))

def _gt(*args):
    return all(args[i] > args[i+1] for i in range(len(args)-1))

def _ge(*args):
    return all(args[i] >= args[i+1] for i in range(len(args)-1))

# ── Predicados numéricos ─────────────────────────────────────

def _is_zero(x):      return x == 0
def _is_positive(x):  return x > 0
def _is_negative(x):  return x < 0
def _is_odd(x):       return int(x) % 2 != 0
def _is_even(x):      return int(x) % 2 == 0
def _is_number(x):    return isinstance(x, (int, float, complex, Fraction))
def _is_integer(x):   return isinstance(x, int) or (isinstance(x, float) and x == int(x))
def _is_real(x):      return isinstance(x, (int, float, Fraction))
def _is_rational(x):  return isinstance(x, (int, Fraction))
def _is_complex(x):   return isinstance(x, (int, float, complex, Fraction))
def _is_exact(x):     return isinstance(x, (int, Fraction)) and not isinstance(x, bool)
def _is_inexact(x):   return isinstance(x, (float, complex))
def _numerator(x):
    if isinstance(x, Fraction): return x.numerator
    if isinstance(x, int): return x
    return Fraction(x).limit_denominator().numerator
def _denominator(x):
    if isinstance(x, Fraction): return x.denominator
    if isinstance(x, int): return 1
    return Fraction(x).limit_denominator().denominator
def _exact_to_inexact(x): return float(x)
def _inexact_to_exact(x): return Fraction(x).limit_denominator()

# ── Igualdade ────────────────────────────────────────────────

def _eq(a, b):    return a is b or a == b
def _eqv(a, b):   return a == b
def _equal(a, b): return a == b

# ── Listas ───────────────────────────────────────────────────

def _cons(a, b):
    if isinstance(b, list): return [a] + b
    if b is None or b == []: return [a]
    return (a, b)   # par dotado como tupla

def _car(p):
    if isinstance(p, (list, tuple)) and len(p) > 0: return p[0]
    raise TypeError(f"car: esperava par, recebeu {type(p).__name__}")

def _cdr(p):
    if isinstance(p, list): return p[1:]
    if isinstance(p, tuple) and len(p) == 2: return p[1]
    raise TypeError(f"cdr: esperava par, recebeu {type(p).__name__}")

def _list(*args):   return list(args)
def _is_null(x):   return x == [] or x is None
def _is_pair(x):   return isinstance(x, (list, tuple)) and len(x) > 0
def _is_list(x):   return isinstance(x, list)
def _length(lst):  return len(lst)

def _append(*lists):
    result = []
    for lst in lists:
        if isinstance(lst, list): result.extend(lst)
        elif lst is not None:     result.append(lst)
    return result

def _reverse(lst):  return list(reversed(lst))
def _list_ref(lst, i):  return lst[i]
def _list_tail(lst, i): return lst[i:]
def _list_copy(lst):    return list(lst)

def _member(x, lst):
    for i, item in enumerate(lst):
        if item == x: return lst[i:]
    return False

def _memq(x, lst):
    for i, item in enumerate(lst):
        if item is x or item == x: return lst[i:]
    return False

_memv = _member

def _assoc(key, alist):
    for pair in alist:
        if isinstance(pair, (list, tuple)) and pair and pair[0] == key: return list(pair)
    return False

_assq = _assv = _assoc

# acessores compostos (caaar..cdddr)
def _caar(p):   return _car(_car(p))
def _cadr(p):   return _car(_cdr(p))
def _cdar(p):   return _cdr(_car(p))
def _cddr(p):   return _cdr(_cdr(p))
def _caaar(p):  return _car(_caar(p))
def _caadr(p):  return _car(_cadr(p))
def _caddr(p):  return _car(_cddr(p))
def _cadddr(p): return _car(_cdr(_cddr(p)))

# ── Map / for-each / apply ────────────────────────────────────

def _map(f, *lists):
    return list(map(f, *lists))

def _for_each(f, *lists):
    for args in zip(*lists):
        f(*args)

def _apply(f, *args):
    if not args: return f()
    last = args[-1]
    head = list(args[:-1])
    if isinstance(last, list): return f(*head, *last)
    return f(*head, last)

def _filter(pred, lst):
    return [x for x in lst if _truthy(pred(x))]

def _fold_left(f, init, lst):
    acc = init
    for x in lst: acc = f(acc, x)
    return acc

def _fold_right(f, init, lst):
    acc = init
    for x in reversed(lst): acc = f(x, acc)
    return acc

def _reduce(f, init, lst):
    return _fold_left(f, init, lst)

# ── Strings ───────────────────────────────────────────────────

def _is_string(x):    return isinstance(x, str)
def _string_length(s): return len(s)
def _string_ref(s, i): return s[i]
def _string_append(*args): return ''.join(args)
def _substring(s, start, end=None): return s[start:end]
def _string_eq(a, b): return a == b
def _string_lt(a, b): return a < b
def _string_gt(a, b): return a > b
def _string_le(a, b): return a <= b
def _string_ge(a, b): return a >= b
def _string_ci_eq(a, b): return a.lower() == b.lower()
def _string_ci_lt(a, b): return a.lower() < b.lower()
def _string_ci_gt(a, b): return a.lower() > b.lower()
def _string_ci_le(a, b): return a.lower() <= b.lower()
def _string_ci_ge(a, b): return a.lower() >= b.lower()
def _string_upcase(s): return s.upper()
def _string_downcase(s): return s.lower()
def _string_copy(s): return str(s)

def _number_to_string(n, radix=10):
    if radix == 16: return hex(n)[2:]
    if radix == 8:  return oct(n)[2:]
    if radix == 2:  return bin(n)[2:]
    return str(n)

def _string_to_number(s, radix=10):
    try:
        if '.' in s: return float(s)
        return int(s, radix)
    except Exception:
        return False

def _string_to_symbol(s): return s
def _symbol_to_string(s): return s
def _string_to_list(s): return list(s)
def _list_to_string(lst): return ''.join(lst)
def _mk_string(*args): return ''.join(args)
def _make_string(n, c=' '): return c * n
def _string_contains(s, sub): return sub in s
def _string_split(s, sep): return list(s.split(sep))

# ── Caracteres ────────────────────────────────────────────────

def _is_char(x):  return isinstance(x, str) and len(x) == 1
def _char_to_int(c): return ord(c)
def _int_to_char(n): return chr(n)
def _char_eq(a, b): return a == b
def _char_lt(a, b): return a < b
def _char_gt(a, b): return a > b
def _char_le(a, b): return a <= b
def _char_ge(a, b): return a >= b
def _char_ci_eq(a, b): return a.lower() == b.lower()
def _char_ci_lt(a, b): return a.lower() < b.lower()
def _char_ci_gt(a, b): return a.lower() > b.lower()
def _char_ci_le(a, b): return a.lower() <= b.lower()
def _char_ci_ge(a, b): return a.lower() >= b.lower()
def _char_alpha(c): return c.isalpha()
def _char_num(c):   return c.isdigit()
def _char_ws(c):    return c.isspace()
def _char_up(c):    return c.upper()
def _char_dn(c):    return c.lower()

# ── Predicados de tipo ────────────────────────────────────────

def _is_symbol(x):    return isinstance(x, str)
def _is_boolean(x):   return isinstance(x, bool)
def _is_procedure(x): return callable(x)
def _is_vector(x):    return isinstance(x, list)
def _is_port(x):      return hasattr(x, 'read') or hasattr(x, 'write')
def _is_input_port(x):  return hasattr(x, 'read')
def _is_output_port(x): return hasattr(x, 'write')
def _is_eof(x):       return x is EOF

# ── Vetores ───────────────────────────────────────────────────

def _make_vector(n, fill=False): return [fill] * n
def _vector(*args):  return list(args)
def _vector_ref(v, i): return v[i]
def _vector_set(v, i, val): v[i] = val
def _vector_length(v): return len(v)
def _vector_to_list(v): return list(v)
def _list_to_vector(lst): return list(lst)
def _vector_fill(v, val):
    for i in range(len(v)): v[i] = val

# ── I/O ───────────────────────────────────────────────────────

class _EOF:
    def __repr__(self): return "#<eof>"

EOF = _EOF()

def _display(x, port=None):
    out = port or sys.stdout
    if isinstance(x, bool):
        out.write('#t' if x else '#f')
    elif isinstance(x, str):
        out.write(x)
    elif isinstance(x, list):
        out.write(_scheme_repr(x))
    else:
        out.write(str(x))

def _newline(port=None):
    out = port or sys.stdout
    out.write('\n')

def _write(x, port=None):
    out = port or sys.stdout
    out.write(_scheme_repr(x))

def _scheme_repr(x):
    if isinstance(x, bool):   return '#t' if x else '#f'
    if isinstance(x, str):    return '"' + x.replace('\\','\\\\').replace('"','\\"') + '"'
    if isinstance(x, list):
        if not x: return '()'
        inner = ' '.join(_scheme_repr(e) for e in x)
        return '(' + inner + ')'
    if isinstance(x, tuple) and len(x) == 2:
        return '(' + _scheme_repr(x[0]) + ' . ' + _scheme_repr(x[1]) + ')'
    return str(x)

def _read(port=None):
    try:
        line = (port or sys.stdin).readline()
        if not line: return EOF
        return line.rstrip('\n')
    except EOFError:
        return EOF

def _read_char(port=None):
    ch = (port or sys.stdin).read(1)
    return ch if ch else EOF

def _write_char(c, port=None):
    (port or sys.stdout).write(c)

def _open_in(path):  return open(path, 'r')
def _open_out(path): return open(path, 'w')
def _close_in(p):    p.close()
def _close_out(p):   p.close()
def _cur_in():       return sys.stdin
def _cur_out():      return sys.stdout

# ── Controle / continuações (limitado) ───────────────────────

def _error(msg, *args):
    raise Exception(f"Scheme error: {msg} {' '.join(str(a) for a in args)}")

def _void(): return None

def _values(*args):
    return args if len(args) != 1 else args[0]

def _call_with_values(producer, consumer):
    result = producer()
    if isinstance(result, tuple):
        return consumer(*result)
    return consumer(result)

def _call_cc(f):
    # continuações completas não suportadas; simula com exceção
    class Continuation(Exception):
        def __init__(self, val): self.val = val
    def k(val): raise Continuation(val)
    try:
        return f(k)
    except Continuation as c:
        return c.val

def _dynamic_wind(before, thunk, after):
    before()
    try:
        result = thunk()
    finally:
        after()
    return result

def _force(promise):
    if callable(promise): return promise()
    return promise

def _delay(thunk):
    called = [False]
    cache  = [None]
    def force():
        if not called[0]:
            cache[0]  = thunk()
            called[0] = True
        return cache[0]
    return force

def _raise(obj):
    raise Exception(str(obj))

def _with_exc_handler(handler, thunk):
    try:
        return thunk()
    except Exception as e:
        return handler(str(e))

# ── Quasiquote helper ─────────────────────────────────────────

def _cons_list(items):
    """Constrói lista respeitando unquote-splicing (marcado como ('splice', lst))."""
    result = []
    for item in items:
        if isinstance(item, tuple) and len(item) == 2 and item[0] == 'splice':
            result.extend(item[1])
        else:
            result.append(item)
    return result

__all__ = [
    name for name in globals()
    if name.startswith("_") and not name.startswith("__")
]
