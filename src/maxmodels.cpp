#include <iostream>
// #include <cstdint>
// #include <ipamir.h>
#include <argparse.h>
#include "internal_representation.hpp"
#include "simplification.hpp"

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
        parser.ParseArgs(argc, argv);

        cout << VERSION << endl;
        cout << COPYRIGHT << endl;
        cout << "% Reading a logic program in the smodels internal format from stdin..." << endl;
        Program program = read_input(cin);
        // Simplify the program unless the "--skip-simplification" flag is set
        if (!(*skip_simplification))
        {
            simplify(program);
        }
        // Print the program only if the "--print" flag is set
        if (*print_only)
        {
            program.print();
            return 0;
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
    //     void *solver = ipamir_init();

    //     // Incremental MaxSAT example:
    //     // Variables: x1, x2

    //     // Add hard clause: (x1 OR x2)
    //     ipamir_add_hard(solver, 1); // x1
    //     ipamir_add_hard(solver, 2); // x2
    //     ipamir_add_hard(solver, 0); // End of clause

    //     // Add initial soft literals:
    //     ipamir_add_soft_lit(solver, -1, 1); // add ~x1 as soft with weight 1

    //     // Solve for the first time
    //     std::cout << "Solving with 1 soft literal..." << std::endl;
    //     int32_t result = ipamir_solve(solver);

    //     if (result == 10)
    //     {
    //         std::cout << "UNSAT (no models)" << std::endl;
    //     }
    //     else if (result == 20 || result == 30)
    //     {
    //         if (result == 30)
    //             std::cout << "OPTIMAL solution found." << std::endl;
    //         else
    //             std::cout << "SAT solution found (not necessarily optimal)." << std::endl;

    //         uint64_t obj = ipamir_val_obj(solver);
    //         std::cout << "Objective (sum of weights of falsified soft clauses): " << obj << std::endl;

    //         int x1 = ipamir_val_lit(solver, 1);
    //         int x2 = ipamir_val_lit(solver, 2);
    //         std::cout << "First solution:" << std::endl;
    //         std::cout << "  x1 = " << (x1 > 0 ? "True" : "False") << std::endl;
    //         std::cout << "  x2 = " << (x2 > 0 ? "True" : "False") << std::endl;
    //     }
    //     else
    //     {
    //         std::cout << "Solving was interrupted and no solution was found." << std::endl;
    //     }

    //     // Now, incrementally add another soft literal and solve again:
    //     ipamir_add_soft_lit(solver, -2, 2); // add ~x2 as soft with weight 2

    //     std::cout << "\nIncrementally solving after adding ~x2 as soft (weight 2)..." << std::endl;
    //     result = ipamir_solve(solver);

    //     if (result == 10)
    //     {
    //         std::cout << "UNSAT (no models)" << std::endl;
    //     }
    //     else if (result == 20 || result == 30)
    //     {
    //         if (result == 30)
    //             std::cout << "OPTIMAL solution found." << std::endl;
    //         else
    //             std::cout << "SAT solution found (not necessarily optimal)." << std::endl;

    //         uint64_t obj = ipamir_val_obj(solver);
    //         std::cout << "Objective (sum of weights of falsified soft clauses): " << obj << std::endl;

    //         int x1 = ipamir_val_lit(solver, 1);
    //         int x2 = ipamir_val_lit(solver, 2);
    //         std::cout << "Second solution:" << std::endl;
    //         std::cout << "  x1 = " << (x1 > 0 ? "True" : "False") << std::endl;
    //         std::cout << "  x2 = " << (x2 > 0 ? "True" : "False") << std::endl;
    //     }
    //     else
    //     {
    //         std::cout << "Solving was interrupted and no solution was found." << std::endl;
    //     }

    //     ipamir_release(solver);

    // #pragma omp parallel for
    //     for (int i = 1; i <= 100; i++)
    //     {
    //         int tid = omp_get_thread_num();
    //         printf("The thread %d  executes i = %d\n", tid, i);
    //     }

    //     return 0;
}