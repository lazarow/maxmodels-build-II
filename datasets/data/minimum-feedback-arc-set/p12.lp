node(0..24).
pos(1..25).
edge(2, 11, 1).
edge(4, 7, 5).
edge(13, 21, 1).
edge(6, 5, 1).
edge(21, 22, 5).
edge(15, 6, 5).
edge(13, 0, 1).
edge(7, 5, 4).
edge(21, 9, 5).
edge(2, 1, 2).
edge(17, 10, 1).
edge(8, 9, 1).
edge(17, 23, 2).
edge(5, 22, 4).
edge(16, 2, 4).
edge(3, 22, 5).
edge(17, 4, 3).
edge(18, 5, 1).
edge(5, 4, 3).
edge(9, 14, 5).
edge(23, 21, 1).
edge(18, 11, 3).
edge(21, 13, 2).
edge(22, 7, 5).
edge(24, 10, 3).
edge(8, 3, 1).
edge(2, 5, 1).
edge(11, 4, 3).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
