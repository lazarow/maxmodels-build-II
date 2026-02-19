#!/usr/bin/env python3
"""
Comparator for baseline.log vs cce.log.
Compares CDCL metrics for each pX.lp problem.
"""

import re
import sys
from pathlib import Path

# Ensure UTF-8 output on Windows
if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8")


def parse_log(filepath: Path) -> dict[str, dict[str, float]]:
    """Parse a log file and extract CDCL metrics per problem."""
    content = filepath.read_text()
    problems = {}
    current_problem = None

    for line in content.splitlines():
        if m := re.match(r"Processing (p\d+\.lp)", line):
            current_problem = m.group(1)
            problems[current_problem] = {}

        if current_problem is None:
            continue

        if m := re.search(r"% CDCL CPU time:\s*([\d.]+)", line):
            problems[current_problem]["cpu_time"] = float(m.group(1))
        elif m := re.search(r"% CDCL conflicts:\s*(\d+)", line):
            problems[current_problem]["conflicts"] = int(m.group(1))
        elif m := re.search(r"% CDCL decisions:\s*(\d+)", line):
            problems[current_problem]["decisions"] = int(m.group(1))
        elif m := re.search(r"% CDCL propagations:\s*(\d+)", line):
            problems[current_problem]["propagations"] = int(m.group(1))
        elif m := re.search(r"% CDCL LAM execution \(cores found\):\s*(\d+)", line):
            problems[current_problem]["lam_cores"] = int(m.group(1))

    return problems


def format_value(val) -> str:
    """Format numeric value for display."""
    if isinstance(val, float):
        return str(val)
    return str(val)


def compare(baseline: dict, cce: dict) -> None:
    """Print comparison of baseline vs cce for each problem."""
    all_problems = sorted(set(baseline.keys()) | set(cce.keys()))

    for problem in all_problems:
        b = baseline.get(problem, {})
        c = cce.get(problem, {})

        if not b or not c:
            print(f"[{problem}]")
            if not b:
                print("  (missing in baseline)")
            if not c:
                print("  (missing in cce)")
            print()
            continue

        print(f"[{problem}]")

        metrics = [
            ("CDCL CPU time", "cpu_time"),
            ("CDCL conflicts", "conflicts"),
            ("CDCL decisions", "decisions"),
            ("CDCL propagations", "propagations"),
            ("CDCL LAM execution (cores found)", "lam_cores"),
        ]

        for label, key in metrics:
            b_val = b.get(key)
            c_val = c.get(key)
            if b_val is None or c_val is None:
                continue

            if c_val > b_val:
                arrow = "↑"  # CCE greater
            elif c_val < b_val:
                arrow = "↓"  # CCE lower
            else:
                arrow = "="

            print(f"{label} {format_value(b_val)} vs {format_value(c_val)} {arrow}")


def main():
    baseline_path = Path("baseline.log")
    cce_path = Path("cce.log")

    if not baseline_path.exists():
        print(f"Error: {baseline_path} not found", file=sys.stderr)
        sys.exit(1)
    if not cce_path.exists():
        print(f"Error: {cce_path} not found", file=sys.stderr)
        sys.exit(1)

    baseline = parse_log(baseline_path)
    cce = parse_log(cce_path)

    compare(baseline, cce)


if __name__ == "__main__":
    main()
