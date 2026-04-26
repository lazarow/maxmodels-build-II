node(0..24).
pos(1..25).
edge(16, 10, 5).
edge(2, 13, 4).
edge(21, 10, 5).
edge(10, 16, 5).
edge(0, 13, 4).
edge(3, 21, 3).
edge(10, 17, 2).
edge(19, 7, 3).
edge(11, 12, 1).
edge(10, 12, 1).
edge(11, 0, 3).
edge(10, 0, 5).
edge(22, 9, 5).
edge(3, 22, 1).
edge(16, 3, 1).
edge(15, 11, 4).
edge(15, 19, 2).
edge(7, 16, 2).
edge(10, 7, 5).
edge(20, 14, 5).
edge(8, 17, 5).
edge(20, 13, 5).
edge(13, 6, 4).
edge(19, 12, 5).
edge(16, 8, 4).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
