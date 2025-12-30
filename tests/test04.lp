v(0..5).
source(0).
target(5).
e(0, 1, 6).
e(0, 2, 6).
e(1, 2, 6).
e(2, 1, 6).
e(1, 3, 6).
e(2, 3, 6).
e(3, 1, 6).
e(3, 2, 6).
e(3, 4, 6).
e(2, 4, 6).
e(4, 2, 6).
e(3, 5, 6).
e(4, 5, 6).


{in(I, J)} :- e(I, J, W).
r(X) :- source(X).
r(Y) :- in(X, Y), source(X).
r(Y) :- in(X, Y), r(X).
:- not r(X), target(X).
:- v(X), #count{X, Y: in(X, Y)} >= 2.
:- v(Y), #count{X, Y: in(X, Y)} >= 2.
:- in(X, Y), target(X).
:- in(X, Y), source(Y).
:- in(X, Y), not r(X).
:~ not in(I, J), e(I, J, W). [W@1, I, J]
