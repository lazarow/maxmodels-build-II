for i in $(seq -w 1 30); do
    file="p${i}.lp"
    if [ -f "$file" ]; then
        cat "$file" | gringo --output=smodels --warn=none | smodels -internal -nolookahead | lp2normal-2.27 | maxmodels --simplify | lp2lp2-1.23 | maxmodels --solve --debug-cdcl --cost-conflict-encoding
    fi
done