#!/bin/sh

expected='1 1 0 0
0
0
B+
0
B-
1
0
1'

find . -type f -name '*.lp' -print0 |
while IFS= read -r file; do
    result=$(your_command "$file")

    if [ "$result" = "$expected" ]; then
        printf '%s\n' "$file"
    fi
done