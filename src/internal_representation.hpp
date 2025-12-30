#pragma once

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <limits>

using namespace std;

using Atom = unsigned int;
using Literal = int;
using Body = vector<Literal>;
using BodyIndex = unsigned int;
using Weight = unsigned long long int;
const Weight MAX_WEIGHT = numeric_limits<Weight>::max();
using Model = unordered_set<Atom>;

/**
 * The internal representation of the program.
 */
struct Program
{
    /**
     * The bodies of the program. `bi` denotes a body index.
     * Each body's first element is the number of undetermined literals.
     * If the first element is -1, then the body is unsatisfied.
     * On the contrary, if the first element is 0, then the body is satisfied.
     */
    vector<Body> bodies;                                 // { b_1, b_2, ..., b_n }
    unordered_map<Atom, unordered_set<BodyIndex>> heads; // h -> { bi_1, bi_2, ..., bi_n }
    unordered_set<BodyIndex> constraints;                // { bi_1, bi_2, ..., bi_n }
    unordered_set<Atom> facts;                           // { a_1, a_2, ..., a_n }
    unordered_set<Atom> required_atoms;                  // B+ = { a_1, a_2, ..., a_n }
    unordered_set<Atom> forbidden_atoms;                 // B- = { a_1, a_2, ..., a_n }
    unordered_map<Literal, Weight> weights;              // l -> w
    unordered_map<Atom, string> symbols;                 // a -> s
    unordered_map<BodyIndex, Atom> body_to_head;         // bi -> h

    Program();
    /**
     * Print the program in the form of the internal representation to the console.
     */
    void print() const;
};

void read_basic_rule(istream &in, Program &program);
void read_minimization_rule(istream &in, Program &program);
void read_rules(istream &in, Program &program);
void read_symbols(istream &in, Program &program);
void read_compute_statements(istream &in, Program &program);
Program read_input(istream &in);

class unsatisfied_exception : public logic_error
{
public:
    unsatisfied_exception(const string &message) : logic_error("The program is unsatisfied.")
    {
        cout << "% " << message << endl;
        cout << "INCONSISTENT" << endl;
    }
};

class satisfied_exception : public logic_error
{
public:
    satisfied_exception(const Program &program, const Model &stable_model) : logic_error("The program is satisfied.")
    {
        cout << "ANSWER" << endl;
        unordered_set<Atom> answer_set;
        for (const auto &atom : program.facts)
            answer_set.insert(atom);
        for (const auto &atom : program.required_atoms)
            answer_set.insert(atom);
        for (const auto &atom : stable_model)
            answer_set.insert(atom);
        for (const auto &atom : answer_set)
        {
            if (program.symbols.contains(atom))
                cout << program.symbols.at(atom) << " ";
        }
        cout << endl;
        if (program.weights.size() > 0)
        {
            Weight cost = 0;
            Weight maximum_cost = 0;
            for (const auto &[literal, weight] : program.weights)
            {
                if (literal > 0 && answer_set.contains(literal))
                    cost += weight;
                else if (literal < 0 && answer_set.contains(-literal) == false)
                    cost += weight;
                maximum_cost += weight;
            }
            cout << "% ALTERNATIVE COST " << (maximum_cost - cost) << endl;
            cout << "COST " << cost << endl;
            cout << "OPTIMUM" << endl;
        }
    }
};