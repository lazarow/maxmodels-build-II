import networkx as nx
import random

def generate():
    node_count = 40
    edge_count = 200
    G = nx.gnm_random_graph(node_count, edge_count, directed=True)
    asp = []
    asp.append(f"v(0..{node_count-1}).")
    asp.append(f"source(0).")
    asp.append(f"target({node_count-1}).")
    for e1, e2 in G.edges:
        asp.append(f"e({e1}, {e2}, {random.randint(1,10)}).")
    asp.append("{in(I, J)} :- e(I, J, W).")
    asp.append("r(X) :- source(X).")
    asp.append("r(Y) :- in(X, Y), source(X).")
    asp.append("r(Y) :- in(X, Y), r(X).")
    asp.append(":- not r(X), target(X).")
    asp.append(":- v(X), 2 {in(X, Y)}.")
    asp.append(":- v(Y), 2 {in(X, Y)}.")
    asp.append(":- in(X, Y), target(X).")
    asp.append(":- in(X, Y), source(Y).")
    asp.append(":- in(X, Y), not r(X).")
    asp.append(":~ in(I, J), e(I, J, W). [-W@1, I, J]")
    return "\n".join(asp)

if __name__ == "__main__":
    asp = generate()
    print(asp)