node(0..24).
pos(1..25).
edge(18, 14, 4).
edge(1, 8, 4).
edge(0, 19, 5).
edge(5, 15, 1).
edge(24, 10, 3).
edge(6, 3, 4).
edge(7, 20, 1).
edge(19, 20, 1).
edge(14, 16, 5).
edge(5, 21, 3).
edge(7, 4, 1).
edge(13, 21, 5).
edge(6, 8, 2).
edge(6, 1, 3).
edge(5, 14, 3).
edge(18, 15, 4).
edge(23, 11, 5).
edge(22, 14, 5).
edge(4, 7, 1).
edge(16, 24, 3).
edge(2, 6, 2).
edge(22, 0, 3).
edge(2, 11, 3).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
