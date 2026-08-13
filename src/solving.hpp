#pragma once

#include <unordered_map>
#include <string>

#include "internal_representation.hpp"

using namespace std;

struct SolvingConfiguration
{
    string external_solver_path = "";
    bool debug_cdcl = false;
    int mode = 0;
    double default_initial_activity = 0.00001;
    int timeout_seconds = 0;
};

struct SolvingBenchmark
{
    int nof_iterations = 0;
    double time = 0.0;
};

class AtomMapper
{
    unordered_map<Atom, unsigned int> atom_to_variable;

public:
    unsigned int current_variable = 1;
    unsigned int get_next_variable();
    int get_variable(Literal literal);
};

void solve(const Program &program, SolvingConfiguration &solving_configuration, SolvingBenchmark &benchmark);