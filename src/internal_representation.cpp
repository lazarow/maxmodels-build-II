#include <cmath>
#include <iostream>

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
    //
    program.atoms.emplace(head);

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

        // Adding the literal (here is always positive) to the set of atoms.
        program.atoms.emplace(literal);

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
    string minimization_rule;
    Atom zero;
    unsigned int nof_literals;
    unsigned int nof_negative_literals;
    Atom literal;
    Weight weight = 0;
    vector<Literal> literals;
    in >> zero >> nof_literals >> nof_negative_literals;
    minimization_rule = "6 " + to_string(zero) + " " + to_string(nof_literals) + " " + to_string(nof_negative_literals);
    literals.reserve(nof_literals);

    for (unsigned int i = 0; i < nof_literals; i++)
    {
        in >> literal;
        minimization_rule += " " + to_string(literal);
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
        minimization_rule += " " + to_string(weight);
        program.weights[literals[i]] = weight;
    }
    program.minimization_rules.push_back(minimization_rule);
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

    in >> header;
    if (header != "B+")
        throw logic_error("Expected B+ header, got " + header);
    in >> atom;
    while (atom != 0)
    {
        if (program.heads.contains(atom) == false && program.facts.contains(atom) == false)
            throw logic_error("B+'s atom " + to_string(atom) + " is not a head or fact in the program");

        // Skip if an atom is known as a fact already.
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
            throw logic_error("B-'s atom " + to_string(atom) + " is a fact or required in the program");
        program.forbidden_atoms.emplace(atom);
        in >> atom;
    }
    in >> header;
    if (header == "E")
    {
        in >> atom;
        while (atom != 0)
        {
            program.extended_atoms.emplace(atom);
            in >> atom;
        }
    }
}

void print_program_in_internal_format(const Program &program)
{
    // It shouldn't happen!
    if (program.constraints.empty() == false && program.forbidden_atoms.empty())
        throw logic_error("A program with constraints should have at least one forbidden atom.");

    // #region Facts
    for (const auto &atom : program.facts)
        cout << "1 " << atom << " 0 0" << endl;
    // #endregion

    // #region Constraints
    Atom constraint_head;
    if (program.constraints.empty() == false)
    {
        constraint_head = *program.forbidden_atoms.begin();
        for (const auto &body_index : program.constraints)
        {
            unsigned int nof_literals = 0;
            unsigned int nof_negative_literals = 0;
            const auto &body = program.bodies[body_index];
            // Note: Falsified constraints should be removed by the simplification.
            unsigned int body_size = body.size();
            // Note: Literals should be ordered.

            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                if (body[literal_index] != 0)
                {
                    nof_literals++;
                    if (body[literal_index] < 0)
                        nof_negative_literals++;
                }
            }
            cout << "1 " << constraint_head << " " << nof_literals << " " << nof_negative_literals;
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                if (body[literal_index] != 0)
                {
                    cout << " " << abs(body[literal_index]);
                }
            }
            cout << endl;
        }
    }
    // #endregion

    // #region Rules
    for (const auto &[head, body_indices] : program.heads)
    {
        for (const auto &body_index : body_indices)
        {
            unsigned int nof_literals = 0;
            unsigned int nof_negative_literals = 0;
            const auto &body = program.bodies[body_index];
            // Note: Falsified constraints should be removed by the simplification.
            unsigned int body_size = body.size();
            // Note: Literals should be ordered.

            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                if (body[literal_index] != 0)
                {
                    nof_literals++;
                    if (body[literal_index] < 0)
                        nof_negative_literals++;
                }
            }
            cout << "1 " << head << " " << nof_literals << " " << nof_negative_literals;
            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                if (body[literal_index] != 0)
                {
                    cout << " " << abs(body[literal_index]);
                }
            }
            cout << endl;
        }
    }
    // #endregion

    // #region Optimization Rule
    for (const auto &minimization_rule : program.minimization_rules)
        cout << minimization_rule << endl;
    // #endregion

    cout << "0" << endl;

    // #region Symbols
    for (const auto &[atom, symbol] : program.symbols)
        cout << atom << " " << symbol << endl;
    cout << "0" << endl;
    // #endregion

    // #region B+
    cout << "B+" << endl;
    for (const auto &atom : program.required_atoms)
        cout << atom << endl;
    cout << "0" << endl;
    // #endregion

    // #region B-
    cout << "B-" << endl;
    if (program.constraints.empty() == false)
        cout << constraint_head << endl;
    cout << "0" << endl;
    // #endregion

    // The number of models. I don't use it.
    cout << 1 << endl;
}