node(0..30).
edge(18, 30, 3).
edge(22, 27, 5).
edge(9, 30, 6).
edge(17, 20, 4).
edge(25, 3, 6).
edge(15, 17, 2).
edge(3, 23, 2).
edge(27, 5, 4).
edge(24, 26, 3).
edge(18, 2, 2).
edge(23, 8, 6).
edge(6, 14, 3).
edge(1, 9, 1).
edge(1, 27, 4).
edge(22, 29, 1).
edge(2, 14, 4).
edge(6, 2, 6).
edge(29, 10, 4).
edge(11, 10, 1).
edge(13, 30, 2).
edge(13, 22, 2).
edge(27, 15, 3).
edge(26, 14, 3).
edge(7, 19, 3).
edge(3, 21, 5).
edge(20, 28, 5).
edge(11, 7, 1).
edge(17, 23, 6).
edge(29, 6, 3).
edge(5, 16, 5).
edge(13, 14, 6).
edge(18, 10, 1).
edge(8, 30, 4).
edge(27, 14, 1).
edge(9, 10, 2).
edge(28, 23, 4).
edge(4, 13, 1).
edge(30, 23, 3).
edge(17, 26, 1).
edge(11, 29, 5).
edge(19, 15, 3).
edge(23, 15, 5).
edge(24, 20, 6).
edge(20, 1, 5).
edge(15, 21, 6).
edge(16, 13, 2).
edge(7, 27, 4).
edge(25, 21, 2).
edge(4, 15, 3).
edge(11, 3, 2).
edge(4, 23, 6).
edge(28, 3, 4).
edge(8, 19, 2).
edge(15, 2, 2).
edge(10, 1, 4).
edge(9, 20, 4).
edge(25, 22, 5).
edge(17, 25, 3).
edge(20, 24, 1).
edge(24, 29, 2).
edge(12, 21, 1).
edge(0, 11, 3).
edge(21, 9, 3).
edge(5, 30, 2).
edge(6, 21, 2).
edge(3, 19, 6).
edge(18, 0, 3).
edge(25, 8, 3).
edge(15, 0, 5).
edge(21, 18, 2).
edge(17, 11, 2).
edge(23, 25, 5).
edge(21, 7, 4).

{in(U,V)} :- edge(U,V,_).
in_cycle(U) :- in(U,_).
in_cycle(V) :- in(_,V).
:- in_cycle(U), #count{V : in(U,V)} != 1.
:- in_cycle(V), #count{U : in(U,V)} != 1.
cycle_exists :- in_cycle(V).
1 { start(V) : in_cycle(V) } 1 :- cycle_exists.
reach(V) :- start(V).
reach(V) :- reach(U), in(U,V).
:- in_cycle(V), not reach(V).
:~ edge(U,V,W), not in(U,V). [W@1,U,V]
