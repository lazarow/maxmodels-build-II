#include <cstring>
#include <sstream>

#include "wcnf.hpp"
#include "process.hpp"

using namespace std;

// #region WMaxCDCL
inline bool extract_int_after(const string &s, const char *key, long &out)
{
    auto pos = s.find(key);
    if (pos == string::npos)
        return false;
    pos += strlen(key);
    out = strtol(s.c_str() + pos, nullptr, 10);
    return true;
}

inline bool extract_int_before(const string &s, const char *key, long &out)
{
    auto pos = s.find(key);
    if (pos == string::npos)
        return false;
    size_t i = pos;
    while (i > 0 && !isdigit(s[i - 1]))
        i--;
    size_t end = i;
    while (i > 0 && isdigit(s[i - 1]))
        i--;
    if (i == end)
        return false;
    out = strtol(s.substr(i, end - i).c_str(), nullptr, 10);
    return true;
}

inline bool extract_double_after(const string &s, const char *key, double &out)
{
    auto pos = s.find(key);
    if (pos == string::npos)
        return false;
    pos += strlen(key);
    out = strtod(s.c_str() + pos, nullptr);
    return true;
}

vector<UBEvent> parse_wmaxcdcl_log(const string &log)
{
    vector<UBEvent> events;
    istringstream iss(log);
    string line;
    UBEvent current;
    bool have_current = false;
    while (getline(iss, line))
    {
        if (line.rfind("c UB=", 0) == 0)
        {
            if (have_current)
            {
                events.push_back(current);
            }
            current = UBEvent{};
            have_current = true;
            auto pos = line.find("UB=");
            current.ub = std::atoi(line.c_str() + pos + 3);
            if (line.find("fails") != string::npos)
            {
                extract_int_after(line, "cnfls=", current.conflicts);
                extract_int_after(line, "hcnfls=", current.hard_conflicts);
            }
            else
            {
                extract_int_after(line, "confls=", current.conflicts);
                extract_int_after(line, "hconfls=", current.hard_conflicts);
                extract_int_before(line, " fixed vars at L0", current.fixed_vars_L0);
            }
            continue;
        }
        if (have_current)
        {
            extract_double_after(line, "succRate ", current.succ_rate);
        }
    }
    if (have_current)
        events.push_back(current);
    return events;
}
// #endregion

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
        cout << "% CDCL log = " << result.stdout_data << endl;
        auto ub_events = parse_wmaxcdcl_log(result.stdout_data);
        for (const auto &ev : ub_events)
        {
            cout << "% UB = " << ev.ub << ", conflicts = " << ev.conflicts << ", hard conflicts = " << ev.hard_conflicts << ", fixed vars at L0 = " << ev.fixed_vars_L0 << ", succ rate = " << ev.succ_rate << endl;
        }
        // throw unsatisfied_exception("Debug CDCL. Skipping the rest of the program.");
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