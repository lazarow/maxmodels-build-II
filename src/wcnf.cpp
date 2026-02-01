#include "wcnf.hpp"
#include "process.hpp"

// #region External Solver
void ExternalSolverWrapperWCNF::init()
{
    wcnf = "";
    soft_clauses = "";
}

void ExternalSolverWrapperWCNF::clear()
{
    wcnf = "";
    soft_clauses = "";
}

void ExternalSolverWrapperWCNF::clear_soft_clauses()
{
    soft_clauses = "";
}

void ExternalSolverWrapperWCNF::add_hard(Literal literal_or_zero)
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

void ExternalSolverWrapperWCNF::add_complex_soft(Literal literal_or_zero)
{
    if (is_soft_clause_open == false)
    {
        soft_clauses += to_string(literal_or_zero);
        is_soft_clause_open = true;
    }
    else if (literal_or_zero == 0)
    {
        soft_clauses += " 0\n";
        is_soft_clause_open = false;
    }
    else
    {
        soft_clauses += " " + to_string(literal_or_zero);
    }
}

void ExternalSolverWrapperWCNF::add_soft(Literal literal, Weight weight)
{
    soft_clauses += to_string(weight) + " " + to_string(literal) + " 0\n";
}

int32_t ExternalSolverWrapperWCNF::solve(const SolvingConfiguration &solving_configuration)
{
    ExecResult result = run_solver(solving_configuration.external_solver_path, {}, wcnf + soft_clauses);
    if (result.exit_code != 0 && result.exit_code != 10 && result.exit_code != 20 && result.exit_code != 30)
        throw runtime_error("Failed to solve the WCNF with the external solver.");
    bool isTimeout = result.stdout_data.find("Segmentation fault") != string::npos;
    isTimeout = isTimeout || result.stdout_data.find("segmentation fault") != string::npos;
    isTimeout = isTimeout || result.stdout_data.find("Segmentation Fault") != string::npos;
    isTimeout = isTimeout || result.stdout_data.find("TIMEOUT") != string::npos;
    if (isTimeout)
    {
        throw runtime_error("The external solver timed out.");
    }
    if (result.stdout_data.find("s UNSATISFIABLE") != string::npos)
    {
        return 10;
    }
    size_t model_position = result.stdout_data.find("\nv ");
    if (model_position == string::npos)
    {
        throw runtime_error("The external solver's result has not been found.");
    }
    unsigned int variable = 1;
    auto output_length = result.stdout_data.length();
    for (size_t i = model_position + 3; i < output_length && (result.stdout_data[i] != '\n' || result.stdout_data[i] != '\r'); i++)
    {
        variable_to_value[variable++] = result.stdout_data[i] == '1' ? 1 : 0;
    }
    return 30;
}

int32_t ExternalSolverWrapperWCNF::val_lit(Literal variable)
{
    if (variable < 0)
        return variable_to_value.at(-variable) == 0 ? 1 : 0;
    return variable_to_value.at(variable);
}
// #endregion