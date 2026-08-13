#include <cstring>
#include <sstream>

#include "wcnf.hpp"
#include "process.hpp"

using namespace std;

auto extract_after_colon = [](const string &line) -> string
{
    auto pos = line.find(':');
    if (pos != string::npos)
    {
        auto rest = line.substr(pos + 1);
        auto first_digit = rest.find_first_not_of(" \t");
        if (first_digit != string::npos)
            rest = rest.substr(first_digit);
        auto last_digit = rest.find_first_of(" \t\n\r");
        if (last_digit != string::npos)
            rest = rest.substr(0, last_digit);
        rest.erase(rest.find_last_not_of(" \t\r\n") + 1);
        return rest;
    }
    return "";
};

auto extract_full_line = [](const string &keyword, const string &stdout_data) -> string
{
    size_t pos = stdout_data.find(keyword);
    if (pos == string::npos)
        return "";
    size_t endpos = stdout_data.find('\n', pos);
    if (endpos == string::npos)
        endpos = stdout_data.size();
    return stdout_data.substr(pos, endpos - pos);
};

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
    vector<string> args;
    ExecResult result = run_solver(solving_configuration.external_solver_path, args, wcnf + soft_clauses, solving_configuration.timeout_seconds);
    if (result.timed_out)
        throw runtime_error("The external solver timed out.");
    if (result.exit_code != 0 && result.exit_code != 1 && result.exit_code != 10 && result.exit_code != 20 && result.exit_code != 30)
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

    if (solving_configuration.debug_cdcl)
    {
        // cout << "% CDCL log = " << result.stdout_data << endl;

        // Parse and print relevant stats from the solver output if --debug-cdcl is enabled
        istringstream iss(result.stdout_data);
        string line;
        string cpu_time, conflicts, decisions, propagations, conflict_literals;
        string la_execution, lam_execution, sla_time;
        while (getline(iss, line))
        {
            if (line.find("c CPU time") != string::npos && cpu_time.empty())
                cpu_time = extract_after_colon(line);
            else if (line.find("c conflicts") != string::npos && conflicts.empty())
                conflicts = extract_after_colon(line);
            else if (line.find("c decisions") != string::npos && decisions.empty())
                decisions = extract_after_colon(line);
            else if (line.find("c propagations") != string::npos && propagations.empty())
                propagations = extract_after_colon(line);
            else if (line.find("c conflict literals") != string::npos && conflict_literals.empty())
                conflict_literals = extract_after_colon(line);
            else if (line.find("LA execution") != string::npos && la_execution.empty())
                la_execution = extract_after_colon(line);
            else if (line.find("LAM execution") != string::npos && lam_execution.empty())
                lam_execution = extract_after_colon(line);
            else if (line.find("SLA time") != string::npos && sla_time.empty())
                sla_time = extract_after_colon(line);
        }
        if (!cpu_time.empty())
            cout << "% CDCL CPU time: " << cpu_time << endl;
        if (!conflicts.empty())
            cout << "% CDCL conflicts: " << conflicts << endl;
        if (!decisions.empty())
            cout << "% CDCL decisions: " << decisions << endl;
        if (!propagations.empty())
            cout << "% CDCL propagations: " << propagations << endl;
        if (!conflict_literals.empty())
            cout << "% CDCL conflict literals: " << conflict_literals << endl;
        if (!la_execution.empty())
            cout << "% CDCL LA execution (succ rate): " << la_execution << endl;
        if (!lam_execution.empty())
            cout << "% CDCL LAM execution (cores found): " << lam_execution << endl;
        if (!sla_time.empty())
            cout << "% CDCL SLA time: " << sla_time << endl;
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