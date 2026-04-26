node(0..25).
pos(1..26).
edge(17, 1, 2).
edge(15, 23, 2).
edge(5, 4, 4).
edge(14, 11, 1).
edge(7, 10, 4).
edge(4, 12, 4).
edge(14, 25, 5).
edge(11, 8, 4).
edge(15, 14, 4).
edge(11, 19, 2).
edge(16, 15, 3).
edge(7, 25, 4).
edge(13, 18, 2).
edge(3, 9, 2).
edge(6, 0, 2).
edge(22, 21, 5).
edge(25, 21, 4).
edge(10, 18, 2).
edge(13, 19, 2).
edge(2, 15, 5).
edge(3, 21, 3).
edge(1, 19, 1).
edge(5, 8, 2).
edge(20, 6, 5).
edge(8, 22, 3).
edge(5, 24, 3).

1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
