node(0..24).
pos(1..25).
edge(0, 9, 2).
edge(3, 23, 1).
edge(3, 1, 1).
edge(9, 19, 3).
edge(13, 21, 3).
edge(21, 12, 4).
edge(8, 9, 5).
edge(16, 4, 4).
edge(9, 2, 2).
edge(7, 14, 1).
edge(19, 11, 3).
edge(13, 12, 3).
edge(23, 24, 5).
edge(2, 19, 1).
edge(1, 3, 1).
edge(10, 22, 2).
edge(8, 22, 5).
edge(8, 7, 3).
edge(24, 16, 3).
edge(7, 1, 5).
edge(14, 9, 3).
edge(12, 6, 2).
edge(21, 14, 3).
edge(15, 21, 4).
edge(24, 17, 5).
edge(17, 14, 3).
edge(10, 21, 3).
edge(16, 13, 1).
edge(19, 3, 2).
edge(5, 0, 3).
edge(13, 11, 3).
edge(12, 18, 3).
edge(11, 2, 2).
edge(12, 4, 1).
edge(5, 20, 4).
edge(0, 2, 1).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
