#include "encoding.hpp"
#include "solving.hpp"

void clark_completion(
    const Program &program,
    unique_ptr<WCNF> &wcnf,
    unordered_map<BodyIndex, unsigned int> &body_to_variable,
    AtomMapper &atom_mapper)
{
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
        vector<unsigned int> body_variables;
        for (const auto &body_index : body_indices)
        {
            const auto &body = program.bodies[body_index];
            if (body[0] < 0)
                // All falsified rules should be removed during the simplification.
                throw logic_error("A rule should be removed during the simplification.");
            else if (body[0] == 0)
                // Justifying a fact during the simplification should remove a head and its related bodies.
                throw logic_error("A rule is a fact.");

            // Create a new variable for a body.
            unsigned int body_variable = atom_mapper.get_next_variable();
            body_variables.push_back(body_variable);
            body_to_variable[body_index] = body_variable;

            // (¬r1 ∨ a)
            // Skip if `a` is known to be true, i.e., (¬r1 ∨ true) = true.
            if (program.required_atoms.contains(head) == false)
            {
                wcnf->add_hard(-body_variable);
                wcnf->add_hard(atom_mapper.get_variable(head));
                wcnf->add_hard(0);
            }

            // (b ∨ ¬r1) ∧ (¬c ∨ ¬r1)
            unsigned int body_size = body.size();
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                // Skip if a literal is determined.
                if (body[literal_index] != 0)
                {
                    wcnf->add_hard(atom_mapper.get_variable(body[literal_index]));
                    wcnf->add_hard(-body_variable);
                    wcnf->add_hard(0);
                }
            }
            // (r1 ∨ ¬b ∨ c)
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                if (body[literal_index] != 0) // Skip if a literal is determined.
                {
                    wcnf->add_hard(-atom_mapper.get_variable(body[literal_index]));
                }
            }
            wcnf->add_hard(body_variable);
            wcnf->add_hard(0);
        }
        if (body_variables.empty())
            // A head must have at least one active rule.
            throw logic_error("An encoded head has no body.");

        /**
         * (r1 ∨ r2 ∨ r3 ∨ ¬a)
         * If `a` is true then:
         * (r1 ∨ r2 ∨ r3 ∨ ¬a) = (r1 ∨ r2 ∨ r3 ∨ false) = (r1 ∨ r2 ∨ r3)
         */
        for (unsigned int body_variable : body_variables)
        {
            wcnf->add_hard(body_variable);
        }
        if (program.required_atoms.contains(head) == false)
        {
            wcnf->add_hard(-atom_mapper.get_variable(head));
        }
        wcnf->add_hard(0);
    }
}

void lp2sat_like_facts(
    const Program &program,
    unique_ptr<WCNF> &wcnf,
    AtomMapper &atom_mapper)
{
    for (const auto &atom : program.facts)
    {
        wcnf->add_hard(atom_mapper.get_variable(atom));
        wcnf->add_hard(0);
    }
}

void lp2sat_like_required_atoms(
    const Program &program,
    unique_ptr<WCNF> &wcnf,
    AtomMapper &atom_mapper)
{
    for (const auto &atom : program.required_atoms)
    {
        wcnf->add_hard(atom_mapper.get_variable(atom));
        wcnf->add_hard(0);
    }
}

void lp2sat_like_rules(
    const Program &program,
    unique_ptr<WCNF> &wcnf,
    unordered_map<BodyIndex, unsigned int> &body_to_variable,
    AtomMapper &atom_mapper)
{
    for (const auto &[head, body_indices] : program.heads)
    {
        // List of bodies for a head (the Tseitin transformation).
        vector<unsigned int> body_variables;
        for (const auto &body_index : body_indices)
        {
            const auto &body = program.bodies[body_index];
            if (body_indices.size() == 1)
            {
                unsigned int body_size = body.size();
                for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
                {
                    if (body[literal_index] != 0)
                    {
                        wcnf->add_hard(atom_mapper.get_variable(body[literal_index]));
                        wcnf->add_hard(-atom_mapper.get_variable(head));
                        wcnf->add_hard(0);
                    }
                }
                for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
                {
                    if (body[literal_index] != 0)
                        wcnf->add_hard(-atom_mapper.get_variable(body[literal_index]));
                }
                wcnf->add_hard(atom_mapper.get_variable(head));
                wcnf->add_hard(0);
                break;
            }
            else
            {
                // Example: a <- b, not c.
                // Example: (¬bt(r1) ∨ a)
                unsigned int body_variable = atom_mapper.get_next_variable();
                body_variables.push_back(body_variable);
                body_to_variable[body_index] = body_variable;
                wcnf->add_hard(-body_variable);
                wcnf->add_hard(atom_mapper.get_variable(head));
                wcnf->add_hard(0);

                // Example: (b ∨ ¬bt(r1)) ∧ (¬c ∨ ¬bt(r1))
                unsigned int body_size = body.size();
                for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
                {
                    // Skip if a literal is determined.
                    if (body[literal_index] != 0)
                    {
                        wcnf->add_hard(atom_mapper.get_variable(body[literal_index]));
                        wcnf->add_hard(-body_variable);
                        wcnf->add_hard(0);
                    }
                }

                // (bt(r1) ∨ ¬b ∨ c)
                for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
                {
                    if (body[literal_index] != 0)
                        wcnf->add_hard(-atom_mapper.get_variable(body[literal_index]));
                }
                wcnf->add_hard(body_variable);
                wcnf->add_hard(0);
            }
        }
        if (body_variables.empty() == false)
        {
            // (r1 ∨ r2 ∨ r3 ∨ ¬a)
            for (unsigned int body_variable : body_variables)
                wcnf->add_hard(body_variable);
            wcnf->add_hard(-atom_mapper.get_variable(head));
            wcnf->add_hard(0);
        }
    }
}

void lp2sat_like_other_atoms(
    const Program &program,
    unique_ptr<WCNF> &wcnf,
    AtomMapper &atom_mapper)
{
    for (const auto &atom : program.atoms)
    {
        if (program.facts.contains(atom))
            continue;
        if (program.required_atoms.contains(atom))
            continue;
        if (program.heads.contains(atom))
            continue;
        if (program.extended_atoms.contains(atom))
            continue;
        wcnf->add_hard(-atom_mapper.get_variable(atom));
        wcnf->add_hard(0);
    }
}

void lp2sat_like(
    const Program &program,
    unique_ptr<WCNF> &wcnf,
    unordered_map<BodyIndex, unsigned int> &body_to_variable,
    AtomMapper &atom_mapper)
{
    lp2sat_like_rules(program, wcnf, body_to_variable, atom_mapper);
    lp2sat_like_facts(program, wcnf, atom_mapper);
    lp2sat_like_required_atoms(program, wcnf, atom_mapper);
    lp2sat_like_other_atoms(program, wcnf, atom_mapper);
}
