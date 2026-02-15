#pragma once

#include <unordered_map>
#include <vector>

#include "internal_representation.hpp"
#include "solving.hpp"

using namespace std;

// The remain after IPAMIR.
class WCNF
{
public:
    virtual void init() = 0;
    virtual void clear() = 0;
    virtual void add_hard(Literal literal_or_zero) = 0;
    virtual void add_soft(Literal literal, Weight weight) = 0;
    virtual void add_complex_soft(Literal literal_or_zero) = 0;
    virtual int32_t solve(const SolvingConfiguration &solving_configuration) = 0;
    virtual int32_t val_lit(Literal literal) = 0;
};

class ExternalSolverWrapperWCNF : public WCNF
{
public:
    void init() override;
    void clear() override;
    void clear_soft_clauses();
    void add_hard(Literal literal_or_zero) override;
    void add_soft(Literal literal, Weight weight) override;
    void add_complex_soft(Literal literal_or_zero) override;
    int32_t solve(const SolvingConfiguration &solving_configuration) override;
    int32_t val_lit(Literal literal) override;

private:
    string wcnf;
    string soft_clauses;
    bool is_hard_clause_open = false;
    bool is_soft_clause_open = false;
    unordered_map<unsigned int, int32_t> variable_to_value;
};
