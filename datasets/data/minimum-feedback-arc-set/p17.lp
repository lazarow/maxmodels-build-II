node(0..24).
pos(1..25).
edge(0, 16, 1).
edge(6, 9, 2).
edge(5, 0, 4).
edge(10, 12, 5).
edge(20, 16, 5).
edge(1, 13, 5).
edge(3, 2, 1).
edge(4, 17, 1).
edge(9, 8, 2).
edge(18, 16, 4).
edge(22, 20, 3).
edge(3, 17, 4).
edge(18, 15, 3).
edge(5, 24, 4).
edge(3, 24, 3).
edge(7, 21, 4).
edge(1, 3, 3).
edge(8, 7, 4).
edge(23, 19, 5).
edge(22, 2, 3).
edge(5, 17, 4).
edge(17, 16, 5).
edge(4, 23, 3).
edge(7, 1, 1).
edge(3, 15, 2).
edge(8, 13, 4).
edge(5, 7, 3).
edge(18, 2, 3).
edge(12, 1, 5).
edge(6, 19, 5).
edge(16, 6, 4).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
