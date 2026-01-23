#pragma once

#include <unordered_map>
#include <string>
#include "internal_representation.hpp"
#include "stable_model.hpp"

using namespace std;

class AtomMapper
{
    unsigned int current_variable = 1;
    unordered_map<Atom, unsigned int> atom_to_variable;

public:
    unsigned int get_next_variable();
    unsigned int get_variable(Literal literal);
};

enum SolvingStrategy
{
    BASELINE = 0,
    ALL_RULES = 1,
    NON_AUXILIARY_RULES = 2,
    SELECTIVE = 3
};

enum LoopFormulasStrategy
{
    ALL = 0
};

struct SolvingConfiguration
{
    SolvingStrategy solving_strategy = SolvingStrategy::ALL_RULES;
    LoopFormulasStrategy loop_formulas_strategy = LoopFormulasStrategy::ALL;
    string external_solver_path = "";
};

void solve(const Program &program, const SolvingConfiguration &solving_configuration);