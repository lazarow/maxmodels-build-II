#include <vector>
#include <omp.h>
#include <cmath>
#include <sstream>
#include "simplification.hpp"

void simplify(Program &program)
{
    /**
     * Simplification can be made only if a program is a valid Answer Set Program.
     */
    if (program.extended_atoms.empty() == false)
    {
        throw logic_error("A program with extended atoms cannot be simplified!");
    }

    BodyIndex nof_bodies = program.bodies.size();
    vector<Atom> constraint_heads;
    constraint_heads.reserve(program.forbidden_atoms.size());
    for (const auto &forbidden_atom : program.forbidden_atoms)
    {
        constraint_heads.push_back(forbidden_atom);
    }

    bool has_changed = true;
    while (has_changed)
    {
        has_changed = false;

        // #region Simplification of bodies (either rules or constraints)
#pragma omp parallel reduction(|| : has_changed)
        {
#pragma omp for
            for (BodyIndex body_index = 1; body_index < nof_bodies; body_index++)
            {
                auto &body = program.bodies[body_index];

                if (body[0] <= 0) // Skip if a body is falsified or satisfied already.
                    continue;

                unsigned int body_size = body.size();
                for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
                {
                    auto &literal = body[literal_index];

                    // Skip if a literal is determined.
                    if (literal == 0)
                        continue;

                    // If a positive literal is a forbidden atom or doesn't have a related head and it's not a fact, then the body is falsified.
                    if (
                        literal > 0 &&
                        (program.forbidden_atoms.contains(literal) ||
                         (program.heads.contains(literal) == false && program.facts.contains(literal) == false)))
                    {
                        body[0] = -1;
                        literal = 0;
                        has_changed = true;
                        break;
                    }
                    // If a negative literal is a fact or a required atom, then the body is falsified.
                    else if (
                        literal < 0 &&
                        (program.facts.contains(-literal) ||
                         program.required_atoms.contains(-literal)))
                    {
                        body[0] = -1;
                        literal = 0;
                        has_changed = true;
                        break;
                    }
                    // If a positive literal is a fact or a required atom, then the literal is determined.
                    else if (
                        literal > 0 &&
                        (program.facts.contains(literal) ||
                         program.required_atoms.contains(literal)))
                    {
                        body[0]--;
                        literal = 0;
                        has_changed = true;
                    }
                    // If a negative literal is a forbidden atom or doesn't have a related head, then the literal is determined.
                    else if (
                        literal < 0 &&
                        (program.forbidden_atoms.contains(-literal) ||
                         (program.heads.contains(-literal) == false && program.facts.contains(-literal) == false)))
                    {
                        body[0]--;
                        literal = 0;
                        has_changed = true;
                    }
                }
            }
        }
        // #endregion

        // #region Creating constraints
        for (Atom constraint_head : constraint_heads)
        {
            // If a forbidden atom is a head, then convert all its bodies to constraints.
            auto it = program.heads.find(constraint_head);
            if (it != program.heads.end())
            {
                for (const auto &body_index : it->second)
                {
                    program.constraints.insert(body_index);
                }
                program.heads.erase(it); // Remove the head from the heads.
            }
        }
        constraint_heads.clear();
        // #endregion

        // #region Checking the constraints
        /**
         * Checking all bodies related to constraints.
         * If a body is:
         * a) falsified -> remove a constraint,
         * b) satisfied -> UNSAT,
         * c) has only one literal -> convert to either a required or forbidden atom.
         */
        for (auto it = program.constraints.begin(); it != program.constraints.end();)
        {
            BodyIndex body_index = *it;
            auto &body = program.bodies[body_index];
            bool should_erase = false;

            // If a body is falsified, then remove a constraint.
            if (body[0] < 0)
            {
                should_erase = true;
                // Note: Removing a falsified constraint doesn't trigger further simplification
            }
            // If a body is satisfied, then the program is UNSAT.
            else if (body[0] == 0)
            {
                throw unsatisfied_exception("A constraint is satisfied.");
            }

            /**
             * If the body has only one undetermined literal, then a constraint can be removed,
             * and the literal can be convert to either a required or forbidden atom.
             */
            else if (body[0] == 1)
            {
                unsigned int literal_index = 1;
                unsigned int body_size = body.size();
                Literal literal = body[literal_index];
                while (literal == 0 && ++literal_index < body_size)
                    literal = body[literal_index];

                if (literal > 0)
                {
                    // In this moment, other scenarios for a constraint body should be resolved by the rules' simplification.
                    if (program.required_atoms.contains(literal))
                        throw unsatisfied_exception("A single-literal constraint with a positive literal contains a required atom.");

                    program.forbidden_atoms.emplace(literal);
                    constraint_heads.push_back(literal);
                }
                else if (literal < 0)
                {
                    // A related head must exist! See, that if negative literal doesn't have a related head, then it will determined.
                    if (program.forbidden_atoms.contains(-literal))
                        throw unsatisfied_exception("A single-literal constraint with a negative literal contains a forbidden atom.");
                    if (program.heads.contains(-literal) == false)
                        throw unsatisfied_exception("A single-literal constraint with a negative literal doesn't have a related head.");

                    program.required_atoms.emplace(-literal);
                }
                body[0] = 0; // The body is determined.
                should_erase = true;
                has_changed = true; // Change due to the forbidden and required atoms update.
            }
            if (should_erase)
            {
                it = program.constraints.erase(it);
            }
            else
            {
                ++it;
            }
        }
        // #endregion

        // #region Simplification of rules
        for (auto heads_it = program.heads.begin(); heads_it != program.heads.end();)
        {
            Atom head = heads_it->first;
            auto &body_indices = heads_it->second;
            bool is_fact = false;
            bool cannot_be_justified = true;
            bool should_be_erased = false;

            if (body_indices.empty())
            {
                // A head should always have at least one body!
                throw logic_error("A head " + to_string(head) + " (" + program.symbols[head] + ") has no bodies.");
            }

            for (auto body_it = body_indices.begin(); body_it != body_indices.end();)
            {
                BodyIndex body_index = *body_it;
                auto &body = program.bodies[body_index];
                if (body[0] < 0)
                {
                    body_it = body_indices.erase(body_it);
                }
                else
                {
                    if (body[0] == 0)
                    {
                        is_fact = true;
                        break;
                    }
                    if (body[0] > 0)
                    {
                        cannot_be_justified = false;
                    }
                    ++body_it;
                }
            }

            if (is_fact)
            {
                // Remove the head from the required atoms, as it is a fact now.
                if (program.required_atoms.contains(head))
                    program.required_atoms.erase(head);

                if (program.forbidden_atoms.contains(head))
                    throw unsatisfied_exception("A head " + program.symbols[head] + " is both fact and forbidden in the program.");

                program.facts.emplace(head);

                // Set all bodies related to the head as satisfied (will be ignored in the simplification and encoding).
                for (const auto &body_index : body_indices)
                    program.bodies[body_index][0] = 0;

                should_be_erased = true;
                has_changed = true;
            }
            else if (cannot_be_justified)
            {
                // Head has bodies but all are falsified - mark as forbidden
                if (program.required_atoms.contains(head))
                    throw unsatisfied_exception("A head " + to_string(head) + " (" + program.symbols[head] + ") is both required and forbidden in the program.");

                program.forbidden_atoms.emplace(head);
                constraint_heads.push_back(head);
                should_be_erased = true;
                has_changed = true;
            }
            if (should_be_erased)
            {
                heads_it = program.heads.erase(heads_it);
            }
            else
            {
                ++heads_it;
            }
        }
        // #endregion
    }
}

void just_constraints(Program &program)
{
    vector<Atom> constraint_heads;
    constraint_heads.reserve(program.forbidden_atoms.size());
    for (const auto &forbidden_atom : program.forbidden_atoms)
        constraint_heads.push_back(forbidden_atom);
    for (Atom constraint_head : constraint_heads)
    {
        // If a forbidden atom is a head, then convert all its bodies to constraints.
        auto it = program.heads.find(constraint_head);
        if (it != program.heads.end())
        {
            for (const auto &body_index : it->second)
            {
                program.constraints.insert(body_index);
            }
            program.heads.erase(it); // Remove the head from the heads.
        }
    }
}