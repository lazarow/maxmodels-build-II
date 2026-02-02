gringo --output=smodels --warn=none $1 $2 | smodels -internal -nolookahead | lp2normal-2.27 | maxmodels --simplify --solve --benchmark > /dev/null
gringo --output=smodels --warn=none $1 $2 | smodels -internal -nolookahead | lp2normal-2.27 | maxmodels --simplify --solve --use-metrics --benchmark --metric-weights=-0.5642520834612947,1.6380235436161399,1.8651010199260154,-0.2172850115662182,1.1220822910292234,1.3702471969479282,-1.1662504189398557,-0.0888535516875334,-1.015581188458127,-1.4734170153933979,-1.6508142609399779 > /dev/null
gringo --output=smodels --warn=none $1 $2 | smodels -internal -nolookahead | lp2normal-2.27 | maxmodels --simplify | lp2lp2-1.23 | maxmodels --solve --benchmark > /dev/null
gringo --output=smodels --warn=none $1 $2 | smodels -internal -nolookahead | lp2normal-2.27 | maxmodels --simplify | lp2lp2-1.23 | maxmodels --solve --use-metrics --benchmark --metric-weights=-0.5642520834612947,1.6380235436161399,1.8651010199260154,-0.2172850115662182,1.1220822910292234,1.3702471969479282,-1.1662504189398557,-0.0888535516875334,-1.015581188458127,-1.4734170153933979,-1.6508142609399779 > /dev/null
echo ""




