#!/bin/bash

FOLDER="$1"

if [[ -z "$FOLDER" ]]; then
    echo "Usage: $0 <folder>"
    exit 1
fi

SUFFIXES=("_default")

for suffix in "${SUFFIXES[@]}"; do
    CMD="bin/preconfigured-runners/maxmodels${suffix}.sh"
    OUT_FILE="$FOLDER/maxmodels${suffix}.log"
    : > "$OUT_FILE"
    for file in "$FOLDER"/*.lp; do
        [ -e "$file" ] || continue
        base=$(basename "$file" .lp)
        echo "Running $CMD on $file → $OUT_FILE"
        echo "Processing $base" >> "$OUT_FILE"
        # Run the solver and filter out 'ANSWER' and the next line from the output
        $CMD < "$file" 2>&1 | awk '
            /^ANSWER$/ { skip=1; next }
            skip { skip=0; next }
            { print }
        ' >> "$OUT_FILE"
 
    done
done
