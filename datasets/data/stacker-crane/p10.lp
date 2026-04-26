loc(0..4).
req(1..5).
task(1..10).
pos(1..10).
depot(0).
pickup(1,1).
delivery(1,6).
loc(1,3).
loc(6,1).
pickup(2,2).
delivery(2,7).
loc(2,4).
loc(7,3).
pickup(3,3).
delivery(3,8).
loc(3,4).
loc(8,3).
pickup(4,4).
delivery(4,9).
loc(4,2).
loc(9,4).
pickup(5,5).
delivery(5,10).
loc(5,1).
loc(10,2).
dist(0,1,10).
dist(0,2,3).
dist(0,3,8).
dist(0,4,1).
dist(1,0,5).
dist(1,2,7).
dist(1,3,5).
dist(1,4,11).
dist(2,0,2).
dist(2,1,11).
dist(2,3,9).
dist(2,4,7).
dist(3,0,5).
dist(3,1,2).
dist(3,2,9).
dist(3,4,6).
dist(4,0,1).
dist(4,1,9).
dist(4,2,5).
dist(4,3,5).

1 { order(T,P) : pos(P) } 1 :- task(T).
1 { order(T,P) : task(T) } 1 :- pos(P).

:- pickup(R,TP), delivery(R,TD), order(TP,P1), order(TD,P2), P2 < P1.

step(P,P1) :- pos(P), P1 = P + 1, pos(P1).

:~ order(T,P), P == 1, loc(T,L), depot(D), dist(D,L,W). [W@1,T,P]
:~ order(T1,P), order(T2,P1), step(P,P1),
   loc(T1,L1), loc(T2,L2), dist(L1,L2,W). [W@1,T1,T2,P]
