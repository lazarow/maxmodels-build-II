node(0..24).
pos(1..25).
edge(19, 3, 2).
edge(4, 22, 4).
edge(6, 7, 2).
edge(14, 17, 2).
edge(3, 14, 2).
edge(4, 16, 2).
edge(2, 5, 5).
edge(16, 19, 5).
edge(22, 3, 3).
edge(21, 17, 4).
edge(16, 1, 5).
edge(7, 0, 4).
edge(14, 7, 1).
edge(5, 11, 3).
edge(11, 0, 3).
edge(24, 23, 3).
edge(14, 12, 1).
edge(2, 6, 2).
edge(14, 23, 1).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
