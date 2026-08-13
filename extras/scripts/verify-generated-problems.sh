#!/usr/bin/env bash

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
sort -z |
while IFS= read -r -d '' file; do
    echo -n "$file ... "
    result=$(cat "$file" | gringo --output=smodels --warn=none| smodels -internal -nolookahead)
    if [ "$result" = "$expected" ]; then
        echo "CORRUPTED"
    else
	echo "OK"
    fi
done