#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>
#include <chrono>
#include <random>
#include <numeric>
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

    // If the program has weights, we need to encode cost conflict encoding if it is not already enabled.
    solving_configuration.cost_conflict_encoding = solving_configuration.cost_conflict_encoding && program.weights.size() > 0;

    for (const auto &body_index : program.constraints)
    {
        const auto &body = program.bodies[body_index];
        if (body[0] < 0)
            throw logic_error("A constraint should be removed during the simplification.");
        else if (body[0] == 0)
            throw logic_error("A constraint is satisfied.");

        unsigned int body_size = body.size();
        bool all_weighted = true;
        vector<Literal> constraint_cost_conflict_literals;
        for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        {
            // Skip if a literal is determined.
            if (body[literal_index] != 0)
            {
                if (all_weighted && program.weights.contains(-body[literal_index]))
                    constraint_cost_conflict_literals.push_back(body[literal_index]);
                else
                    all_weighted = false;
                wcnf->add_hard(-atom_mapper.get_variable(body[literal_index]));
            }
        }
        wcnf->add_hard(0);

        if (solving_configuration.cost_conflict_encoding && all_weighted && constraint_cost_conflict_literals.size() > 0)
        {
            vector<Literal> cost_conflict_clause;
            for (const auto &literal : constraint_cost_conflict_literals)
            {
                if (all_cost_conflict_literals.contains(-literal) == false)
                {
                    Atom atom = literal < 0 ? -literal : literal;
                    if (program.heads.contains(atom) && program.required_atoms.contains(atom) == false)
                        all_cost_conflict_literals.insert(-literal);
                    else
                    {
                        cost_conflict_clause.clear();
                        break;
                    }
                }
                cost_conflict_clause.push_back(literal);
            }
            if (cost_conflict_clause.size() > 0)
                all_cost_conflict_clauses.push_back(cost_conflict_clause);
        }
    }
    // #endregion

    // #region Soft Clauses
    if (program.weights.size() > 0)
    {
        vector<vector<Literal>> cost_conflict_clauses;
        unordered_set<Literal> cost_conflict_literals;

        // #region Overlap method
        if (all_cost_conflict_clauses.empty() == false)
        {
            unsigned int total_overlap = 0;
            unsigned int max_overlap = 0;
            unsigned int min_overlap = numeric_limits<unsigned int>::max();
            unsigned int size_of_clauses = all_cost_conflict_clauses.size();
            vector<unsigned int> overlap_values;
            overlap_values.reserve(size_of_clauses);
            for (unsigned int i = 0; i < size_of_clauses; i++)
            {
                const auto &clause_i = all_cost_conflict_clauses[i];
                unordered_set<Literal> literals_i(clause_i.begin(), clause_i.end());
                unsigned int overlap = 0;
                for (unsigned int j = 0; j < size_of_clauses; j++)
                {
                    if (i == j)
                        continue;
                    const auto &clause_j = all_cost_conflict_clauses[j];
                    unordered_set<Literal> literals_j(clause_j.begin(), clause_j.end());
                    unsigned int overlap_j = 0;
                    for (const auto &literal : literals_i)
                    {
                        if (literals_j.contains(literal))
                            overlap_j++;
                    }
                    overlap += overlap_j > 0 ? 1 : 0;
                }
                // cout << "Overlap for clause " << i << ": " << overlap << endl;
                total_overlap += overlap;
                max_overlap = max(max_overlap, overlap);
                min_overlap = min(min_overlap, overlap);
                overlap_values.push_back(overlap);
            }
            double avg_overlap = total_overlap / all_cost_conflict_clauses.size();
            double stddev_overlap = 0.0;
            for (const auto &overlap : overlap_values)
            {
                stddev_overlap += (overlap - avg_overlap) * (overlap - avg_overlap);
            }
            stddev_overlap = sqrt(stddev_overlap / overlap_values.size());
            cout << "% Overlap method: max: " << max_overlap << ", min: " << min_overlap << ", avg: " << avg_overlap << ", sd: " << stddev_overlap << endl;
            for (unsigned int i = 0; i < size_of_clauses; i++)
            {
                if (overlap_values[i] > avg_overlap)
                {
                    cost_conflict_clauses.push_back(all_cost_conflict_clauses[i]);
                    for (const auto &literal : all_cost_conflict_clauses[i])
                    {
                        cost_conflict_literals.insert(-literal);
                    }
                }
            }
        }
        // #endregion

        unordered_map<Literal, unsigned int> relaxation_var;
        unsigned int nof_cost_conflict_clauses = 0;
        unsigned int nof_cost_conflict_literals = 0;
        for (const auto &[literal, weight] : program.weights)
        {
            Atom atom = literal < 0 ? -literal : literal;
            if (
                program.heads.contains(atom) && program.required_atoms.contains(atom) == false)
            {
                if (solving_configuration.cost_conflict_encoding && cost_conflict_literals.contains(literal))
                {
                    unsigned int atom_var = atom_mapper.get_variable(literal);
                    unsigned int r = atom_mapper.get_next_variable();
                    relaxation_var[literal] = r;
                    // (¬atom ∨ r) ∧ (atom ∨ ¬r)
                    wcnf->add_hard(-atom_var);
                    wcnf->add_hard(r);
                    wcnf->add_hard(0);
                    wcnf->add_hard(atom_var);
                    wcnf->add_hard(-r);
                    wcnf->add_hard(0);
                    wcnf->add_soft(-r, weight);
                    nof_cost_conflict_literals++;
                }
                else
                {
                    wcnf->add_soft(-atom_mapper.get_variable(literal), weight);
                }
            }
        }
        if (solving_configuration.cost_conflict_encoding)
        {
            for (const auto &cost_conflict_clause : cost_conflict_clauses)
            {
                for (const auto &literal : cost_conflict_clause)
                {
                    wcnf->add_hard(relaxation_var[-literal]);
                }
                wcnf->add_hard(0);
                nof_cost_conflict_clauses++;
            }
            if (cost_conflict_clauses.empty() == false)
            {
                cout << "% The number of cost conflict literals: " << nof_cost_conflict_literals << endl;
                cout << "% The number of cost conflict clauses: " << nof_cost_conflict_clauses << endl;
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

unsigned int AtomMapper::get_variable(Literal literal)
{
    Atom atom = literal < 0 ? -literal : literal;
    if (atom_to_variable.contains(atom) == false)
    {
        atom_to_variable[atom] = get_next_variable();
    }
    return literal < 0 ? -atom_to_variable.at(atom) : atom_to_variable.at(atom);
}
