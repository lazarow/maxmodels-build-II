loc(0..6).
req(1..6).
task(1..12).
pos(1..12).
depot(0).
pickup(1,1).
delivery(1,7).
loc(1,4).
loc(7,3).
pickup(2,2).
delivery(2,8).
loc(2,1).
loc(8,5).
pickup(3,3).
delivery(3,9).
loc(3,5).
loc(9,3).
pickup(4,4).
delivery(4,10).
loc(4,1).
loc(10,6).
pickup(5,5).
delivery(5,11).
loc(5,1).
loc(11,6).
pickup(6,6).
delivery(6,12).
loc(6,3).
loc(12,5).
dist(0,1,9).
dist(0,2,6).
dist(0,3,4).
dist(0,4,6).
dist(0,5,11).
dist(0,6,3).
dist(1,0,6).
dist(1,2,7).
dist(1,3,11).
dist(1,4,5).
dist(1,5,1).
dist(1,6,2).
dist(2,0,9).
dist(2,1,4).
dist(2,3,9).
dist(2,4,3).
dist(2,5,6).
dist(2,6,11).
dist(3,0,8).
dist(3,1,7).
dist(3,2,1).
dist(3,4,3).
dist(3,5,11).
dist(3,6,8).
dist(4,0,5).
dist(4,1,6).
dist(4,2,8).
dist(4,3,8).
dist(4,5,6).
dist(4,6,11).
dist(5,0,4).
dist(5,1,8).
dist(5,2,7).
dist(5,3,4).
dist(5,4,6).
dist(5,6,11).
dist(6,0,5).
dist(6,1,11).
dist(6,2,3).
dist(6,3,2).
dist(6,4,6).
dist(6,5,9).

1 { order(T,P) : pos(P) } 1 :- task(T).
1 { order(T,P) : task(T) } 1 :- pos(P).

:- pickup(R,TP), delivery(R,TD), order(TP,P1), order(TD,P2), P2 < P1.

step(P,P1) :- pos(P), P1 = P + 1, pos(P1).

:~ order(T,P), P == 1, loc(T,L), depot(D), dist(D,L,W). [W@1,T,P]
:~ order(T1,P), order(T2,P1), step(P,P1),
   loc(T1,L1), loc(T2,L2), dist(L1,L2,W). [W@1,T1,T2,P]
