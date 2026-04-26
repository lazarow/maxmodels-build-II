loc(0..6).
req(1..7).
task(1..14).
pos(1..14).
depot(0).
pickup(1,1).
delivery(1,8).
loc(1,2).
loc(8,3).
pickup(2,2).
delivery(2,9).
loc(2,4).
loc(9,2).
pickup(3,3).
delivery(3,10).
loc(3,2).
loc(10,5).
pickup(4,4).
delivery(4,11).
loc(4,1).
loc(11,2).
pickup(5,5).
delivery(5,12).
loc(5,3).
loc(12,2).
pickup(6,6).
delivery(6,13).
loc(6,1).
loc(13,3).
pickup(7,7).
delivery(7,14).
loc(7,6).
loc(14,5).
dist(0,1,10).
dist(0,2,8).
dist(0,3,6).
dist(0,4,2).
dist(0,5,10).
dist(0,6,7).
dist(1,0,11).
dist(1,2,3).
dist(1,3,2).
dist(1,4,1).
dist(1,5,9).
dist(1,6,2).
dist(2,0,9).
dist(2,1,8).
dist(2,3,8).
dist(2,4,10).
dist(2,5,4).
dist(2,6,5).
dist(3,0,10).
dist(3,1,11).
dist(3,2,7).
dist(3,4,4).
dist(3,5,8).
dist(3,6,7).
dist(4,0,6).
dist(4,1,9).
dist(4,2,10).
dist(4,3,11).
dist(4,5,8).
dist(4,6,1).
dist(5,0,11).
dist(5,1,3).
dist(5,2,8).
dist(5,3,6).
dist(5,4,5).
dist(5,6,10).
dist(6,0,3).
dist(6,1,2).
dist(6,2,7).
dist(6,3,8).
dist(6,4,2).
dist(6,5,8).

1 { order(T,P) : pos(P) } 1 :- task(T).
1 { order(T,P) : task(T) } 1 :- pos(P).

:- pickup(R,TP), delivery(R,TD), order(TP,P1), order(TD,P2), P2 < P1.

step(P,P1) :- pos(P), P1 = P + 1, pos(P1).

:~ order(T,P), P == 1, loc(T,L), depot(D), dist(D,L,W). [W@1,T,P]
:~ order(T1,P), order(T2,P1), step(P,P1),
   loc(T1,L1), loc(T2,L2), dist(L1,L2,W). [W@1,T1,T2,P]
