import argparse
import json
import math
import random
import subprocess
import time

def _rand_weight(config, default_min=1, default_max=1):
    min_w = config.get("min_weight", config.get("w_min", default_min))
    max_w = config.get("max_weight", config.get("w_max", default_max))
    return random.randint(min_w, max_w)

def _generate_undirected_edges(nof_nodes, edge_prob, config, max_node_degree=None):
    edges = []
    if edge_prob <= 0 or nof_nodes <= 1:
        return edges
    node_degrees = [0 for _ in range(nof_nodes)]
    pairs = [(i, j) for i in range(nof_nodes) for j in range(i + 1, nof_nodes)]
    random.shuffle(pairs)
    target = int(len(pairs) * edge_prob)
    if target == 0 and edge_prob > 0:
        target = 1
    for i, j in pairs:
        if len(edges) >= target:
            break
        if max_node_degree is not None:
            if node_degrees[i] >= max_node_degree or node_degrees[j] >= max_node_degree:
                continue
        edges.append((i, j, _rand_weight(config, 1, config.get("max_weight", 1))))
        node_degrees[i] += 1
        node_degrees[j] += 1
    return edges

def _generate_directed_edges(nof_nodes, edge_prob, config):
    edges = []
    if edge_prob <= 0 or nof_nodes <= 1:
        return edges
    pairs = [(i, j) for i in range(nof_nodes) for j in range(nof_nodes) if i != j]
    random.shuffle(pairs)
    target = int(len(pairs) * edge_prob)
    if target == 0 and edge_prob > 0:
        target = 1
    for i, j in pairs:
        if len(edges) >= target:
            break
        edges.append((i, j, _rand_weight(config, 1, config.get("max_weight", 1))))
    return edges

# Problem definition:
# There is a directed graph G with N vertices and M edges.
# The vertices are numbered 0,1,...,N-1, and for each i (1<=i<=M),
# the i-th directed edge goes from Vertex x_i to y_i with weight w_i.
# Find a directed path from Vertex 0 to Vertex N-1 with maximum total weight.
# The path may not repeat vertices.
def generate_longest_path(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    edges = _generate_directed_edges(
        config["nof_nodes"],
        config["edge_prob"],
        config
    )
    instance = {
        "config": config,
        "logic_program": "",
    }
    instance["logic_program"] += f"node(0..{config['nof_nodes'] - 1}).\n"
    instance["logic_program"] += f"source(0).\n"
    instance["logic_program"] += f"target({config['nof_nodes'] - 1}).\n"
    for i, j, w in edges:
        instance["logic_program"] += f"edge({i}, {j}, {w}).\n"
    instance["logic_program"] += """{in(I, J)} :- edge(I, J, W).
r(X) :- source(X).
r(Y) :- in(X, Y), r(X).
:- not r(X), target(X).
:- node(X), #count{Y: in(X, Y)} >= 2.
:- node(Y), #count{X: in(X, Y)} >= 2.
:- in(X, Y), target(X).
:- in(X, Y), source(Y).
:- in(X, Y), not r(X).
:~ not in(I, J), edge(I, J, W). [W@1, I, J]
"""
    return instance

# Problem definition:
# There is an undirected graph G with N vertices and M edges.
# The vertices are numbered 0,1,...,N-1, and for each i (1<=i<=M),
# the i-th undirected edge connects Vertex x_i and y_i with weight w_i.
# Partition the vertices into two sets so that the sum of weights of edges
# with endpoints in different sets is maximized.
def generate_max_cut(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    edges = _generate_undirected_edges(
        config["nof_nodes"],
        config["edge_prob"],
        config,
        config.get("max_node_degree")
    )
    instance = {
        "config": config,
        "logic_program": "",
    }
    instance["logic_program"] += f"node(0..{config['nof_nodes'] - 1}).\n"
    for i, j, w in edges:
        instance["logic_program"] += f"edge({i}, {j}, {w}).\n"
    instance["logic_program"] += """
{side(V)} :- node(V).
cut(U,V) :- edge(U,V,_), side(U), not side(V).
cut(U,V) :- edge(U,V,_), not side(U), side(V).
:~ edge(U,V,W), not cut(U,V). [W@1,U,V]
"""
    return instance

# Problem definition:
# There is an undirected graph G with N vertices and M edges.
# The vertices are numbered 0,1,...,N-1, and each vertex v has weight w_v.
# Find a clique (every pair of vertices is connected by an edge) with maximum
# total weight.
def generate_maximal_clique(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    edges = _generate_undirected_edges(
        config["nof_nodes"],
        config["edge_prob"],
        config,
        config.get("max_node_degree")
    )
    node_weights = {i: _rand_weight(config, 1, 1) for i in range(config["nof_nodes"])}
    instance = {
        "config": config,
        "logic_program": "",
    }
    instance["logic_program"] += f"node(0..{config['nof_nodes'] - 1}).\n"
    for i, j, _ in edges:
        instance["logic_program"] += f"edge({i}, {j}).\n"
    for i, w in node_weights.items():
        instance["logic_program"] += f"weight({i}, {w}).\n"
    instance["logic_program"] += """
{in(V)} :- node(V).
:- in(U), in(V), U < V, not edge(U,V).
:~ not in(V), weight(V,W). [W@1,V]
"""
    return instance

# Problem definition:
# There are N items numbered 1,2,...,N and M tests numbered 1,2,...,M.
# Each test i contains a subset of items and has weight w_i.
# A test separates a pair (a,b) if exactly one of a or b is in the test.
# Find a subset of tests with minimum total weight that separates every pair
# of items.
def generate_minimum_test_set(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    n = config["n"]
    m = config["m"]
    item_prob = config.get("test_item_prob", 0.5)
    tests = [set() for _ in range(m)]
    item_masks = [0] * (n + 1)
    for t in range(m):
        bit = 1 << t
        for i in range(1, n + 1):
            if random.random() < item_prob:
                tests[t].add(i)
                item_masks[i] |= bit
        if not tests[t]:
            i = random.randint(1, n)
            tests[t].add(i)
            item_masks[i] |= bit
    for i in range(1, n):
        for j in range(i + 1, n + 1):
            if item_masks[i] == item_masks[j]:
                t = random.randrange(m)
                bit = 1 << t
                tests[t].add(i)
                item_masks[i] |= bit
                if j in tests[t]:
                    tests[t].remove(j)
                    item_masks[j] &= ~bit
    weights = {t + 1: _rand_weight(config, 1, 1) for t in range(m)}
    instance = {
        "config": config,
        "logic_program": "",
    }
    logic_program_parts = [
        f"item(1..{n}).\n",
        f"test(1..{m}).\n",
    ]
    for t in range(m):
        for i in tests[t]:
            logic_program_parts.append(f"contains({t + 1}, {i}).\n")
    for t, w in weights.items():
        logic_program_parts.append(f"weight({t}, {w}).\n")
    for i in range(1, n):
        for j in range(i + 1, n + 1):
            diff_mask = item_masks[i] ^ item_masks[j]
            while diff_mask:
                lsb = diff_mask & -diff_mask
                t = lsb.bit_length()
                logic_program_parts.append(f"separates({t}, {i}, {j}).\n")
                diff_mask ^= lsb
            logic_program_parts.append(f"pair({i}, {j}).\n")
    logic_program_parts.append("""
{choose(T)} :- test(T).
covered(I,J) :- choose(T), separates(T,I,J).
:- pair(I,J), not covered(I,J).
:~ choose(T), weight(T,W). [W@1,T]
""")
    instance["logic_program"] = "".join(logic_program_parts)
    return instance

# Problem definition:
# There is an undirected graph G with N vertices.
# The vertices are numbered 0,1,...,N-1, and each vertex v has weight w_v.
# A set D dominates a vertex if the vertex is in D or adjacent to a vertex in D.
# Find a dominating set with total weight at most B and minimum total weight.
def generate_weight_bounded_dominating_set(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    edges = _generate_undirected_edges(
        config["nof_nodes"],
        config["edge_prob"],
        config,
        config.get("max_node_degree")
    )
    weights = {i: _rand_weight(config, 1, 1) for i in range(config["nof_nodes"])}
    instance = {
        "config": config,
        "logic_program": "",
    }
    instance["logic_program"] += f"node(0..{config['nof_nodes'] - 1}).\n"
    for i, j, _ in edges:
        instance["logic_program"] += f"edge({i}, {j}).\n"
    for i, w in weights.items():
        instance["logic_program"] += f"weight({i}, {w}).\n"
    instance["logic_program"] += f"bound({config['weight_bound']}).\n"
    instance["logic_program"] += """
adj(U,V) :- edge(U,V).
adj(U,V) :- edge(V,U).
{in(V)} :- node(V).
dominated(V) :- in(V).
dominated(V) :- in(U), adj(U,V).
:- node(V), not dominated(V).
:- bound(B), #sum{W,V : in(V), weight(V,W)} > B.
:~ in(V), weight(V,W). [W@1,V]
"""
    return instance

# Problem definition:
# There is an N x N grid of locations. Some locations are unreachable.
# A robot starts at a given reachable location and can move to adjacent
# (up, down, left, right) reachable locations.
# A set of goal locations is given (here: all reachable locations).
# Find a sequence of moves that visits all goal locations with the minimum
# number of moves.
def generate_visit_all(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    rows = config["rows"]
    cols = config["cols"]
    hole_prob = config.get("hole_prob", 0.0)
    steps = config.get("steps", None)
    cells = []
    for r in range(1, rows + 1):
        for c in range(1, cols + 1):
            if random.random() > hole_prob:
                cells.append((r, c))
    if not cells:
        cells.append((1, 1))
    cell_set = set(cells)
    start_r, start_c = random.choice(cells)
    if steps is None:
        steps = max(1, len(cells) * 2)
    instance = {
        "config": config,
        "logic_program": "",
    }
    instance["logic_program"] += f"time(0..{steps}).\n"
    instance["logic_program"] += f"step(1..{steps}).\n"
    for r, c in cells:
        instance["logic_program"] += f"cell({r}, {c}).\n"
        instance["logic_program"] += f"visit({r}, {c}).\n"
    instance["logic_program"] += f"start({start_r}, {start_c}).\n"
    directions = [(1, 0), (-1, 0), (0, 1), (0, -1)]
    for r, c in cells:
        for dr, dc in directions:
            r2 = r + dr
            c2 = c + dc
            if (r2, c2) in cell_set:
                instance["logic_program"] += f"connected({r}, {c}, {r2}, {c2}).\n"
    instance["logic_program"] += """
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
"""
    return instance

# Problem definition:
# There is a directed graph G with N vertices and M edges.
# The vertices are numbered 0,1,...,N-1, and for each i (1<=i<=M),
# the i-th directed edge goes from Vertex x_i to y_i with weight w_i.
# Find a set of edges of minimum total weight whose removal makes G acyclic.
def generate_minimum_feedback_arc_set(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    edges = _generate_directed_edges(config["nof_nodes"], config["edge_prob"], config)
    instance = {
        "config": config,
        "logic_program": "",
    }
    instance["logic_program"] += f"node(0..{config['nof_nodes'] - 1}).\n"
    instance["logic_program"] += f"pos(1..{config['nof_nodes']}).\n"
    for i, j, w in edges:
        instance["logic_program"] += f"edge({i}, {j}, {w}).\n"
    instance["logic_program"] += """
1 { order(V,P) : pos(P) } 1 :- node(V).
1 { order(V,P) : node(V) } 1 :- pos(P).
back(U,V) :- edge(U,V,_), order(U,P1), order(V,P2), P1 > P2.
:~ back(U,V), edge(U,V,W). [W@1,U,V]
"""
    return instance

# Problem definition:
# There is a directed graph G with N vertices and M edges.
# The vertices are numbered 0,1,...,N-1, and for each i (1<=i<=M),
# the i-th directed edge goes from Vertex x_i to y_i with weight w_i.
# Find a directed cycle with maximum total weight.
def generate_longest_circuit(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    edges = _generate_directed_edges(config["nof_nodes"], config["edge_prob"], config)
    instance = {
        "config": config,
        "logic_program": "",
    }
    instance["logic_program"] += f"node(0..{config['nof_nodes'] - 1}).\n"
    for i, j, w in edges:
        instance["logic_program"] += f"edge({i}, {j}, {w}).\n"
    instance["logic_program"] += """
{in(U,V)} :- edge(U,V,_).
in_cycle(U) :- in(U,_).
in_cycle(V) :- in(_,V).
:- in_cycle(U), #count{V : in(U,V)} != 1.
:- in_cycle(V), #count{U : in(U,V)} != 1.
cycle_exists :- in_cycle(V).
1 { start(V) : in_cycle(V) } 1 :- cycle_exists.
reach(V) :- start(V).
reach(V) :- reach(U), in(U,V).
:- in_cycle(V), not reach(V).
:~ edge(U,V,W), not in(U,V). [W@1,U,V]
"""
    return instance

# Problem definition:
# There are L locations numbered 1,2,...,L and a depot at Location 0.
# For each pair of locations (u,v) there is a travel cost c(u,v).
# There are R transport requests; request i has a pickup location p_i and
# a delivery location d_i. A request must be picked up before it is delivered.
# Find an order of serving all pickups and deliveries that minimizes total
# travel cost, starting from the depot.
def generate_stacker_crane(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    nof_locations = config["nof_locations"]
    nof_requests = config["nof_requests"]
    max_distance = config.get("max_distance", 20)

    locations = list(range(1, nof_locations + 1))
    requests = []
    for i in range(1, nof_requests + 1):
        pickup = random.choice(locations)
        delivery = random.choice(locations)
        while delivery == pickup and nof_locations > 1:
            delivery = random.choice(locations)
        requests.append((i, pickup, delivery))

    distances = []
    for u in range(0, nof_locations + 1):
        for v in range(0, nof_locations + 1):
            if u == v:
                continue
            w = random.randint(1, max_distance)
            distances.append((u, v, w))

    instance = {
        "config": config,
        "logic_program": "",
    }
    instance["logic_program"] += f"loc(0..{nof_locations}).\n"
    instance["logic_program"] += f"req(1..{nof_requests}).\n"
    instance["logic_program"] += f"task(1..{2 * nof_requests}).\n"
    instance["logic_program"] += f"pos(1..{2 * nof_requests}).\n"
    instance["logic_program"] += "depot(0).\n"
    for i, p, d in requests:
        pickup_task = i
        delivery_task = i + nof_requests
        instance["logic_program"] += f"pickup({i},{pickup_task}).\n"
        instance["logic_program"] += f"delivery({i},{delivery_task}).\n"
        instance["logic_program"] += f"loc({pickup_task},{p}).\n"
        instance["logic_program"] += f"loc({delivery_task},{d}).\n"
    for u, v, w in distances:
        instance["logic_program"] += f"dist({u},{v},{w}).\n"
    instance["logic_program"] += """
1 { order(T,P) : pos(P) } 1 :- task(T).
1 { order(T,P) : task(T) } 1 :- pos(P).

:- pickup(R,TP), delivery(R,TD), order(TP,P1), order(TD,P2), P2 < P1.

step(P,P1) :- pos(P), P1 = P + 1, pos(P1).

:~ order(T,P), P == 1, loc(T,L), depot(D), dist(D,L,W). [W@1,T,P]
:~ order(T1,P), order(T2,P1), step(P,P1),
   loc(T1,L1), loc(T2,L2), dist(L1,L2,W). [W@1,T1,T2,P]
"""
    return instance

# Problem definition:
# There are N elements numbered 1,2,...,N and M sets numbered 1,2,...,M.
# Each set i contains a subset of the elements and has weight w_i.
# Find a collection of sets with maximum total weight such that no two chosen
# sets share an element.
def generate_set_packing(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    n = config["n"]
    m = config["m"]
    item_prob = config.get("set_item_prob", 0.3)
    sets = {s: set() for s in range(1, m + 1)}
    for s in sets:
        for e in range(1, n + 1):
            if random.random() < item_prob:
                sets[s].add(e)
        if not sets[s]:
            sets[s].add(random.randint(1, n))
    weights = {s: _rand_weight(config, 1, 1) for s in sets}
    instance = {
        "config": config,
        "logic_program": "",
    }
    instance["logic_program"] += f"elem(1..{n}).\n"
    instance["logic_program"] += f"set(1..{m}).\n"
    for s in sets:
        for e in sets[s]:
            instance["logic_program"] += f"in({s}, {e}).\n"
    for s, w in weights.items():
        instance["logic_program"] += f"weight({s}, {w}).\n"
    instance["logic_program"] += """
{ choose(S) } :- set(S).
:- choose(S1), choose(S2), S1 < S2, in(S1,E), in(S2,E).
:~ not choose(S), weight(S,W). [W@1,S]
"""
    return instance

def test(instance):
    start_time = time.perf_counter()
    subprocess.run(
        ["bash", "maxmodels_test.sh"],
        input=instance["logic_program"],
        text=True,
        timeout=600  # 10 minutes
    )
    elapsed = time.perf_counter() - start_time
    config_str = json.dumps(instance["config"])
    print(f"{config_str} - Elapsed time: {elapsed} seconds")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--problem", type=str, required=True)
    parser.add_argument("-c", "--config", type=json.loads, required=True)
    parser.add_argument("--test", action="store_true", required=False)
    args = parser.parse_args()

    if "seed" not in args.config:
        args.config["seed"] = random.randint(0, 400000000)

    generator = None
    if args.problem == "longest-path":
        generator = generate_longest_path
    elif args.problem == "max-cut":
        generator = generate_max_cut
    elif args.problem == "maximal-clique":
        generator = generate_maximal_clique
    elif args.problem == "minimum-test-set":
        generator = generate_minimum_test_set
    elif args.problem == "weight-bounded-dominating-set":
        generator = generate_weight_bounded_dominating_set
    elif args.problem == "visit-all":
        generator = generate_visit_all
    elif args.problem == "minimum-feedback-arc-set":
        generator = generate_minimum_feedback_arc_set
    elif args.problem == "longest-circuit":
        generator = generate_longest_circuit
    elif args.problem == "stacker-crane":
        generator = generate_stacker_crane
    elif args.problem == "set-packing":
        generator = generate_set_packing
    if generator is None:
        raise ValueError(f"Unknown problem: {args.problem}")

    instance = generator(args.config, args.test)
    
    if args.test:
        print(f"Testing instance...")
        test(instance)
    else:
        print(instance["logic_program"])
