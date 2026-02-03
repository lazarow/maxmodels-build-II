gringo --output=smodels --warn=none $1 | smodels -internal -nolookahead | lp2normal-2.27 | maxmodels --simplify | lp2lp2-1.23 | maxmodels --solve --use-metrics --metric-weights=$2
