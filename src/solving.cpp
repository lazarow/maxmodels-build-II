#include <cstdint>
#include <ipamir.h>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <vector>
#include "solving.hpp"
#include "stable_model.hpp"
#include "loop_formulas.hpp"

void solve(Program &program, SolvingConfiguration &solving_configuration)
{
    void *solver = ipamir_init();

    AtomMapper atom_mapper;
    unordered_map<BodyIndex, unsigned int> body_to_variable;

    // #region Step 1: Encoding normal rules.
    unsigned int nof_first_level_soft_clauses = 0;
    /**
     * Consider:
     * a <- b, not c.
     * a <- not d.
     * a <- e.
     * Completion:
     * (a ⇔ r1 ∨ r2 ∨ r3) ∧ (r1 ⇔ b ∧ ¬c) ∧ (r2 ⇔ ¬d) ∧ (r3 ⇔ e)
     * CNF:
     * (r1 ∨ r2 ∨ r3 ∨ ¬a) ∧ (¬r1 ∨ a) ∧ (¬r2 ∨ a) ∧ (¬r3 ∨ a) ∧
     * (r1 ∨ ¬b ∨ c) ∧ (b ∨ ¬r1) ∧ (¬c ∨ ¬r1) ∧
     * (r2 ∨ d) ∧ (¬d ∨ ¬r2) ∧
     * (r3 ∨ ¬e) ∧ (e ∨ ¬r3)
     * where r1, r2 and r3 are additional variables denoted rules' bodies.
     */
    for (const auto &[head, body_indices] : program.heads)
    {
        // List of bodies for a head (the Tseitin transformation).
        queue<unsigned int> body_variables;
        for (const auto &body_index : body_indices)
        {
            const auto &body = program.bodies[body_index];
            // Skip if a body is falsified.
            if (body[0] < 0)
                continue;
            else if (body[0] == 0)
                // Justifying a fact during the simplification should remove a head and its related bodies.
                throw logic_error("An encoded rule cannot be a fact.");

            // Create a new variable for a body.
            unsigned int body_variable = atom_mapper.get_next_variable();
            body_variables.push(body_variable);
            body_to_variable[body_index] = body_variable;

            // (¬r1 ∨ a)
            // Skip if `a` is known to be true, i.e., (¬r1 ∨ true) = true.
            if (program.required_atoms.contains(head) == false)
            {
                ipamir_add_hard(solver, -body_variable);
                ipamir_add_hard(solver, atom_mapper.get_variable(head));
                ipamir_add_hard(solver, 0);
            }

            // (b ∨ ¬r1) ∧ (¬c ∨ ¬r1)
            unsigned int body_size = body.size();
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                if (body[literal_index] != 0) // Skip if a literal is determined.
                {
                    ipamir_add_hard(solver, atom_mapper.get_variable(body[literal_index]));
                    ipamir_add_hard(solver, -body_variable);
                    ipamir_add_hard(solver, 0);
                }
            }
            // (r1 ∨ ¬b ∨ c)
            ipamir_add_hard(solver, body_variable);
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                if (body[literal_index] != 0) // Skip if a literal is determined.
                {
                    ipamir_add_hard(solver, -atom_mapper.get_variable(body[literal_index]));
                }
            }
            ipamir_add_hard(solver, 0);

            /**
             * The First Level of Weights (looking for a stable model).
             * The following atoms are ignored:
             * - required atoms (as they must be in a stable model already),
             * - auxilary atoms (e.g. created during normalization of extended rules).
             */
            if (solving_configuration.add_body_weights &&
                program.symbols.contains(head) &&
                program.required_atoms.contains(head) == false)
            {
                ipamir_add_soft_lit(solver, -body_variable, 1);
                nof_first_level_soft_clauses++;
            }
        }
        if (body_variables.empty())
            // A head must have at least one active rule.
            throw logic_error("An encoded head has no body.");

        if (program.required_atoms.contains(head))
        {
            /**
             * If `a` is true then:
             * (r1 ∨ r2 ∨ r3 ∨ ¬a) = (r1 ∨ r2 ∨ r3 ∨ false) = (r1 ∨ r2 ∨ r3)
             */
            while (body_variables.empty() == false)
            {
                ipamir_add_hard(solver, body_variables.front());
                body_variables.pop();
            }
            ipamir_add_hard(solver, 0);
        }
        else
        {
            // (r1 ∨ r2 ∨ r3 ∨ ¬a)
            while (body_variables.empty() == false)
            {
                ipamir_add_hard(solver, body_variables.front());
                body_variables.pop();
            }
            ipamir_add_hard(solver, -atom_mapper.get_variable(head));
            ipamir_add_hard(solver, 0);
        }
    }
    // #endregion

    // #region Step 2: Encoding Constraints.
    for (const auto &body_index : program.constraints)
    {
        const auto &body = program.bodies[body_index];
        if (body[0] < 0)
            continue;
        else if (body[0] == 0)
            throw logic_error("An encoded constraint cannot be satisfied.");

        unsigned int body_size = body.size();
        for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        {
            if (body[literal_index] != 0) // Skip if a literal is determined.
            {
                ipamir_add_hard(solver, -atom_mapper.get_variable(body[literal_index]));
            }
        }
        ipamir_add_hard(solver, 0);
    }
    // #endregion

    // #region Step 3: The Second Level of Weights (optimization).
    if (program.weights.size() > 0)
    {
        unsigned int augmenting = nof_first_level_soft_clauses == 0
                                      ? 1
                                      : pow(10, ceil(log10(nof_first_level_soft_clauses)));
        for (const auto &[literal, weight] : program.weights)
        {
            Atom atom = literal < 0 ? -literal : literal;
            if (program.heads.contains(atom))
            {
                ipamir_add_soft_lit(solver, atom_mapper.get_variable(literal), weight * augmenting);
            }
        }
    }
    // #endregion

    // #region Step 4: Solving the problem by means of iMaxHS.
    unsigned int nof_iterations = 0;
    while (true)
    {
        int32_t result = ipamir_solve(solver);
        nof_iterations++;
        if (result == 10)
            throw unsatisfied_exception("The program is unsatisfied.");
        else if (result == 20 || result == 30)
        {
            Model supporting_model;
            for (const auto &[head, body_indices] : program.heads)
            {
                if (ipamir_val_lit(solver, atom_mapper.get_variable(head)) > 0)
                    supporting_model.insert(head);
            }

            /**
             * To enable ipamir_print_wcnf, the following changes need to be made to vendor/incremental-maxhs:
             *
             * 1. In vendor/incremental-maxhs/src/ipamir/ipamir.h:
             *    - Add the function declaration: IPAMIR_API void ipamir_print_wcnf(void *solver);
             *    - Place it after ipamir_set_terminate and before the closing extern "C" block.
             *
             * 2. In vendor/incremental-maxhs/src/ipamir/IpamirMaxHS.cc:
             *    - Add a print_wcnf() method to the IpamirMaxHS class (in the public section):
             *      void print_wcnf() { formula->printFormula(); }
             *    - Add the C wrapper function in the extern "C" block:
             *      void ipamir_print_wcnf(void *solver) { import(solver)->print_wcnf(); }
             */
            // ipamir_print_wcnf(solver);
            // uint64_t obj = ipamir_val_obj(solver);

            Model consequences = compute_consequences(program, supporting_model);
            Model M_minus;
            for (Atom atom : supporting_model)
                if (consequences.contains(atom) == false)
                    M_minus.insert(atom);
            if (M_minus.empty())
                throw satisfied_exception(program, supporting_model);
            else
            {
                vector<Model> loop_formulas = compute_loop_formulas(program, M_minus);
                for (const auto &loop_formula : loop_formulas)
                {
                    vector<BodyIndex> external_bodies;
                    for (Atom p : loop_formula)
                    {
                        for (BodyIndex body_index : program.heads.at(p))
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
                                ipamir_add_hard(solver, body_to_variable[body_index]);
                            }
                        }
                        ipamir_add_hard(solver, -atom_mapper.get_variable(p));
                    }
                    ipamir_add_hard(solver, 0);
                }
                cout << "% Iteration " << nof_iterations << " completed. " << loop_formulas.size() << " loops found." << endl;
            }
        }
        else
            throw logic_error("Unknown result of the solver.");
    }
    // #endregion
    ipamir_release(solver);
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
