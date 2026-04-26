time(0..14).
step(1..14).
cell(1, 1).
visit(1, 1).
cell(1, 2).
visit(1, 2).
cell(1, 3).
visit(1, 3).
cell(2, 1).
visit(2, 1).
cell(2, 2).
visit(2, 2).
cell(2, 3).
visit(2, 3).
cell(3, 1).
visit(3, 1).
cell(3, 2).
visit(3, 2).
cell(3, 3).
visit(3, 3).
start(2, 3).
connected(1, 1, 2, 1).
connected(1, 1, 1, 2).
connected(1, 2, 2, 2).
connected(1, 2, 1, 3).
connected(1, 2, 1, 1).
connected(1, 3, 2, 3).
connected(1, 3, 1, 2).
connected(2, 1, 3, 1).
connected(2, 1, 1, 1).
connected(2, 1, 2, 2).
connected(2, 2, 3, 2).
connected(2, 2, 1, 2).
connected(2, 2, 2, 3).
connected(2, 2, 2, 1).
connected(2, 3, 3, 3).
connected(2, 3, 1, 3).
connected(2, 3, 2, 2).
connected(3, 1, 2, 1).
connected(3, 1, 3, 2).
connected(3, 2, 2, 2).
connected(3, 2, 3, 3).
connected(3, 2, 3, 1).
connected(3, 3, 2, 3).
connected(3, 3, 3, 2).

at(R,C,0) :- start(R,C).
{ move(R1,C1,R2,C2,T) } :- step(T), connected(R1,C1,R2,C2).
:- move(R1,C1,_,_,T), not at(R1,C1,T-1).
:- step(T), #count{R1,C1,R2,C2 : move(R1,C1,R2,C2,T)} > 1.
moved(T) :- move(_,_,_,_,T).
at(R,C,T) :- at(R,C,T-1), step(T), not moved(T).
at(R2,C2,T) :- move(R1,C1,R2,C2,T).
visited(R,C) :- at(R,C,T), time(T).
:- visit(R,C), not visited(R,C).
:~ move(R1,C1,R2,C2,T). [1@1,R1,C1,R2,C2,T]
