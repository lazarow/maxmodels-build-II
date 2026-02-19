for i in $(seq -w 1 15); do
    file="p${i}.lp"
    if [ -f "$file" ]; then
        echo "Processing $file"
        start_time=$(date +%s.%N)
        cat "$file" | gringo --output=smodels --warn=none | smodels -internal -nolookahead | lp2normal-2.27 | maxmodels --simplify | lp2lp2-1.23 | maxmodels --solve --debug-cdcl --cost-conflict-encoding
        end_time=$(date +%s.%N)
        elapsed=$(echo "$end_time - $start_time" | bc)
        echo "Time taken: $elapsed seconds"
    fi
done