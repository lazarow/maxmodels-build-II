#include "answer_set.hpp"

bool is_answer_set(const Program &program, const Model &model)
{
    // Checking the constraints first.
    for (const auto &constraint : program.constraints)
    {
        const auto &body = program.bodies[constraint];
        bool is_satisfied = true;
        unsigned int body_size = body.size();
        for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        {
            const auto &literal = body[literal_index];
            if (literal > 0)
            {
                if (!model.contains(literal))
                {
                    is_satisfied = false;
                    break;
                }
            }
            else if (literal < 0 && model.contains(-literal))
            {
                is_satisfied = false;
                break;
            }
        }
        if (is_satisfied)
        {
            return false;
        }
    }

    // Checking the rules next.
    Model smallest_fixpoint;
    bool is_fixpoint = false;
    while (!is_fixpoint)
    {
        is_fixpoint = true;
        for (const auto &[head, body_indices] : program.heads)
        {
            if (smallest_fixpoint.contains(head))
                continue;
            for (const auto &body_index : body_indices)
            {
                const auto &body = program.bodies[body_index];
                bool is_satisfied = true;
                unsigned int body_size = body.size();
                for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
                {
                    if (body[literal_index] > 0)
                    {
                        if (!model.contains(body[literal_index]))
                        {
                            is_satisfied = false;
                            break;
                        }
                    }
                    else if (body[literal_index] < 0 && model.contains(-body[literal_index]))
                    {
                        is_satisfied = false;
                        break;
                    }
                }
                if (is_satisfied)
                {
                    smallest_fixpoint.insert(head);
                    is_fixpoint = false;
                    break;
                }
            }
        }
    }

    return smallest_fixpoint == model;
}