#include <unordered_map>
#include "internal_representation.hpp"
#include "stable_model.hpp"

class AtomMapper
{
    unsigned int current_variable = 1;
    unordered_map<Atom, unsigned int> atom_to_variable;
    unordered_map<unsigned int, Atom> variable_to_atom;

public:
    unsigned int get_next_variable();
    unsigned int get_variable(Literal literal);
    bool has_atom(Atom atom) const;
    Atom get_atom(unsigned int variable) const;
};

void solve(Program &program);