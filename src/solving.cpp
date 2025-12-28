#include <cstdint>
#include <ipamir.h>
#include <cmath>
#include <queue>
#include "solving.hpp"
#include "stable_model.hpp"

void solve(Program &program)
{
    void *solver = ipamir_init();

    AtomMapper atom_mapper;

    // #region Step 1: Encoding normal rules.
    unsigned int nof_first_level_soft_clauses = 0;
    for (const auto &[head, body_indices] : program.heads)
    {
        queue<unsigned int> body_variables;
        for (const auto &body_index : body_indices)
        {
            const auto &body = program.bodies[body_index];
            unsigned int body_variable = atom_mapper.get_next_variable();
            body_variables.push(body_variable);

            if (body[0] < 0)
                continue;
            else if (body[0] == 0)
                // Justify a fact during the simplification should remove a head and its related bodies.
                throw logic_error("An encoded rule cannot be a fact.");

            // (a lub ~r1)
            // Skip if a is known to be true.
            if (program.required_atoms.contains(head) == false)
            {
                ipamir_add_hard(solver, atom_mapper.get_variable(head));
                ipamir_add_hard(solver, -body_variable);
                ipamir_add_hard(solver, 0);
            }

            // a <- b, not c.
            // b, not c <=> r1
            unsigned int body_size = body.size();
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                // (~r1 lub b) ^ (~r1 lub ~c)
                if (body[literal_index] != 0)
                {
                    ipamir_add_hard(solver, -body_variable);
                    ipamir_add_hard(solver, atom_mapper.get_variable(body[literal_index]));
                    ipamir_add_hard(solver, 0);
                }
            }
            // (r1 lub ~b lub c)
            ipamir_add_hard(solver, body_variable);
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                if (body[literal_index] != 0)
                {
                    ipamir_add_hard(solver, -atom_mapper.get_variable(body[literal_index]));
                }
            }
            ipamir_add_hard(solver, 0);

            // Adding the first level of soft clauses.
            // I use only actual atom from the program without auxilary atoms.
            // Additionally, I skip required atoms.
            if (program.symbols.contains(head) && program.required_atoms.contains(head) == false)
            {
                ipamir_add_soft_lit(solver, -body_variable, 1);
                nof_first_level_soft_clauses++;
            }
        }
        if (body_variables.empty())
            // A head must have at least one active rule.
            throw logic_error("An encoded head has no body.");

        // Required atoms should be already removed from the bodies.
        if (program.required_atoms.contains(head))
        {
            // (r1 lub r2)
            while (body_variables.empty() == false)
            {
                ipamir_add_hard(solver, body_variables.front());
                body_variables.pop();
            }
            ipamir_add_hard(solver, 0);
        }
        else
        {
            // (~a lub r1 lub r2)
            ipamir_add_hard(solver, -atom_mapper.get_variable(head));
            while (body_variables.empty() == false)
            {
                ipamir_add_hard(solver, body_variables.front());
                body_variables.pop();
            }
            ipamir_add_hard(solver, 0);
        }
    }
    // #endregion

    // #region Step 2: Encoding constraints.
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
            ipamir_add_hard(solver, -atom_mapper.get_variable(body[literal_index]));
        }
        ipamir_add_hard(solver, 0);
    }
    // #endregion

    // #region Step 3: Additional clauses for weights (optimization).
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

            uint64_t obj = ipamir_val_obj(solver);
            cout << "OBJ: " << obj << endl;

            if (is_stable_model(program, supporting_model))
                throw satisfied_exception(program, supporting_model);
            else
            {
                cout << "AN ANSWER IS NOT ANSWER SET" << endl;
                cout << "FOUNDING LOOP FORMULAS IS NOT IMPLEMENTED YET" << endl;
            }
            break;
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
        variable_to_atom[atom_to_variable[atom]] = atom;
    }
    return literal < 0 ? -atom_to_variable.at(atom) : atom_to_variable.at(atom);
}

bool AtomMapper::has_atom(Atom atom) const
{
    return atom_to_variable.contains(atom);
}

Atom AtomMapper::get_atom(unsigned int variable) const
{
    return variable_to_atom.at(variable);
}