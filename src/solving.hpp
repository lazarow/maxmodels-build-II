#pragma once

#include <unordered_map>
#include <string>

#include "internal_representation.hpp"

using namespace std;

struct SolvingConfiguration
{
    string external_solver_path = "";
};

class AtomMapper
{
    unsigned int current_variable = 1;
    unordered_map<Atom, unsigned int> atom_to_variable;

public:
    unsigned int get_next_variable();
    unsigned int get_variable(Literal literal);
};

void solve(const Program &program, const SolvingConfiguration &solving_configuration);