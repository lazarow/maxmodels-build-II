node(0..24).
pos(1..25).
edge(12, 11, 5).
edge(20, 18, 5).
edge(4, 11, 2).
edge(4, 7, 2).
edge(18, 1, 3).
edge(7, 12, 5).
edge(8, 5, 5).
edge(0, 19, 5).
edge(21, 12, 2).
edge(7, 4, 1).
edge(22, 12, 2).
edge(11, 10, 3).
edge(12, 4, 1).
edge(16, 9, 5).
edge(9, 0, 3).
edge(9, 14, 1).
edge(22, 24, 3).
edge(13, 14, 2).
edge(10, 13, 2).
edge(22, 15, 4).
edge(8, 0, 2).
edge(0, 15, 5).
edge(15, 12, 5).
edge(23, 11, 1).
edge(4, 6, 3).
edge(2, 17, 2).
edge(4, 20, 5).
edge(14, 20, 2).
edge(3, 14, 1).
edge(6, 20, 2).
edge(15, 19, 4).
edge(14, 7, 4).
edge(2, 13, 5).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
