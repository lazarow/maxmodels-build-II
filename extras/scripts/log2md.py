#!/usr/bin/env python3
"""
Analyze a single maxmodels-style .log file and produce a Markdown report.

Reads one log, prompts the user for a free-form description, and prints a
Markdown document with per-instance CDCL stats in a compact one-line format:

    p01 cpu: 12.7258 conf: 37239 dec: 58543 prop: 7425299 cflit: 1451848 lam: 20245 sla: 1.33453
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from datetime import datetime

RE_PROCESSING = re.compile(r"^Processing\s+(.+?)\s*$")
RE_COST = re.compile(r"^COST\s+(\d+)\s*$")
RE_OPTIMUM = re.compile(r"^OPTIMUM\s*$")

RE_CDCL_CPU = re.compile(r"^%\s*CDCL CPU time:\s*([\d.]+)\s*$")
RE_CDCL_CONFLICTS = re.compile(r"^%\s*CDCL conflicts:\s*(\d+)\s*$")
RE_CDCL_DECISIONS = re.compile(r"^%\s*CDCL decisions:\s*(\d+)\s*$")
RE_CDCL_PROPAGATIONS = re.compile(r"^%\s*CDCL propagations:\s*(\d+)\s*$")
RE_CDCL_CONFLICT_LITS = re.compile(r"^%\s*CDCL conflict literals:\s*(\d+)\s*$")
RE_CDCL_LAM = re.compile(r"^%\s*CDCL LAM execution \(cores found\):\s*(\d+)\s*$")
RE_CDCL_SLA = re.compile(r"^%\s*CDCL SLA time:\s*([\d.]+)\s*$")


@dataclass
class Instance:
    name: str
    cost: int | None = None
    optimum: bool = False
    cpu_s: float | None = None
    conflicts: int | None = None
    decisions: int | None = None
    propagations: int | None = None
    conflict_literals: int | None = None
    lam_cores: int | None = None
    sla_s: float | None = None


@dataclass
class LogData:
    instances: list[Instance] = field(default_factory=list)


def parse_log(path: Path) -> LogData:
    data = LogData()
    current: Instance | None = None

    text = path.read_text(encoding="utf-8", errors="replace")
    for raw in text.splitlines():
        line = raw.rstrip("\n")

        m = RE_PROCESSING.match(line)
        if m:
            current = Instance(name=m.group(1).strip())
            data.instances.append(current)
            continue

        if current is None:
            continue

        if m := RE_COST.match(line):
            current.cost = int(m.group(1))
        elif RE_OPTIMUM.match(line):
            current.optimum = True
        elif m := RE_CDCL_CPU.match(line):
            current.cpu_s = float(m.group(1))
        elif m := RE_CDCL_CONFLICTS.match(line):
            current.conflicts = int(m.group(1))
        elif m := RE_CDCL_DECISIONS.match(line):
            current.decisions = int(m.group(1))
        elif m := RE_CDCL_PROPAGATIONS.match(line):
            current.propagations = int(m.group(1))
        elif m := RE_CDCL_CONFLICT_LITS.match(line):
            current.conflict_literals = int(m.group(1))
        elif m := RE_CDCL_LAM.match(line):
            current.lam_cores = int(m.group(1))
        elif m := RE_CDCL_SLA.match(line):
            current.sla_s = float(m.group(1))

    return data


def _fmt(v: float | int | None) -> str:
    if v is None:
        return "n/a"
    if isinstance(v, float):
        return f"{v:g}"
    return str(v)


def format_instance_line(inst: Instance) -> str:
    return (
        f"{inst.name} "
        f"cpu: {_fmt(inst.cpu_s)} "
        f"conf: {_fmt(inst.conflicts)} "
        f"dec: {_fmt(inst.decisions)} "
        f"prop: {_fmt(inst.propagations)} "
        f"cflit: {_fmt(inst.conflict_literals)} "
        f"lam: {_fmt(inst.lam_cores)} "
        f"sla: {_fmt(inst.sla_s)}"
    )


def read_description(preset: str | None) -> str:
    if preset is not None:
        return preset.strip()
    if not sys.stdin.isatty():
        return sys.stdin.read().strip()

    print(
        "Enter a description for this run (finish with an empty line, "
        "or Ctrl-Z + Enter on Windows / Ctrl-D on Unix):",
        file=sys.stderr,
    )
    lines: list[str] = []
    try:
        while True:
            line = input()
            if line == "" and lines:
                break
            lines.append(line)
    except EOFError:
        pass
    return "\n".join(lines).strip()


def render_markdown(data: LogData, description: str) -> str:
    out: list[str] = []
    out.append(f"# Log report: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    out.append("")
    out.append(description if description else "_(no description provided)_")
    out.append("")

    out.append("## Results")
    out.append("")
    if not data.instances:
        out.append("_No instances were found in the log._")
        return "\n".join(out) + "\n"

    out.append("```")
    for inst in data.instances:
        out.append(format_instance_line(inst))
    out.append("```")
    out.append("")

    return "\n".join(out) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Analyze a single maxmodels .log file and produce a Markdown report."
    )
    ap.add_argument("log", type=Path, help="Path to the .log file to analyze")
    ap.add_argument(
        "-d",
        "--description",
        type=str,
        default=None,
        help="Description text (skips the interactive prompt). Pass '-' to read from stdin.",
    )
    ap.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="Write the Markdown report to this file (default: stdout)",
    )
    args = ap.parse_args()

    log_path: Path = args.log
    if not log_path.is_file():
        print(f"error: not a file: {log_path}", file=sys.stderr)
        return 2

    data = parse_log(log_path)

    preset: str | None
    if args.description == "-":
        preset = sys.stdin.read()
    else:
        preset = args.description
    description = read_description(preset)

    markdown = render_markdown(data, description)

    if args.output is not None:
        args.output.write_text(markdown, encoding="utf-8")
        print(f"Wrote {args.output}", file=sys.stderr)
    else:
        sys.stdout.write(markdown)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
