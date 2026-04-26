node(0..25).
pos(1..26).
edge(23, 6, 3).
edge(21, 24, 5).
edge(7, 8, 3).
edge(3, 22, 2).
edge(15, 1, 2).
edge(2, 6, 1).
edge(1, 13, 4).
edge(8, 22, 3).
edge(17, 24, 3).
edge(25, 23, 3).
edge(14, 16, 2).
edge(23, 5, 5).
edge(17, 1, 5).
edge(11, 19, 3).
edge(3, 15, 1).
edge(1, 20, 5).
edge(17, 13, 3).
edge(1, 12, 3).
edge(3, 9, 3).
edge(16, 10, 5).
edge(21, 2, 3).
edge(5, 16, 4).
edge(5, 6, 3).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
