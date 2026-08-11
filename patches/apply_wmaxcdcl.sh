#!/usr/bin/env bash
# Apply WMaxCDCL2024.patch with CRLF/LF normalized for Windows+WSL checkouts.
# Usage: apply_wmaxcdcl.sh <WMaxCDCL2024_dir> <patch_file>
set -euo pipefail

TARGET_DIR="${1:?target dir}"
PATCH_FILE="${2:?patch file}"

python3 - "$TARGET_DIR" "$PATCH_FILE" <<'PY'
import pathlib, sys, tempfile, subprocess, os

target = pathlib.Path(sys.argv[1])
patch_file = pathlib.Path(sys.argv[2])
files = [
    "code/utils/ParseUtils.h",
    "code/core/Dimacs.h",
    "code/core/Solver.h",
    "code/core/Solver.cc",
]

def to_lf(data: bytes) -> bytes:
    return data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")

for rel in files:
    path = target / rel
    path.write_bytes(to_lf(path.read_bytes()))

lf_patch = to_lf(patch_file.read_bytes())
with tempfile.NamedTemporaryFile(prefix="WMaxCDCL2024.", suffix=".patch", delete=False) as tmp:
    tmp.write(lf_patch)
    tmp_path = tmp.name

try:
    with open(tmp_path, "rb") as fh:
        proc = subprocess.run(
            ["patch", "-p1", "--binary"],
            cwd=target,
            stdin=fh,
            check=False,
        )
    if proc.returncode != 0:
        sys.exit(proc.returncode)
finally:
    os.unlink(tmp_path)
PY
