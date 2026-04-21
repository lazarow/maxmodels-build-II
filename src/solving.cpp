#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>
#include <chrono>
#include <random>
#include <numeric>
#include <limits>
#include "solving.hpp"
#include "stable_model.hpp"
#include "loop_formulas.hpp"
#include "wcnf.hpp"
#include "encoding.hpp"

using namespace std;
using namespace chrono;

void solve(const Program &program, SolvingConfiguration &solving_configuration, SolvingBenchmark &benchmark)
{
    auto start_time = high_resolution_clock::now();

    unique_ptr<WCNF> wcnf = make_unique<ExternalSolverWrapperWCNF>();
    wcnf->init();

    AtomMapper atom_mapper;
    unordered_map<BodyIndex, unsigned int> body_to_variable;

    // Cost Clauses Encoding
    vector<vector<Literal>> all_cost_conflict_clauses;
    unordered_set<Literal> all_cost_conflict_literals;

    // #region Encoding
    if (program.extended_atoms.empty() == false)
        lp2sat_like(program, wcnf, body_to_variable, atom_mapper);
    else
        clark_completion(program, wcnf, body_to_variable, atom_mapper);
    // #endregion

    // #region Constraints
    for (const auto &body_index : program.constraints)
    {
        const auto &body = program.bodies[body_index];
        if (body[0] < 0)
            throw logic_error("A constraint should be removed during the simplification.");
        else if (body[0] == 0)
            throw logic_error("A constraint is satisfied.");

        unsigned int body_size = body.size();
        for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        {
            // Skip if a literal is determined.
            if (body[literal_index] != 0)
            {
                wcnf->add_hard(-atom_mapper.get_variable(body[literal_index]));
            }
        }
        wcnf->add_hard(0);
    }
    // #endregion

    // #region Soft Clauses
    if (program.weights.size() > 0)
    {
        for (const auto &[literal, weight] : program.weights)
        {
            Atom atom = literal < 0 ? -literal : literal;
            if (
                program.heads.contains(atom) && program.required_atoms.contains(atom) == false)
            {
                wcnf->add_soft(-atom_mapper.get_variable(literal), weight);
                wcnf->add_initial_activity(atom_mapper.get_variable(literal), 0.0);
            }
        }
    }
    // #endregion

    // #region Step 4: Solving
    unsigned int nof_iterations = 0;
    while (true)
    {
        int32_t result = wcnf->solve(solving_configuration);
        nof_iterations++;
        if (result == 10)
        {
            benchmark.time = duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count() / 1000.0;
            throw unsatisfied_exception("The program is unsatisfied.");
        }
        else if (result == 20 || result == 30)
        {
            Model supporting_model;
            for (const auto &[head, body_indices] : program.heads)
            {
                if (
                    program.required_atoms.contains(head) || wcnf->val_lit(atom_mapper.get_variable(head)) > 0)
                {
                    supporting_model.insert(head);
                }
            }

            /**
             * When using level ranking (extended_atoms), the solver returns a stable model
             * in a single call, so we skip loop formula checking.
             */
            if (program.extended_atoms.empty() == false)
            {
                benchmark.time = duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count() / 1000.0;
                throw satisfied_exception(program, supporting_model);
            }

            Model consequences = compute_consequences(program, supporting_model);
            Model M_minus;
            for (Atom atom : supporting_model)
                if (consequences.contains(atom) == false)
                    M_minus.insert(atom);
            if (M_minus.empty())
            {
                benchmark.time = duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count() / 1000.0;
                throw satisfied_exception(program, supporting_model);
            }
            else
            {
                vector<Model> loop_formulas;
                loop_formulas = compute_maximal_loop_formulas(program, M_minus);
                for (const auto &loop_formula : loop_formulas)
                {
                    for (Atom atom : loop_formula)
                    {
                        for (BodyIndex body_index : program.heads.at(atom))
                        {
                            const Body &body = program.bodies[body_index];
                            if (body[0] < 0)
                                continue;
                            bool touches_loop = false;
                            for (unsigned int literal_index = 1; literal_index < body.size(); literal_index++)
                            {
                                if (loop_formula.contains(abs(body[literal_index])))
                                {
                                    touches_loop = true;
                                    break;
                                }
                            }
                            if (touches_loop == false)
                            {
                                wcnf->add_hard(body_to_variable[body_index]);
                            }
                        }
                        wcnf->add_hard(-atom_mapper.get_variable(atom));
                    }
                    wcnf->add_hard(0);
                }
                cout << "% Iteration " << nof_iterations << " completed. " << loop_formulas.size() << " loops found." << endl;
                benchmark.nof_iterations++;
            }
        }
        else
            throw logic_error("Unknown result of the solver.");
    }
    // #endregion
    wcnf->clear();
}

unsigned int AtomMapper::get_next_variable()
{
    return current_variable++;
}

int AtomMapper::get_variable(Literal literal)
{
    Atom atom = literal < 0 ? -literal : literal;
    if (atom_to_variable.contains(atom) == false)
    {
        atom_to_variable[atom] = get_next_variable();
    }
    return literal < 0 ? -atom_to_variable.at(atom) : atom_to_variable.at(atom);
}
