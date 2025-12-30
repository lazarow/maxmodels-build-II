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
    NON_AUXILIARY_RULES = 2
};

enum LoopFormulasStrategy
{
    MAXIMAL = 0,
    MINIMAL_FIRST = 1,
    MINIMAL_SMALLEST = 2
};

struct SolvingConfiguration
{
    SolvingStrategy solving_strategy = NON_AUXILIARY_RULES;
    LoopFormulasStrategy loop_formulas_strategy = MAXIMAL;
};

void solve(Program &program, SolvingConfiguration &solving_configuration);