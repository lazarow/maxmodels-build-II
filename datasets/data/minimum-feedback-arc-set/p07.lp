node(0..25).
pos(1..26).
edge(5, 15, 5).
edge(18, 23, 2).
edge(14, 2, 2).
edge(23, 15, 3).
edge(15, 24, 3).
edge(19, 7, 4).
edge(18, 6, 1).
edge(22, 21, 2).
edge(1, 22, 2).
edge(12, 21, 4).
edge(0, 12, 5).
edge(25, 16, 5).
edge(11, 16, 1).
edge(23, 14, 4).
edge(20, 22, 3).
edge(13, 10, 2).
edge(12, 24, 1).
edge(0, 8, 5).
edge(19, 2, 5).
edge(9, 0, 3).
edge(22, 15, 3).
edge(15, 19, 4).
edge(17, 10, 5).
edge(6, 15, 4).
edge(1, 16, 1).
edge(20, 14, 3).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
