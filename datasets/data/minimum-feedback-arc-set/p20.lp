node(0..24).
pos(1..25).
edge(16, 4, 4).
edge(3, 4, 4).
edge(5, 22, 1).
edge(18, 19, 2).
edge(15, 16, 3).
edge(16, 15, 3).
edge(13, 9, 3).
edge(20, 23, 3).
edge(13, 18, 3).
edge(9, 8, 1).
edge(20, 10, 2).
edge(8, 20, 2).
edge(13, 1, 3).
edge(20, 3, 2).
edge(17, 22, 1).
edge(19, 0, 5).
edge(8, 14, 4).
edge(15, 13, 4).
edge(7, 3, 2).
edge(8, 7, 1).
edge(6, 0, 5).
edge(22, 16, 3).
edge(13, 11, 2).
edge(19, 2, 1).
edge(12, 2, 5).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
