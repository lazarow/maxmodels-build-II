node(0..25).
pos(1..26).
edge(15, 4, 1).
edge(22, 17, 4).
edge(9, 20, 5).
edge(4, 16, 3).
edge(8, 24, 2).
edge(18, 24, 3).
edge(9, 3, 3).
edge(21, 5, 5).
edge(2, 17, 2).
edge(17, 12, 1).
edge(9, 25, 5).
edge(20, 19, 4).
edge(18, 23, 5).
edge(3, 12, 4).
edge(0, 25, 5).
edge(9, 6, 4).
edge(9, 14, 3).
edge(2, 8, 2).
edge(19, 14, 2).
edge(12, 18, 1).
edge(17, 3, 4).
edge(10, 16, 5).
edge(19, 2, 2).
edge(19, 23, 2).
edge(25, 0, 1).
edge(18, 21, 4).
edge(17, 13, 3).
edge(24, 18, 1).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
