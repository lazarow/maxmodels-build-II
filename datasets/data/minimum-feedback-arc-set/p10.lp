node(0..24).
pos(1..25).
edge(15, 9, 5).
edge(2, 8, 5).
edge(10, 9, 2).
edge(9, 7, 4).
edge(1, 3, 1).
edge(4, 13, 3).
edge(0, 18, 4).
edge(24, 14, 1).
edge(8, 14, 4).
edge(16, 12, 4).
edge(22, 5, 2).
edge(7, 24, 5).
edge(0, 16, 4).
edge(24, 4, 3).
edge(12, 8, 5).
edge(21, 19, 5).
edge(9, 5, 4).
edge(11, 3, 5).
edge(3, 8, 3).
edge(12, 18, 2).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
