import json
import os
import random
import signal
import subprocess
import time
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import generator as gen

TARGET_BUCKETS: List[Tuple[int, int]] = [
    (10, 30),
    (30, 70),
    (70, 120),
]

def _clamp(value: float, min_value: float, max_value: float) -> float:
    return max(min_value, min(max_value, value))


def _lerp(a: float, b: float, t: float) -> float:
    return a + (b - a) * t


def _rand_int(rng: random.Random, a: int, b: int) -> int:
    return rng.randint(a, b)


def _jitter(rng: random.Random, value: float, span: float) -> float:
    return value + rng.uniform(-span, span)


def build_longest_path(scale: float, rng: random.Random) -> Dict[str, Any]:
    nof_nodes = int(_lerp(20, 200, scale))
    edge_prob = _clamp(_jitter(rng, _lerp(0.08, 0.55, scale), 0.05), 0.02, 0.9)
    max_weight = int(_lerp(5, 40, scale))
    return {
        "seed": _rand_int(rng, 0, 400000000),
        "nof_nodes": max(2, nof_nodes),
        "edge_prob": edge_prob,
        "min_weight": 1,
        "max_weight": max_weight,
    }


def build_max_cut(scale: float, rng: random.Random) -> Dict[str, Any]:
    nof_nodes = int(_lerp(40, 220, scale))
    edge_prob = _clamp(_jitter(rng, _lerp(0.06, 0.5, scale), 0.05), 0.02, 0.9)
    max_weight = int(_lerp(5, 30, scale))
    return {
        "seed": _rand_int(rng, 0, 400000000),
        "nof_nodes": max(2, nof_nodes),
        "edge_prob": edge_prob,
        "min_weight": 1,
        "max_weight": max_weight,
    }


def build_maximal_clique(scale: float, rng: random.Random) -> Dict[str, Any]:
    nof_nodes = int(_lerp(30, 180, scale))
    edge_prob = _clamp(_jitter(rng, _lerp(0.1, 0.6, scale), 0.06), 0.02, 0.95)
    max_weight = int(_lerp(5, 25, scale))
    return {
        "seed": _rand_int(rng, 0, 400000000),
        "nof_nodes": max(2, nof_nodes),
        "edge_prob": edge_prob,
        "min_weight": 1,
        "max_weight": max_weight,
    }


def build_minimum_test_set(scale: float, rng: random.Random) -> Dict[str, Any]:
    n = int(_lerp(10, 100, scale))
    m = int(_lerp(10, 160, scale))
    test_item_prob = _clamp(_jitter(rng, _lerp(0.25, 0.7, scale), 0.08), 0.05, 0.95)
    max_weight = int(_lerp(3, 12, scale))
    return {
        "seed": _rand_int(rng, 0, 400000000),
        "n": max(2, n),
        "m": max(2, m),
        "test_item_prob": test_item_prob,
        "min_weight": 1,
        "max_weight": max_weight,
    }


def build_weight_bounded_dominating_set(scale: float, rng: random.Random) -> Dict[str, Any]:
    nof_nodes = int(_lerp(30, 250, scale))
    edge_prob = _clamp(_jitter(rng, _lerp(0.05, 0.45, scale), 0.05), 0.02, 0.9)
    max_weight = int(_lerp(3, 12, scale))
    bound_ratio = _clamp(_jitter(rng, _lerp(0.25, 0.65, scale), 0.07), 0.1, 0.9)
    weight_bound = int(max(1, nof_nodes * bound_ratio))
    return {
        "seed": _rand_int(rng, 0, 400000000),
        "nof_nodes": max(2, nof_nodes),
        "edge_prob": edge_prob,
        "min_weight": 1,
        "max_weight": max_weight,
        "weight_bound": weight_bound,
    }


def build_visit_all(scale: float, rng: random.Random) -> Dict[str, Any]:
    rows = int(_lerp(3, 18, scale))
    cols = int(_lerp(3, 18, scale))
    hole_prob = _clamp(_jitter(rng, _lerp(0.0, 0.25, scale), 0.05), 0.0, 0.7)
    cells = max(1, rows * cols)
    steps = int(cells * _lerp(1.4, 4.0, scale))
    return {
        "seed": _rand_int(rng, 0, 400000000),
        "rows": max(1, rows),
        "cols": max(1, cols),
        "hole_prob": hole_prob,
        "steps": max(1, steps),
    }


def build_minimum_feedback_arc_set(scale: float, rng: random.Random) -> Dict[str, Any]:
    nof_nodes = int(_lerp(25, 180, scale))
    edge_prob = _clamp(_jitter(rng, _lerp(0.08, 0.55, scale), 0.05), 0.02, 0.9)
    max_weight = int(_lerp(5, 40, scale))
    return {
        "seed": _rand_int(rng, 0, 400000000),
        "nof_nodes": max(2, nof_nodes),
        "edge_prob": edge_prob,
        "min_weight": 1,
        "max_weight": max_weight,
    }


def build_longest_circuit(scale: float, rng: random.Random) -> Dict[str, Any]:
    nof_nodes = int(_lerp(25, 180, scale))
    edge_prob = _clamp(_jitter(rng, _lerp(0.08, 0.55, scale), 0.05), 0.02, 0.9)
    max_weight = int(_lerp(5, 40, scale))
    return {
        "seed": _rand_int(rng, 0, 400000000),
        "nof_nodes": max(2, nof_nodes),
        "edge_prob": edge_prob,
        "min_weight": 1,
        "max_weight": max_weight,
    }


def build_stacker_crane(scale: float, rng: random.Random) -> Dict[str, Any]:
    nof_locations = int(_lerp(4, 30, scale))
    nof_requests = int(_lerp(4, 45, scale))
    max_distance = int(_lerp(10, 60, scale))
    return {
        "seed": _rand_int(rng, 0, 400000000),
        "nof_locations": max(2, nof_locations),
        "nof_requests": max(1, nof_requests),
        "max_distance": max(2, max_distance),
    }


def build_set_packing(scale: float, rng: random.Random) -> Dict[str, Any]:
    n = int(_lerp(30, 200, scale))
    m = int(_lerp(40, 350, scale))
    set_item_prob = _clamp(_jitter(rng, _lerp(0.2, 0.77, scale), 0.08), 0.02, 0.95)
    max_weight = int(_lerp(3, 12, scale))
    return {
        "seed": _rand_int(rng, 0, 400000000),
        "n": max(2, n),
        "m": max(2, m),
        "set_item_prob": set_item_prob,
        "min_weight": 1,
        "max_weight": max_weight,
    }

PROBLEM_BUILDERS = {
    "set-packing": build_set_packing,
    "weight-bounded-dominating-set": build_weight_bounded_dominating_set,
    "longest-path": build_longest_path,
    "max-cut": build_max_cut,
    "maximal-clique": build_maximal_clique,
    "minimum-test-set": build_minimum_test_set,
    "visit-all": build_visit_all,
    "minimum-feedback-arc-set": build_minimum_feedback_arc_set,
    "longest-circuit": build_longest_circuit,
    "stacker-crane": build_stacker_crane,
}

GENERATOR_BY_PROBLEM = {
    "longest-path": gen.generate_longest_path,
    "max-cut": gen.generate_max_cut,
    "maximal-clique": gen.generate_maximal_clique,
    "minimum-test-set": gen.generate_minimum_test_set,
    "weight-bounded-dominating-set": gen.generate_weight_bounded_dominating_set,
    "visit-all": gen.generate_visit_all,
    "minimum-feedback-arc-set": gen.generate_minimum_feedback_arc_set,
    "longest-circuit": gen.generate_longest_circuit,
    "stacker-crane": gen.generate_stacker_crane,
    "set-packing": gen.generate_set_packing,
}

def run_test_instance(logic_program: str, timeout: int, script_path: Path) -> Optional[float]:
    start_time = time.perf_counter()
    try:
        proc = subprocess.Popen(
            ["bash", str(script_path)],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            start_new_session=True,  # Process group so we can kill entire pipeline on timeout
        )
        stdout, _ = proc.communicate(input=logic_program, timeout=timeout)
        if stdout is not None:
            for line in stdout.splitlines():
                if line.startswith("% CDCL CPU time:"):
                    try:
                        value = float(line.split(":")[1].strip())
                        return value
                    except Exception:
                        pass
    except subprocess.TimeoutExpired:
        # Kill entire process group (bash + gringo + smodels + maxmodels + wmaxcdcl chain)
        # Prevents orphaned processes from accumulating on the server
        if hasattr(os, "killpg"):
            try:
                os.killpg(os.getpgid(proc.pid), signal.SIGKILL)
            except (ProcessLookupError, OSError):
                pass
        else:
            proc.kill()
        proc.wait()
        return None
    return time.perf_counter() - start_time

def adaptive_search(
    problem: str,
    builder,
    generator,
    rng: random.Random,
    target_min: int,
    target_max: int,
    max_attempts: int,
    timeout_buffer: int,
    output_path: Path,
    scale_min: float = 0.0,
    scale_max: float = 1.0,
) -> List[Dict[str, Any]]:
    found: List[Dict[str, Any]] = []
    seen_configs = set()
    low, high = scale_min, scale_max
    too_fast_streak = 0
    too_slow_streak = 0
    script_path = "maxmodels.sh"

    for attempt in range(1, max_attempts + 1):
        if len(found) >= 10:
            break

        if low >= high:
            print(
                f"[{problem}] reset scale range ({low:.3f}-{high:.3f}) "
                f"-> ({scale_min:.3f}-{scale_max:.3f})"
            )
            low, high = scale_min, scale_max

        scale = rng.uniform(low, high)
        config = builder(scale, rng)
        config_key = json.dumps(config, sort_keys=True)
        if config_key in seen_configs:
            print(
                f"[{problem}] attempt {attempt}/{max_attempts}: "
                f"duplicate config, skipping"
            )
            continue
        seen_configs.add(config_key)

        instance = generator(config, False)
        elapsed = run_test_instance(
            instance["logic_program"],
            timeout=target_max + timeout_buffer,
            script_path=script_path,
        )

        if elapsed is None or elapsed > target_max:
            too_slow_streak += 1
            too_fast_streak = 0
            high = min(high, scale)
            elapsed_label = "timeout" if elapsed is None else f"{elapsed:.2f}s"
            print(
                f"[{problem}] attempt {attempt}/{max_attempts}: "
                f"scale {scale:.3f} in {low:.3f}-{high:.3f}, "
                f"elapsed {elapsed_label} -> too slow "
                f"(streak {too_slow_streak})"
            )
        elif elapsed < target_min:
            too_fast_streak += 1
            too_slow_streak = 0
            low = max(low, scale)
            print(
                f"[{problem}] attempt {attempt}/{max_attempts}: "
                f"scale {scale:.3f} in {low:.3f}-{high:.3f}, "
                f"elapsed {elapsed:.2f}s -> too fast "
                f"(streak {too_fast_streak})"
            )
        else:
            too_fast_streak = 0
            too_slow_streak = 0
            record = {
                "problem": problem,
                "target_range": [target_min, target_max],
                "elapsed_seconds": elapsed,
                "config": config,
            }
            found.append(record)
            with output_path.open("a", encoding="utf-8") as handle:
                handle.write(json.dumps(record, sort_keys=True) + "\n")
            print(
                f"[{problem}] attempt {attempt}/{max_attempts}: "
                f"scale {scale:.3f} in {low:.3f}-{high:.3f}, "
                f"elapsed {elapsed:.2f}s -> accepted "
                f"({len(found)}/10 for {target_min}-{target_max}s)"
            )

        if too_fast_streak >= 8 or too_slow_streak >= 8:
            print(
                f"[{problem}] reset streaks after "
                f"{max(too_fast_streak, too_slow_streak)} misses"
            )
            low, high = scale_min, scale_max
            too_fast_streak = 0
            too_slow_streak = 0

    return found


def main() -> None:
    rng = random.Random(time.time())

    for problem in PROBLEM_BUILDERS.keys():
        if problem not in PROBLEM_BUILDERS or problem not in GENERATOR_BY_PROBLEM:
            raise ValueError(f"Unknown problem: {problem}")

        builder = PROBLEM_BUILDERS[problem]
        generator = GENERATOR_BY_PROBLEM[problem]
        output_path = Path(f"{problem}.jsonl")

        for target_min, target_max in TARGET_BUCKETS:
            existing = []
            if output_path.exists():
                with output_path.open("r", encoding="utf-8") as handle:
                    for line in handle:
                        try:
                            data = json.loads(line)
                        except json.JSONDecodeError:
                            continue
                        if (
                            data.get("problem") == problem
                            and data.get("target_range") == [target_min, target_max]
                        ):
                            existing.append(data)
            if len(existing) >= 10:
                print(
                    f"[{problem}] target {target_min}-{target_max}s already has "
                    f"{len(existing)} instances, skipping."
                )
                continue

            print(
                f"[{problem}] searching for target {target_min}-{target_max}s "
                f"(need {10 - len(existing)})"
            )
            found = adaptive_search(
                problem=problem,
                builder=builder,
                generator=generator,
                rng=rng,
                target_min=target_min,
                target_max=target_max,
                max_attempts=250,
                timeout_buffer=5,
                output_path=output_path,
            )
            if len(found) + len(existing) < 10:
                print(
                    f"[{problem}] warning: only {len(found) + len(existing)} "
                    f"instances found for {target_min}-{target_max}s"
                )

if __name__ == "__main__":
    main()
