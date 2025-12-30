#include <unordered_map>
#include <vector>
#include <stack>

#include "loop_formulas.hpp"

struct TarjanSCC
{
    unordered_map<Atom, int> index, lowlink;
    unordered_set<Atom> on_stack;
    stack<Atom> S;
    int current_index = 0;
    vector<Model> SCCs;
};

void strongconnect(const Atom &atom, const unordered_map<Atom, vector<Atom>> &graph, TarjanSCC &t)
{
    t.index[atom] = t.lowlink[atom] = t.current_index++;
    t.S.push(atom);
    t.on_stack.insert(atom);
    auto it = graph.find(atom);
    if (it != graph.end())
    {
        for (Atom w : it->second)
        {
            if (!t.index.contains(w))
            {
                strongconnect(w, graph, t);
                t.lowlink[atom] = min(t.lowlink[atom], t.lowlink[w]);
            }
            else if (t.on_stack.contains(w))
            {
                t.lowlink[atom] = min(t.lowlink[atom], t.index[w]);
            }
        }
    }
    if (t.lowlink[atom] == t.index[atom])
    {
        Model scc;
        Atom w;
        do
        {
            w = t.S.top();
            t.S.pop();
            t.on_stack.erase(w);
            scc.insert(w);
        } while (w != atom);
        t.SCCs.push_back(move(scc));
    }
}

vector<Model> compute_maximal_loop_formulas(const Program &program, const Model &M_minus)
{
    vector<Model> loops;

    // #region Building a positive dependency graph based on M^-.
    unordered_map<Atom, vector<Atom>> G_minus;
    for (Atom head : M_minus)
    {
        for (BodyIndex body_index : program.heads.at(head))
        {
            const Body &body = program.bodies[body_index];
            if (body[0] < 0)
                continue;
            for (unsigned int literal_index = 1; literal_index < body.size(); literal_index++)
            {
                Literal literal = body[literal_index];
                if (literal > 0 && M_minus.contains(literal))
                {
                    G_minus[head].push_back(literal);
                }
            }
        }
    }
    // #endregion

    // #region Computing SCCs from the positive dependency graph.
    TarjanSCC t;
    for (Atom head : M_minus)
    {
        if (t.index.contains(head) == false)
        {
            strongconnect(head, G_minus, t);
        }
    }
    for (const auto &SCC : t.SCCs)
    {
        if (SCC.size() > 1)
        {
            loops.push_back(move(SCC));
        }
        else
        {
            Atom p = *SCC.begin();
            for (Atom q : G_minus[p])
            {
                if (q == p)
                {
                    loops.push_back(move(SCC));
                    break;
                }
            }
        }
    }
    // #endregion

    return loops;
}