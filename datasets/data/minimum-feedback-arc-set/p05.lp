node(0..24).
pos(1..25).
edge(15, 6, 3).
edge(17, 8, 1).
edge(13, 17, 4).
edge(8, 16, 1).
edge(9, 2, 3).
edge(2, 4, 2).
edge(10, 20, 3).
edge(14, 5, 2).
edge(9, 8, 1).
edge(1, 15, 4).
edge(7, 9, 4).
edge(24, 11, 3).
edge(4, 18, 2).
edge(16, 11, 1).
edge(13, 18, 3).
edge(4, 7, 3).
edge(12, 18, 3).
edge(9, 24, 5).
edge(8, 10, 4).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
