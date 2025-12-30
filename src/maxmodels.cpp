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
        auto skip_body_weights = parser.AddFlag("skip-body-weights", "Skip the body weights");
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
        solving_configuration.add_body_weights = !((bool)(*skip_body_weights));
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