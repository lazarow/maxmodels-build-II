node(0..24).
pos(1..25).
edge(11, 10, 1).
edge(5, 21, 5).
edge(3, 20, 2).
edge(20, 4, 2).
edge(0, 22, 3).
edge(4, 17, 3).
edge(1, 19, 5).
edge(11, 15, 1).
edge(15, 3, 4).
edge(14, 9, 5).
edge(18, 15, 5).
edge(17, 6, 4).
edge(19, 5, 2).
edge(17, 19, 2).
edge(23, 13, 3).
edge(12, 21, 3).
edge(7, 20, 3).
edge(6, 19, 1).
edge(21, 24, 4).
edge(16, 18, 2).
edge(11, 13, 1).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
