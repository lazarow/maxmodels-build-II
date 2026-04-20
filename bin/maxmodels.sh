#!/bin/bash

# Check if all required commands are available
REQUIRED_COMMANDS=("gringo" "smodels" "lp2normal-2.27" "maxmodels" "lp2lp2-1.23")
for cmd in "${REQUIRED_COMMANDS[@]}"; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "Error: Required command '$cmd' not found in PATH." >&2
        exit 1
    fi
done

gringo --output=smodels --warn=none | smodels -internal -nolookahead | lp2normal-2.27 | maxmodels --simplify | lp2lp2-1.23 | maxmodels --solve --debug-cdcl --external-solver=/home/an/maxsat-solvers/WMaxCDCL2024/code/simp/wmaxcdcl_static --use-initial-activities