node(0..24).
pos(1..25).
edge(11, 14, 1).
edge(14, 2, 4).
edge(11, 6, 3).
edge(22, 11, 2).
edge(16, 0, 2).
edge(9, 8, 2).
edge(24, 3, 3).
edge(2, 23, 1).
edge(21, 5, 2).
edge(18, 8, 2).
edge(14, 8, 4).
edge(20, 4, 5).
edge(9, 11, 1).
edge(2, 14, 4).
edge(23, 21, 5).
edge(19, 4, 5).
edge(21, 13, 5).
edge(15, 2, 3).
edge(5, 22, 5).
edge(17, 8, 3).
edge(7, 1, 3).
edge(19, 13, 4).
edge(18, 13, 3).
edge(4, 20, 4).
edge(18, 24, 4).
edge(7, 5, 4).
edge(4, 9, 2).
edge(20, 21, 1).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
