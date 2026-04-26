loc(0..5).
req(1..5).
task(1..10).
pos(1..10).
depot(0).
pickup(1,1).
delivery(1,6).
loc(1,2).
loc(6,5).
pickup(2,2).
delivery(2,7).
loc(2,4).
loc(7,2).
pickup(3,3).
delivery(3,8).
loc(3,3).
loc(8,1).
pickup(4,4).
delivery(4,9).
loc(4,3).
loc(9,2).
pickup(5,5).
delivery(5,10).
loc(5,1).
loc(10,5).
dist(0,1,8).
dist(0,2,4).
dist(0,3,7).
dist(0,4,1).
dist(0,5,12).
dist(1,0,5).
dist(1,2,12).
dist(1,3,10).
dist(1,4,11).
dist(1,5,5).
dist(2,0,6).
dist(2,1,8).
dist(2,3,11).
dist(2,4,2).
dist(2,5,11).
dist(3,0,6).
dist(3,1,5).
dist(3,2,8).
dist(3,4,11).
dist(3,5,6).
dist(4,0,8).
dist(4,1,9).
dist(4,2,9).
dist(4,3,10).
dist(4,5,11).
dist(5,0,8).
dist(5,1,2).
dist(5,2,6).
dist(5,3,7).
dist(5,4,3).

1 { order(T,P) : pos(P) } 1 :- task(T).
1 { order(T,P) : task(T) } 1 :- pos(P).

:- pickup(R,TP), delivery(R,TD), order(TP,P1), order(TD,P2), P2 < P1.

step(P,P1) :- pos(P), P1 = P + 1, pos(P1).

:~ order(T,P), P == 1, loc(T,L), depot(D), dist(D,L,W). [W@1,T,P]
:~ order(T1,P), order(T2,P1), step(P,P1),
   loc(T1,L1), loc(T2,L2), dist(L1,L2,W). [W@1,T1,T2,P]
