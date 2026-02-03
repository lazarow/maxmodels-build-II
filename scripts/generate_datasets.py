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

    if generator is None:
        raise ValueError(f"Unknown problem: {args.problem}")

    instance = generator(args.config, args.test)
    
    if args.test:
        print(f"Testing instance...")
        test(instance)
    else:
        print(instance["logic_program"])
