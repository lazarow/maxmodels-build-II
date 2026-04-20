#!/bin/bash

FOLDER="$1"

if [[ -z "$FOLDER" ]]; then
    echo "Usage: $0 <folder> <output_dir>"
    exit 1
fi

SUFFIXES=("")

for suffix in "${SUFFIXES[@]}"; do
    CMD="maxmodels${suffix}.sh"
    OUT_FILE="$FOLDER/maxmodels${suffix}.log"
    : > "$OUT_FILE"
    for file in "$FOLDER"/*.lp; do
        [ -e "$file" ] || continue
        base=$(basename "$file" .lp)
        echo "Running $CMD on $file → $OUT_FILE"
        echo "Processing $base" >> "$OUT_FILE"
        cat "$file" | $CMD >> "$OUT_FILE" 2>&1
    done
done
