#include <cstdint>
#include <ipamir.h>
#include <cmath>
#include "solving.hpp"
#include "answer_set.hpp"

void solve(Program &program)
{
    void *solver = ipamir_init();

    // #region Step 1: Encoding normal rules.
    unsigned int last_body_variable = program.max_atom;
    unsigned int nof_first_level_soft_clauses = 0;
    for (const auto &[head, body_indices] : program.heads)
    {
        unsigned int nof_body_variables = 0;
        for (const auto &body_index : body_indices)
        {
            const auto &body = program.bodies[body_index];
            if (body[0] < 0)
                continue;
            else if (body[0] == 0)
                // Justify a fact during the simplification should remove a head and its related bodies.
                throw logic_error("An encoded rule cannot be a fact.");

            last_body_variable++;
            nof_body_variables++;

            // a <- b, not c.
            // b, not c <=> r1
            unsigned int body_size = body.size();
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                // (~r1 lub b) ^ (~r1 lub ~c)
                if (body[literal_index] != 0)
                {
                    ipamir_add_hard(solver, -last_body_variable);
                    ipamir_add_hard(solver, body[literal_index]);
                    ipamir_add_hard(solver, 0);
                }
            }
            // (r1 lub ~b lub c)
            ipamir_add_hard(solver, last_body_variable);
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                if (body[literal_index] != 0)
                {
                    ipamir_add_hard(solver, -body[literal_index]);
                }
            }
            ipamir_add_hard(solver, 0);

            // Adding the first level of soft clauses.
            // I use only actual atom from the program without auxilary atoms.
            // Additionally, I skip required atoms.
            if (program.symbols.contains(head) && program.required_atoms.contains(head) == false)
            {
                ipamir_add_soft_lit(solver, -last_body_variable, 1);
                nof_first_level_soft_clauses++;
            }
        }
        if (nof_body_variables == 0)
            // A head must have at least one active rule.
            throw logic_error("An encoded head has no body.");

        // Required atoms should be already removed from the bodies.
        if (program.required_atoms.contains(head))
        {
            // (r1 lub r2)
            while (nof_body_variables > 0)
            {
                ipamir_add_hard(solver, last_body_variable - nof_body_variables + 1);
                nof_body_variables--;
            }
            ipamir_add_hard(solver, 0);
        }
        else
        {
            // (~a lub r1 lub r2)
            ipamir_add_hard(solver, -head);
            while (nof_body_variables > 0)
            {
                ipamir_add_hard(solver, last_body_variable - nof_body_variables + 1);
                nof_body_variables--;
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
            ipamir_add_hard(solver, -body[literal_index]);
        }
        ipamir_add_hard(solver, 0);
    }
    // #endregion

    // #region Step 3: Additional clauses for weights (optimization).
    if (program.weights.size() > 0)
    {
        unsigned int augmenting = pow(10, ceil(log10(nof_first_level_soft_clauses)));
        for (const auto &[literal, weight] : program.weights)
        {
            Atom atom = literal < 0 ? -literal : literal;
            if (
                program.symbols.contains(atom) &&
                program.heads.contains(atom) &&
                program.required_atoms.contains(atom) == false)
            {
                ipamir_add_soft_lit(solver, literal, weight * augmenting);
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
                if (ipamir_val_lit(solver, head) > 0)
                    supporting_model.insert(head);
            }
            ipamir_print_wcnf(solver);
            uint64_t obj = ipamir_val_obj(solver);
            cout << result << endl;
            cout << "Objective value: " << obj << endl;
            cout << "Supporting model: ";
            for (const auto &atom : supporting_model)
            {
                cout << atom << " ";
            }
            cout << endl;
            cout << is_answer_set(program, supporting_model) << endl;
            break;
        }
        else
            throw logic_error("Unknown result of the solver.");
    }
    ipamir_release(solver);
}