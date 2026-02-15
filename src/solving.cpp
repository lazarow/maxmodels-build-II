#include <cmath>
#include <memory>
#include <utility>
#include <chrono>
#include "solving.hpp"
#include "stable_model.hpp"
#include "loop_formulas.hpp"
#include "wcnf.hpp"
#include "encoding.hpp"

using namespace std;
using namespace chrono;

void solve(const Program &program, const SolvingConfiguration &solving_configuration, SolvingBenchmark &benchmark)
{
    auto start_time = high_resolution_clock::now();

    unique_ptr<WCNF> wcnf = make_unique<ExternalSolverWrapperWCNF>();
    wcnf->init();

    AtomMapper atom_mapper;
    unordered_map<BodyIndex, unsigned int> body_to_variable;

    vector<vector<Literal>> cost_conflict_clauses;
    unordered_set<Literal> cost_conflict_literals;

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
        bool all_weighted = true;
        Weight initial_weight = -1;
        Weight weight = initial_weight;
        vector<Literal> constraint_cost_conflict_literals;
        for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        {
            // Skip if a literal is determined.
            if (body[literal_index] != 0)
            {
                if (all_weighted && program.weights.contains(-body[literal_index]) && (weight == initial_weight || weight == program.weights.at(-body[literal_index])))
                {
                    if (weight == initial_weight)
                        weight = program.weights.at(-body[literal_index]);
                    constraint_cost_conflict_literals.push_back(body[literal_index]);
                }
                else
                {
                    all_weighted = false;
                }
                wcnf->add_hard(-atom_mapper.get_variable(body[literal_index]));
            }
        }
        wcnf->add_hard(0);

        if (solving_configuration.cost_conflict_encoding && all_weighted && constraint_cost_conflict_literals.size() > 0)
        {
            vector<Literal> cost_conflict_clause;
            for (const auto &literal : constraint_cost_conflict_literals)
            {
                cost_conflict_literals.insert(-literal);
                cost_conflict_clause.push_back(literal);
            }
            cost_conflict_clauses.push_back(cost_conflict_clause);
        }
    }
    // #endregion

    // #region Soft Clauses
    if (program.weights.size() > 0)
    {
        unordered_map<Literal, unsigned int> relaxation_var;
        unsigned int nof_soft_clauses = 0;
        for (const auto &[literal, weight] : program.weights)
        {
            Atom atom = literal < 0 ? -literal : literal;
            if (
                program.heads.contains(atom) && program.required_atoms.contains(atom) == false)
            {
                if (cost_conflict_literals.contains(literal))
                {
                    unsigned int atom_var = atom_mapper.get_variable(literal);
                    unsigned int r = atom_mapper.get_next_variable();
                    relaxation_var[literal] = r;
                    wcnf->add_hard(-atom_var);
                    wcnf->add_hard(r);
                    wcnf->add_hard(0);
                    wcnf->add_soft(-r, weight);
                }
                else
                {
                    wcnf->add_soft(-atom_mapper.get_variable(literal), weight);
                }
                nof_soft_clauses++;
            }
        }
        for (const auto &cost_conflict_clause : cost_conflict_clauses)
        {
            bool all_relaxed = true;
            for (const auto &literal : cost_conflict_clause)
            {
                if (relaxation_var.contains(literal) == false)
                {
                    all_relaxed = false;
                    break;
                }
            }
            if (all_relaxed)
            {
                for (const auto &literal : cost_conflict_clause)
                {
                    wcnf->add_hard(-relaxation_var[literal]);
                }
                wcnf->add_hard(0);
            }
        }
        if (cost_conflict_clauses.empty() == false)
        {
            cout << "% The number of cost conflict clauses: " << cost_conflict_clauses.size() << endl;
            cout << "% The number of soft clauses: " << nof_soft_clauses << endl;
            cout << "% The ratio of cost conflict clauses to soft clauses: " << static_cast<double>(cost_conflict_clauses.size()) / nof_soft_clauses << endl;
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