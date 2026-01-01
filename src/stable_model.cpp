#include "stable_model.hpp"

Model compute_consequences(const Program &program, const Model &model)
{
    Model consequences;
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (const auto &[head, body_indices] : program.heads)
        {
            if (model.contains(head) == false || consequences.contains(head))
                continue;
            for (BodyIndex body_index : body_indices)
            {
                const Body &body = program.bodies[body_index];
                if (body[0] < 0)
                    continue;
                bool ok = true;
                for (unsigned int literal_index = 1; literal_index < body.size(); literal_index++)
                {
                    Literal literal = body[literal_index];
                    if (literal > 0 && consequences.contains(literal) == false)
                    {
                        ok = false;
                        break;
                    }
                    if (literal < 0 && model.contains(-literal))
                    {
                        ok = false;
                        break;
                    }
                }
                if (ok)
                {
                    consequences.insert(head);
                    changed = true;
                    break;
                }
            }
        }
    }
    return consequences;
}