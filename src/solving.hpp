#pragma once

#include <unordered_map>
#include <string>

#include "internal_representation.hpp"

using namespace std;

struct SolvingConfiguration
{
    string external_solver_path = "";
    bool use_metrics = false;
    int max_metrics_weight = 9;
    vector<double> metric_weights = {1.5, 1.5, 1.5, -1.5, -1.5, -2.0, -1.5, -1.5, 1.5, 1.5, 1.5};
};

struct SolvingBenchmark
{
    int nof_iterations = 0;
    double time = 0.0;
};

class AtomMapper
{
    unsigned int current_variable = 1;
    unordered_map<Atom, unsigned int> atom_to_variable;

public:
    unsigned int get_next_variable();
    unsigned int get_variable(Literal literal);
};

void solve(const Program &program, const SolvingConfiguration &solving_configuration, SolvingBenchmark &benchmark);