from fractions import Fraction
from scheme_runtime import *

def make_counter():
    return (lambda _box_count: lambda : [_box_count.__setitem__(0, _add(_box_count[0], 1)), _box_count[0]][-1])([0])
c1 = make_counter()
c2 = make_counter()
_display(c1())
_newline()
_display(c1())
_newline()
_display(c2())
_newline()
_display(c1())
_newline()
def make_adder(n):
    return lambda x: _add(x, n)
add5 = make_adder(5)
add10 = make_adder(10)
_display(add5(3))
_newline()
_display(add10(3))
_newline()
