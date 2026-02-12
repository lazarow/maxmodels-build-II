import json
import sys
from pathlib import Path

import generator as gen

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

def problem_name_from_jsonl_path(jsonl_path: Path) -> str:
    return jsonl_path.stem


def load_configs(jsonl_path: Path) -> list[dict]:
    configs = []
    with jsonl_path.open("r", encoding="utf-8") as f:
        for line_num, line in enumerate(f, start=1):
            line = line.strip()
            if not line:
                continue
            try:
                data = json.loads(line)
            except json.JSONDecodeError as e:
                raise ValueError(
                    f"{jsonl_path}: invalid JSON on line {line_num}: {e}"
                ) from e
            config = data.get("config")
            if config is None:
                raise ValueError(
                    f"{jsonl_path}: line {line_num} has no 'config' key"
                )
            configs.append(config)
    return configs


def generate_instances(
    jsonl_dir: Path,
    output_base: Path,
    problem_filter: list[str] | None = None,
) -> None:
    jsonl_files = sorted(jsonl_dir.glob("*.jsonl"))

    if not jsonl_files:
        print(f"No .jsonl files found in {jsonl_dir}", file=sys.stderr)
        sys.exit(1)

    for jsonl_path in jsonl_files:
        problem = problem_name_from_jsonl_path(jsonl_path)

        if problem_filter is not None and problem not in problem_filter:
            continue

        if problem not in GENERATOR_BY_PROBLEM:
            print(
                f"Skipping {jsonl_path.name}: unknown problem '{problem}'",
                file=sys.stderr,
            )
            continue

        generator_fn = GENERATOR_BY_PROBLEM[problem]
        configs = load_configs(jsonl_path)

        out_dir = output_base / problem
        out_dir.mkdir(parents=True, exist_ok=True)

        for i, config in enumerate(configs, start=1):
            instance = generator_fn(config, test=False)
            lp_path = out_dir / f"p{i:02d}.lp"
            lp_path.write_text(instance["logic_program"], encoding="utf-8")

        print(f"Generated {len(configs)} instances for {problem} -> {out_dir}")


def main() -> None:

    output_base = Path("../data")
    if not output_base.is_absolute():
        output_base = (output_base).resolve()
    output_base.mkdir(parents=True, exist_ok=True)

    generate_instances(
        jsonl_dir=(Path(".")).resolve(),
        output_base=output_base,
    )

if __name__ == "__main__":
    main()
