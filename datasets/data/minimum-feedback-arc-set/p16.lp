node(0..24).
pos(1..25).
edge(5, 6, 4).
edge(9, 6, 2).
edge(16, 5, 3).
edge(18, 7, 1).
edge(7, 22, 4).
edge(12, 10, 3).
edge(13, 1, 5).
edge(19, 17, 1).
edge(17, 11, 3).
edge(6, 20, 5).
edge(17, 21, 4).
edge(23, 5, 3).
edge(0, 22, 4).
edge(15, 24, 2).
edge(4, 23, 3).
edge(9, 20, 2).
edge(9, 5, 1).
edge(2, 8, 3).
edge(4, 19, 4).
edge(6, 15, 4).
edge(2, 13, 4).
edge(3, 11, 4).
edge(6, 22, 3).
edge(8, 11, 3).
edge(6, 5, 3).
edge(9, 8, 3).
edge(22, 21, 4).
edge(14, 20, 2).
edge(24, 10, 4).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
