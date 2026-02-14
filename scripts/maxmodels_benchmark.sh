gringo --output=smodels --warn=none $1 encoding.lp | smodels -internal -nolookahead | lp2normal-2.27 | maxmodels --simplify | lp2lp2-1.23 | maxmodels --solve --benchmark > /dev/null
gringo --output=smodels --warn=none $1 encoding.lp | smodels -internal -nolookahead | lp2normal-2.27 | maxmodels --simplify | lp2lp2-1.23 | maxmodels --solve --use-metrics --benchmark --metric-weights=0.952,0.793,0,0,-0.352,0,0,0.058  > /dev/null
echo ""


