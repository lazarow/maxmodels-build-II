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

enum SolvingStrategy
{
    BASELINE = 0,
    ALL_RULES = 1,
    NON_AUXILIARY_RULES = 2,
    SELECTIVE = 3
};

enum LoopFormulasStrategy
{
    ALL = 0,
    FIRST_ONLY = 1,
};

struct SolvingConfiguration
{
    SolvingStrategy solving_strategy = NON_AUXILIARY_RULES;
    LoopFormulasStrategy loop_formulas_strategy = ALL;
};

void solve(Program &program, SolvingConfiguration &solving_configuration);