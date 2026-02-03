import argparse
import json
import random
import subprocess
import time

def generate_longest_path(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    instance = {
        "config": config,
        "logic_program": "",
    }
    nodes = list(range(config["nof_nodes"]))
    edges = set()
    node_degrees = [0 for _ in nodes]
    min_edge_count = (config["nof_nodes"] * (config["nof_nodes"] - 1) // 2) * config["edge_prob"]
    while len(edges) < min_edge_count:
        for i in nodes:
            for j in nodes:
                if i != j and node_degrees[i] < config["max_node_degree"] and node_degrees[j] < config["max_node_degree"] and random.random() < config["edge_prob"]:
                    w = random.randint(1, config["max_weight"])
                    edges.add((i, j, w))
                    node_degrees[i] += 1
                    node_degrees[j] += 1
        if test:
            print(f"Missing edges: {min_edge_count - len(edges)}")
    instance["logic_program"] += f"v(0..{config['nof_nodes'] - 1}).\n"
    instance["logic_program"] += f"source(0).\n"
    instance["logic_program"] += f"target({config['nof_nodes'] - 1}).\n"
    for i, j, w in edges:
        instance["logic_program"] += f"e({i}, {j}, {w}).\n"
    instance["logic_program"] += """{in(I, J)} :- e(I, J, W).
r(X) :- source(X).
r(Y) :- in(X, Y), source(X).
r(Y) :- in(X, Y), r(X).
:- not r(X), target(X).
:- v(X), #count{X, Y: in(X, Y)} >= 2.
:- v(Y), #count{X, Y: in(X, Y)} >= 2.
:- in(X, Y), target(X).
:- in(X, Y), source(Y).
:- in(X, Y), not r(X).
:~ not in(I, J), e(I, J, W). [W@1, I, J]
"""
    return instance

def generate_set_cover(config, test=False):
    if test:
        print(f"Generating test instance...")
    random.seed(config["seed"])
    sets = {s: set() for s in range(1, config["m"] + 1)}
    for e in range(1, config["n"] + 1):
        chosen_sets = random.sample(range(1, config["m"] + 1), config["d"])
        for s in chosen_sets:
            sets[s].add(e)
    for s in sets:
        while len(sets[s]) < config["k"]:
            sets[s].add(random.randint(1, config["n"]))
    weights = {}
    for s in sets:
        base = (config["w_max"] - config["w_min"]) * (config["k"] / len(sets[s]))
        weights[s] = max(config["w_min"], min(config["w_max"], int(base + random.gauss(0, 1))))
    instance = {
        "config": config,
        "logic_program": "",
    }
    instance["logic_program"] += f"elem(1..{config['n']}).\n"
    instance["logic_program"] += f"set(1..{config['m']}).\n"
    for s in sets:
        for e in sets[s]:
            instance["logic_program"] += f"covers({s}, {e}).\n"
    for s in sets:
        instance["logic_program"] += f"weight({s}, {weights[s]}).\n"
    instance["logic_program"] += """
{ choose(S) } :- set(S).
covered(E) :- choose(S), covers(S,E).
:- elem(E), not covered(E).
:~ choose(S), weight(S,W). [W@1, S]
"""
    return instance

def generate_fjsp(config, test=False):
    random.seed(config["seed"])

    J = config["jobs"]
    O = config["ops_per_job"]
    R = config["resources"]
    A = config["alts"]          # alternatywy zasobów
    Tmax = config["tmax"]

    instance = {"config": config, "logic_program": ""}

    instance["logic_program"] += f"time(0..{Tmax}).\n"

    for j in range(1, J+1):
        instance["logic_program"] += f"job({j}).\n"
        for o in range(1, O+1):
            instance["logic_program"] += f"op({j},{o}).\n"
            if o < O:
                instance["logic_program"] += f"prec({j},{o},{o+1}).\n"

    for r in range(1, R+1):
        cost = random.randint(config["c_min"], config["c_max"])
        instance["logic_program"] += f"res({r}). cost({r},{cost}).\n"

    for j in range(1, J+1):
        for o in range(1, O+1):
            dur = random.randint(1, config["dur_max"])
            instance["logic_program"] += f"dur({j},{o},{dur}).\n"

            alts = random.sample(range(1, R+1), A)
            for r in alts:
                instance["logic_program"] += f"alt({j},{o},{r}).\n"

    instance["logic_program"] += """
{ assign(J,O,R) : alt(J,O,R) } = 1 :- op(J,O).
{ start(J,O,T) : time(T) } = 1 :- op(J,O).

:- prec(J,O1,O2),
   start(J,O1,T1), start(J,O2,T2),
   dur(J,O1,D),
   T2 < T1 + D.

:- assign(J1,O1,R), assign(J2,O2,R),
   start(J1,O1,T1), start(J2,O2,T2),
   dur(J1,O1,D1), dur(J2,O2,D2),
   J1 != J2,
   T1 < T2 + D2, T2 < T1 + D1.

:~ assign(J,O,R), cost(R,C). [C@1,J,O,R]
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
        args.config["seed"] = random.randint(0, 1000000)

    generator = None
    if args.problem == "longest-path":
        generator = generate_longest_path
    elif args.problem == "set-cover":
        generator = generate_set_cover
    elif args.problem == "fjsp":
        generator = generate_fjsp
    if generator is None:
        raise ValueError(f"Unknown problem: {args.problem}")

    instance = generator(args.config, args.test)
    
    if args.test:
        print(f"Testing instance...")
        test(instance)
    else:
        print(instance["logic_program"])
