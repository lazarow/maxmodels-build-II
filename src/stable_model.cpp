#include "stable_model.hpp"

bool is_stable_model(const Program &program, const Model &supporting_model)
{

    PositiveRules positive_rules;
    for (const auto &[head, body_indices] : program.heads)
    {
        for (const auto &body_index : body_indices)
        {
            const auto &body = program.bodies[body_index];
            PositiveRule positive_rule;
            positive_rule.reserve(body[0] + 1);
            positive_rule.push_back(head);
            bool add_rule = true;
            for (unsigned int literal_index = 1; literal_index < body.size(); literal_index++)
            {
                if (body[literal_index] > 0)
                {
                    positive_rule.push_back(body[literal_index]);
                }
                else if (body[literal_index] < 0 && supporting_model.contains(-body[literal_index]))
                {
                    add_rule = false;
                    break;
                }
            }
            if (add_rule)
            {
                positive_rules.push_back(positive_rule);
            }
        }
    }

    bool is_fixpoint = false;
    Model fixpoint;
    while (!is_fixpoint)
    {
        is_fixpoint = true;
        for (auto it = positive_rules.begin(); it != positive_rules.end();)
        {
            if (fixpoint.contains((*it)[0]))
            {
                it = positive_rules.erase(it);
            }
            else
            {
                bool is_satisfied = true;
                unsigned int body_size = it->size();

                // Check if all positive literals in the body are satisfied
                for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
                {
                    const auto &literal = it->at(literal_index);
                    if (literal > 0 && !supporting_model.contains(literal))
                    {
                        is_satisfied = false;
                        break; // Early exit optimization
                    }
                }

                if (is_satisfied)
                {
                    is_fixpoint = false;
                    fixpoint.insert((*it)[0]);
                    it = positive_rules.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }
    return fixpoint == supporting_model;
}