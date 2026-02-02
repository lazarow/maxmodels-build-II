{ lc(X, Y) : arc(X, Y, L) } <= 1 :- vtx(X).

occurs(X) :- lc(X, Y), arc(X, Y, L).
{initial(X)} :- occurs(X), vtx(X).
:- vtx(X), vtx(Y), occurs(X), initial(Y), X < Y.

r(Y) :- lc(X, Y), initial(X), arc(X, Y, L).
r(Y) :- lc(X, Y), r(X), arc(X, Y, L).

:- vtx(X), occurs(X), not r(X).

:~ arc(X, Y, L), not lc(X, Y). [L@1, X, Y]
