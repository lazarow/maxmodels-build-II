node(0..24).
pos(1..25).
edge(24, 11, 2).
edge(20, 10, 3).
edge(14, 17, 5).
edge(1, 5, 5).
edge(10, 1, 5).
edge(3, 10, 2).
edge(11, 19, 2).
edge(1, 9, 1).
edge(13, 7, 1).
edge(7, 11, 5).
edge(23, 0, 4).
edge(21, 1, 1).
edge(11, 4, 1).
edge(7, 20, 5).
edge(11, 12, 2).
edge(23, 11, 3).
edge(8, 23, 3).
edge(7, 1, 3).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
