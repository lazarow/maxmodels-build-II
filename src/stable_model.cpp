#include "stable_model.hpp"

Model compute_consequences(const Program &program, const Model &model)
{
    Model consequences;

    bool has_changed = true;
    while (has_changed)
    {
        has_changed = false;
        for (const auto &[head, body_indices] : program.heads)
        {
            if (model.contains(head) == false || consequences.contains(head))
                continue;

            for (BodyIndex body_index : body_indices)
            {
                const Body &body = program.bodies[body_index];
                // Note: I don't have to check bodies' numbers of undetermined literals, as I've already check them during encoding!

                bool is_justified = true;
                for (unsigned int literal_index = 1; literal_index < body.size(); literal_index++)
                {
                    Literal literal = body[literal_index];
                    if (literal > 0 && consequences.contains(literal) == false)
                    {
                        is_justified = false;
                        break;
                    }
                    if (literal < 0 && model.contains(-literal))
                    {
                        is_justified = false;
                        break;
                    }
                }
                if (is_justified)
                {
                    consequences.insert(head);
                    has_changed = true;
                    break;
                }
            }
        }
    }
    return consequences;
}