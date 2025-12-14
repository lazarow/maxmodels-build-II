#include <vector>
#include <omp.h>
#include "simplification.hpp"

void simplify(Program &program)
{
    BodyIndex nof_bodies = program.bodies.size();
    vector<Atom> forbidden_to_check;
    forbidden_to_check.reserve(program.forbidden.size());
    for (const auto &forbidden_atom : program.forbidden)
    {
        forbidden_to_check.push_back(forbidden_atom);
    }

    bool has_changed = true;
    while (has_changed)
    {
        has_changed = false;

        // #region Simplification of bodies (either rules or constraints)
        /**
         * The idea is simple, check every active literal (<>0) in each body.
         * If a literal is positive and it is a fact of it is contained in B+, then "remove" it from a body.
         * If a literal is negative and it is contained in B-, then "remove" it from a body.
         * If a literal is known to be falsified, then mark a body as falsified (-1).
         */
#pragma omp parallel for reduction(|| : has_changed)
        for (BodyIndex body_index = 1; body_index < nof_bodies; body_index++)
        {
            auto &body = program.bodies[body_index];
            if (body[0] <= 0)
            {
                continue;
            }
            unsigned int body_size = body.size();
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                auto &literal = body[literal_index];
                if (literal == 0)
                    continue;
                if (
                    literal > 0 &&
                    (program.facts.contains(literal) ||
                     program.required.contains(literal)))
                {
                    body[0]--;
                    literal = 0;
                    has_changed = true;
                }
                else if (
                    literal < 0 &&
                    (program.forbidden.contains(-literal) ||
                     program.heads.contains(-literal) == false))
                {
                    body[0]--;
                    literal = 0;
                    has_changed = true;
                }
                else if (
                    literal > 0 &&
                    (program.forbidden.contains(literal) ||
                     program.heads.contains(literal) == false))
                {
                    body[0] = -1;
                    has_changed = true;
                    break;
                }
                else if (
                    literal < 0 &&
                    (program.facts.contains(-literal) ||
                     program.required.contains(-literal)))
                {
                    body[0] = -1;
                    has_changed = true;
                    break;
                }
            }
        }
        // #endregion

        // #region Handling forbidden atoms (B-)
        /**
         * If an atom is known to be forbidden in a model and it is a head, then convert all its bodies into constraints.
         */
        for (Atom forbidden_atom : forbidden_to_check)
        {
            auto it = program.heads.find(forbidden_atom);
            if (it != program.heads.end())
            {
                for (const auto &body_index : it->second)
                {
                    program.constraints.insert(body_index);
                }
                program.heads.erase(it);
            }
        }
        forbidden_to_check.clear();
        // #endregion

        // #region Simplification of constraints
        /**
         * Checking all bodies related to constraints.
         * If a body is:
         * a) falsified -> remove a constraint,
         * b) satisfied -> UNSAT,
         * c) has only one literal -> convert to a required or forbidden atom.
         */
        for (auto it = program.constraints.begin(); it != program.constraints.end();)
        {
            BodyIndex body_index = *it;
            auto &body = program.bodies[body_index];
            bool should_erase = false;

            if (body[0] < 0)
            {
                should_erase = true;
                // has_changed = true; // Removing constraint as it is falsified doesn't have impact on other rules in the simplification.
            }
            else if (body[0] == 0)
            {
                throw unsatisfied_exception("A constraint is satisfied.");
            }
            // If the body has only one literal, then a contraint can be removed.
            else if (body[0] == 1)
            {
                unsigned int literal_index = 1;
                unsigned int body_size = body.size();
                Literal literal = body[literal_index];
                while (literal == 0 && literal_index < body_size)
                {
                    literal = body[++literal_index];
                }
                if (literal > 0)
                {
                    program.forbidden.insert(literal);
                    forbidden_to_check.push_back(literal);
                }
                else if (literal < 0)
                {
                    auto head_it = program.heads.find(-literal);
                    if (head_it != program.heads.end())
                    {
                        program.required.insert(-literal);
                    }
                }
                should_erase = true;
                has_changed = true;
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

            if (program.facts.contains(head))
            {
                heads_it = program.heads.erase(heads_it);
                continue;
            }

            auto &body_indices = heads_it->second;
            bool is_fact = false;
            bool is_falsified = true;
            bool should_erase = false;

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
                        is_falsified = false;
                    }
                    ++body_it;
                }
            }

            if (is_fact)
            {
                program.facts.insert(head);
                should_erase = true;
                has_changed = true;
            }
            else if (is_falsified)
            {
                program.forbidden.insert(head);
                forbidden_to_check.push_back(head);
                should_erase = true;
                has_changed = true;
            }

            if (should_erase)
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