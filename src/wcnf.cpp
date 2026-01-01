#include <ipamir.h>

#include "wcnf.hpp"
#include "process.hpp"

// #region IPAMIR (iMaxHS)
void IpamirWCNF::init()
{
    solver = ipamir_init();
}

void IpamirWCNF::clear()
{
    ipamir_release(solver);
}

void IpamirWCNF::add_hard(Literal literal_or_zero)
{
    ipamir_add_hard(solver, literal_or_zero);
}

void IpamirWCNF::add_soft(Literal literal, Weight weight)
{
    ipamir_add_soft_lit(solver, literal, weight);
}

int32_t IpamirWCNF::solve(const SolvingConfiguration &solving_configuration)
{
    // Mark as unused to avoid compiler warning
    (void)solving_configuration;
    return ipamir_solve(solver);
}

int32_t IpamirWCNF::val_lit(Literal literal)
{
    return ipamir_val_lit(solver, literal);
}
// #endregion

// #region WMaxCDCL
void WMaxCDCLWCNF::init()
{
    wcnf = "";
}

void WMaxCDCLWCNF::clear()
{
    wcnf = "";
}

void WMaxCDCLWCNF::add_hard(Literal literal_or_zero)
{
    if (is_hard_clause_open == false)
    {
        wcnf += "h " + to_string(literal_or_zero);
        is_hard_clause_open = true;
    }
    else if (literal_or_zero == 0)
    {
        wcnf += " 0\n";
        is_hard_clause_open = false;
    }
    else
    {
        wcnf += " " + to_string(literal_or_zero);
    }
}

void WMaxCDCLWCNF::add_soft(Literal literal, Weight weight)
{
    wcnf += to_string(weight) + " " + to_string(-literal) + " 0\n";
}

int32_t WMaxCDCLWCNF::solve(const SolvingConfiguration &solving_configuration)
{
    ExecResult result = run_solver(solving_configuration.wmaxcdcl_solver_path, {}, wcnf);
    if (result.exit_code != 0)
        throw runtime_error("Failed to solve the WCNF with WMaxCDCL.");
    bool isTimeout = result.stdout_data.find("Segmentation fault") != string::npos;
    isTimeout = isTimeout || result.stdout_data.find("segmentation fault") != string::npos;
    isTimeout = isTimeout || result.stdout_data.find("Segmentation Fault") != string::npos;
    isTimeout = isTimeout || result.stdout_data.find("TIMEOUT") != string::npos;
    if (isTimeout)
    {
        throw runtime_error("WMaxCDCL timed out.");
    }
    if (result.stdout_data.find("s UNSATISFIABLE") != string::npos)
    {
        return 10;
    }
    size_t model_position = result.stdout_data.find("\nv ");
    if (model_position == string::npos)
    {
        throw runtime_error("WMaxCDCL's result has not been found.");
    }
    unsigned int variable = 1;
    auto output_length = result.stdout_data.length();
    for (size_t i = model_position + 3; i < output_length && (result.stdout_data[i] != '\n' || result.stdout_data[i] != '\r'); i++)
    {
        variable_to_value[variable++] = result.stdout_data[i] == '1' ? 1 : 0;
    }
    return 30;
}

int32_t WMaxCDCLWCNF::val_lit(Literal variable)
{
    if (variable < 0)
        return variable_to_value.at(-variable) == 0 ? 1 : 0;
    return variable_to_value.at(variable);
}
// #endregion