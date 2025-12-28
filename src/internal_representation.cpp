#include "internal_representation.hpp"

Program::Program()
{
    // Initialize the first dummy body, so all bodies start indexing from 1.
    bodies.emplace_back(1, -1);
}

Program read_input(istream &in)
{
    Program program;
    read_rules(in, program);
    read_symbols(in, program);
    read_compute_statements(in, program);
    return program;
}

void read_basic_rule(istream &in, Program &program)
{
    Atom head;
    Atom literal;
    Body body;
    unsigned int nof_literals = 0, nof_negative_literals = 0, nof_undetermined_literals = 0;
    bool is_fact = false, skip_body = false;

    // Handling input rule definition
    in >> head >> nof_literals >> nof_negative_literals;
    // Checking if a head is a fact
    is_fact = program.facts.contains(head);

    // Checking if a head is already mapped
    if (is_fact == false && program.heads.contains(head) == false)
        program.heads.emplace(head, unordered_set<BodyIndex>());

    // Reading rule body. Reserve space to avoid reallocations.
    body.reserve(nof_literals + 1);
    body.push_back(0);

    for (unsigned int i = 0; i < nof_literals; i++)
    {
        in >> literal;

        // If a literal is a head, then skip a body, i.e., a :- a. a :- not a.
        if (literal == head)
            skip_body = true;

        // No need to process literals.
        if (skip_body || is_fact)
            continue;

        if (nof_negative_literals > 0)
        {
            // If a negative literal is a fact -> skip a body
            if (program.facts.contains(literal))
                skip_body = true;
            else
            {
                nof_undetermined_literals++;
                body.push_back(-literal);
            }
            nof_negative_literals--;
        }
        else
        {
            // If a positive literal is a fact -> don't add it to a body
            if (program.facts.contains(literal) == false)
            {
                nof_undetermined_literals++;
                body.push_back(literal);
            }
        }
    }

    if (is_fact == false && skip_body == false)
    {
        // Set the number of undetermined literals in the body.
        body[0] = nof_undetermined_literals;

        if (body[0] == 0)
        {
            program.facts.emplace(head);
            for (const auto &body_index : program.heads[head])
            {
                // Set the number of undetermined literals of the head related bodies to 0 (satisfied).
                program.bodies[body_index][0] = 0;
            }
            program.heads.erase(head);
        }
        else
        {
            program.bodies.emplace_back(move(body));
            program.heads[head].insert(program.bodies.size() - 1);
        }
    }
}

void read_minimization_rule(istream &in, Program &program)
{
    Atom head;
    unsigned int nof_literals;
    unsigned int nof_negative_literals;
    Atom literal;
    Weight weight = 0;
    vector<Literal> literals;
    in >> head >> nof_literals >> nof_negative_literals;
    literals.reserve(nof_literals);

    for (unsigned int i = 0; i < nof_literals; i++)
    {
        in >> literal;
        if (nof_negative_literals > 0)
        {
            literals.push_back(-literal);
            nof_negative_literals--;
        }
        else
        {
            literals.push_back(literal);
        }
    }

    for (unsigned int i = 0; i < nof_literals; i++)
    {
        in >> weight;
        program.weights[literals[i]] = weight;
    }
}

void read_rules(istream &in, Program &program)
{
    unsigned int rule_type;
    in >> rule_type;
    while (rule_type != 0)
    {
        switch (rule_type)
        {
        case 1:
            read_basic_rule(in, program);
            break;
        case 2:
            throw logic_error("Unsupported rule type 2");
        case 3:
            throw logic_error("Unsupported rule type 3");
        case 4:
            throw logic_error("Unsupported rule type 4");
        case 5:
            throw logic_error("Unsupported rule type 5");
        case 6:
            read_minimization_rule(in, program);
            break;
        case 7:
            throw logic_error("Unsupported rule type 7");
        default:
            throw logic_error("Unsupported rule type: " + to_string(rule_type));
        }
        in >> rule_type;
    }
}

void read_symbols(istream &in, Program &program)
{
    Atom atom;
    string symbol;
    in >> atom;
    while (atom != 0)
    {
        in >> symbol;
        program.symbols.emplace(atom, move(symbol));
        in >> atom;
    }
}

void read_compute_statements(istream &in, Program &program)
{
    Atom atom;
    string header;
    long long int expected_nof_models;

    in >> header;
    if (header != "B+")
        throw logic_error("Expected B+ header, got " + header);
    in >> atom;
    while (atom != 0)
    {
        if (program.heads.contains(atom) == false && program.facts.contains(atom) == false)
            throw unsatisfied_exception("B+'s atom " + to_string(atom) + " is not a head or fact in the program");

        // Skip if an atom is a fact, as all its rules should be removed now.
        if (program.facts.contains(atom) == false)
            program.required_atoms.emplace(atom);

        in >> atom;
    }

    in >> header;
    if (header != "B-")
        throw logic_error("Expected B- header, got " + header);
    in >> atom;
    while (atom != 0)
    {
        if (program.facts.contains(atom) || program.required_atoms.contains(atom))
            throw unsatisfied_exception("B-'s atom " + to_string(atom) + " is a fact or required in the program");
        program.forbidden_atoms.emplace(atom);
        in >> atom;
    }

    in >> expected_nof_models;
    (void)expected_nof_models; // Suppress unused variable warning
}

void Program::print() const
{
    cout << "Facts:" << endl;
    for (const auto &fact : facts)
    {
        cout << fact << " ";
    }
    cout << endl;
    cout << "Required:" << endl;
    for (const auto &required : required_atoms)
    {
        cout << required << " ";
    }
    cout << endl;
    cout << "Forbidden:" << endl;
    for (const auto &forbidden : forbidden_atoms)
    {
        cout << forbidden << " ";
    }
    cout << endl;
    cout << "Weights:" << endl;
    for (const auto &weight : weights)
    {
        cout << weight.first << "@" << weight.second << " ";
    }
    cout << endl;
    cout << "Rules:" << endl;
    for (const auto &[head, body_indices] : heads)
    {
        for (const auto &body_index : body_indices)
        {
            cout << head << " <-";
            for (unsigned int i = 1; i < bodies[body_index].size(); i++)
            {
                cout << " " << bodies[body_index][i];
            }
            cout << " (" << bodies[body_index][0] << " undetermined literals)" << "." << endl;
        }
    }
    cout << "Constraints:" << endl;
    for (const auto &body_index : constraints)
    {
        cout << "<-";
        for (unsigned int i = 1; i < bodies[body_index].size(); i++)
        {
            cout << " " << bodies[body_index][i];
        }
        cout << " (" << bodies[body_index][0] << " undetermined literals)" << "." << endl;
    }
}