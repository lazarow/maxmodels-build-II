#pragma once

#include <string>
#include <vector>

using namespace std;

struct ExecResult
{
    int exit_code;
    string stdout_data;
};

ExecResult run_solver(const string &solver_path, const vector<std::string> &args, const string &stdin_data);