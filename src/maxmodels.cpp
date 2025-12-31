#include <iostream>
#include <argparse.h>
#include "internal_representation.hpp"
#include "simplification.hpp"
#include "solving.hpp"

using namespace std;

const string VERSION = "% maxmodels (build II) 1.0.0";
const string COPYRIGHT = "% Copyright (c) 2025, Arkadiusz Nowakowski and Wojciech Wieczorek";

int main(int argc, char *argv[])
{
    try
    {
        argparse::Parser parser;
        auto print_only = parser.AddFlag("print", 'p', "Print the program only and exit");
        auto skip_simplification = parser.AddFlag("skip-simplification", "Skip the simplification step");
        auto solving_strategy = parser.AddArg<int>("solving-strategy", 's', "The solving strategy to use [0=baseline, 1=all rules, 2=non-extended rules, 3=lazy]").Default(2);
        auto loop_formulas_strategy = parser.AddArg<int>("loop-formulas-strategy", 'l', "The loop formulas strategy to use [0=all, 1=first only]").Default(0);
        parser.ParseArgs(argc, argv);

        cout << VERSION << endl;
        cout << COPYRIGHT << endl;
        cout << "% Reading a logic program in the smodels internal format from stdin..." << endl;
        Program program = read_input(cin);
        // Simplify the program unless the "--skip-simplification" flag is set
        if ((bool)(*skip_simplification) == false)
            simplify(program);
        // Print the program only if the "--print" flag is set
        if ((bool)(*print_only))
            program.print();
        SolvingConfiguration solving_configuration;
        solving_configuration.solving_strategy = static_cast<SolvingStrategy>(*solving_strategy);
        if (*solving_strategy < 0 || *solving_strategy > 2)
        {
            cerr << "Error: The solving strategy must be one of 0 (baseline), 1 (all rules) or 2 (non-extended rules)." << endl;
            return 1;
        }
        solving_configuration.loop_formulas_strategy = static_cast<LoopFormulasStrategy>(*loop_formulas_strategy);
        if (*loop_formulas_strategy < 0 || *loop_formulas_strategy > 2)
        {
            cerr << "Error: The loop formulas strategy must be one of 0 (maximal), 1 (minimal first), or 2 (minimal smallest)." << endl;
            return 1;
        }
        cout << "% Solving strategy = " << (int)(solving_configuration.solving_strategy) << endl;
        cout << "% Loop formulas strategy = " << (int)(solving_configuration.loop_formulas_strategy) << endl;
        solve(program, solving_configuration);
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