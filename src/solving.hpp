#include <unordered_map>
#include "internal_representation.hpp"
#include "stable_model.hpp"

class AtomMapper
{
    unsigned int current_variable = 1;
    unordered_map<Atom, unsigned int> atom_to_variable;

public:
    unsigned int get_next_variable();
    unsigned int get_variable(Literal literal);
};

void solve(Program &program);