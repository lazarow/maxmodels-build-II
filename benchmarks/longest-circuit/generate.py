import random
from typing import List, Tuple
import argparse

class DirectedGraphInstance:
    def __init__(self, vertices: List[int], arcs: List[Tuple[int, int, int]]):
        self.vertices = vertices
        self.arcs = arcs

    def to_asp(self) -> str:
        """Serialize instance to ASP facts."""
        lines = []
        for v in self.vertices:
            lines.append(f"vtx({v}).")
        for x, y, w in self.arcs:
            lines.append(f"arc({x},{y},{w}).")
        return "\n".join(lines)


def generate_longest_circuit_instance(
    n_vertices: int,
    edge_probability: float = 0.3,
    min_weight: int = 1,
    max_weight: int = 20,
    seed: int | None = None
) -> DirectedGraphInstance:
    if seed is None:
        seed = random.randint(0, 1000000)
    random.seed(seed)
    vertices = list(range(1, n_vertices + 1))
    arcs = set()
    cycle_length = random.randint(2, n_vertices)
    cycle_vertices = random.sample(vertices, cycle_length)
    for i in range(cycle_length):
        x = cycle_vertices[i]
        y = cycle_vertices[(i + 1) % cycle_length]
        w = random.randint(min_weight, max_weight)
        arcs.add((x, y, w))
    for x in vertices:
        for y in vertices:
            if x == y:
                continue
            if random.random() < edge_probability:
                w = random.randint(min_weight, max_weight)
                arcs.add((x, y, w))
    print(f"% n_vertices: {n_vertices}")
    print(f"% arcs: {len(arcs)}")
    print(f"% edge_probability: {edge_probability}")
    print(f"% min_weight: {min_weight}")
    print(f"% max_weight: {max_weight}")
    print(f"% cycle_length: {cycle_length}")
    print(f"% seed: {seed}")
    return DirectedGraphInstance(vertices, list(arcs))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--n_vertices", type=int, default=30)
    parser.add_argument("--edge_probability", type=float, default=0.25)
    parser.add_argument("--seed", type=int, default=None)
    args = parser.parse_args()  
    instance = generate_longest_circuit_instance(
        n_vertices=args.n_vertices,
        edge_probability=args.edge_probability,
        seed=args.seed
    )
    print(instance.to_asp())