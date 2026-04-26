node(0..24).
pos(1..25).
edge(7, 8, 3).
edge(10, 22, 3).
edge(23, 5, 3).
edge(16, 2, 4).
edge(16, 9, 4).
edge(2, 0, 1).
edge(2, 4, 1).
edge(14, 0, 4).
edge(12, 0, 2).
edge(0, 21, 2).
edge(5, 19, 3).
edge(4, 1, 2).
edge(0, 20, 5).
edge(16, 15, 5).
edge(21, 6, 4).
edge(24, 23, 3).
edge(0, 4, 3).
edge(2, 3, 2).
edge(17, 1, 5).
edge(6, 9, 2).
edge(17, 8, 2).
edge(18, 20, 3).
edge(11, 7, 5).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
