node(0..24).
pos(1..25).
edge(12, 24, 4).
edge(15, 24, 1).
edge(23, 22, 2).
edge(1, 6, 4).
edge(5, 17, 3).
edge(24, 23, 3).
edge(15, 21, 5).
edge(12, 19, 1).
edge(1, 8, 2).
edge(10, 15, 5).
edge(21, 5, 1).
edge(17, 0, 3).
edge(4, 22, 1).
edge(2, 19, 3).
edge(2, 18, 4).
edge(0, 19, 4).
edge(20, 9, 2).
edge(13, 11, 2).
edge(6, 13, 4).
edge(0, 10, 5).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
