#ifndef GIT_VERSION
#define GIT_VERSION "{placeholder-git-version}"
#endif

#ifndef EXTERNAL_SOLVER_PATH
#define EXTERNAL_SOLVER_PATH ""
#endif

#include <iostream>

#include "argparse.h"
#include "internal_representation.hpp"
#include "simplification.hpp"
#include "solving.hpp"

using namespace std;

const string VERSION = "% maxmodels (build II) " + string(GIT_VERSION);
const string COPYRIGHT = "% Copyright (c) 2026, Arkadiusz Nowakowski and Wojciech Wieczorek";

int main(int argc, char *argv[])
{
    try
    {
        argparse::Parser parser;
        auto simplification = parser.AddFlag("simplify", "Simplify the program");
        auto solving = parser.AddFlag("solve", "Encode and solve the program");
        auto external_solver_path = parser.AddArg<string>("external-solver", "The path to an external solver").Default("");
        auto use_metrics = parser.AddFlag("use-metrics", "Use metric-driven optimization");
        parser.ParseArgs(argc, argv);

        Program program = read_input(cin);

        if (*simplification)
        {
            simplify(program);
            if (*solving == false)
            {
                print_program_in_internal_format(program);
                return 0;
            }
        }
        else
        {
            just_constraints(program);
        }

        cout << VERSION << endl;
        cout << COPYRIGHT << endl;

        if (*solving)
        {
            SolvingConfiguration solving_configuration;
            solving_configuration.external_solver_path = *external_solver_path;
            if (solving_configuration.external_solver_path.empty())
                solving_configuration.external_solver_path = string(EXTERNAL_SOLVER_PATH);
            if (solving_configuration.external_solver_path.empty())
                throw runtime_error("The external solver path is not set.");
            cout << "% External solver path = " << solving_configuration.external_solver_path << endl;
            solving_configuration.use_metrics = *use_metrics;
            solve(program, solving_configuration);
        }
        return 0;
    }
    catch (const unsatisfied_exception &error)
    {
        return 0;
    }
    catch (const satisfied_exception &error)
    {
        return 0;
    }
    catch (const logic_error &error)
    {
        cout << "Error: " << error.what() << endl;
        return 1;
    }
    catch (const runtime_error &error)
    {
        cout << "Error: " << error.what() << endl;
        return 1;
    }
    catch (...)
    {
        cout << "Error: Unknown exception." << endl;
        return 1;
    }
}