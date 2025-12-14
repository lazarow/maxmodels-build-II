#include <iostream>
#include <cstdint>
#include <ipamir.h>
#include <omp.h>

int main()
{
    void *solver = ipamir_init();

    // Incremental MaxSAT example:
    // Variables: x1, x2

    // Add hard clause: (x1 OR x2)
    ipamir_add_hard(solver, 1); // x1
    ipamir_add_hard(solver, 2); // x2
    ipamir_add_hard(solver, 0); // End of clause

    // Add initial soft literals:
    ipamir_add_soft_lit(solver, -1, 1); // add ~x1 as soft with weight 1

    // Solve for the first time
    std::cout << "Solving with 1 soft literal..." << std::endl;
    int32_t result = ipamir_solve(solver);

    if (result == 10)
    {
        std::cout << "UNSAT (no models)" << std::endl;
    }
    else if (result == 20 || result == 30)
    {
        if (result == 30)
            std::cout << "OPTIMAL solution found." << std::endl;
        else
            std::cout << "SAT solution found (not necessarily optimal)." << std::endl;

        uint64_t obj = ipamir_val_obj(solver);
        std::cout << "Objective (sum of weights of falsified soft clauses): " << obj << std::endl;

        int x1 = ipamir_val_lit(solver, 1);
        int x2 = ipamir_val_lit(solver, 2);
        std::cout << "First solution:" << std::endl;
        std::cout << "  x1 = " << (x1 > 0 ? "True" : "False") << std::endl;
        std::cout << "  x2 = " << (x2 > 0 ? "True" : "False") << std::endl;
    }
    else
    {
        std::cout << "Solving was interrupted and no solution was found." << std::endl;
    }

    // Now, incrementally add another soft literal and solve again:
    ipamir_add_soft_lit(solver, -2, 2); // add ~x2 as soft with weight 2

    std::cout << "\nIncrementally solving after adding ~x2 as soft (weight 2)..." << std::endl;
    result = ipamir_solve(solver);

    if (result == 10)
    {
        std::cout << "UNSAT (no models)" << std::endl;
    }
    else if (result == 20 || result == 30)
    {
        if (result == 30)
            std::cout << "OPTIMAL solution found." << std::endl;
        else
            std::cout << "SAT solution found (not necessarily optimal)." << std::endl;

        uint64_t obj = ipamir_val_obj(solver);
        std::cout << "Objective (sum of weights of falsified soft clauses): " << obj << std::endl;

        int x1 = ipamir_val_lit(solver, 1);
        int x2 = ipamir_val_lit(solver, 2);
        std::cout << "Second solution:" << std::endl;
        std::cout << "  x1 = " << (x1 > 0 ? "True" : "False") << std::endl;
        std::cout << "  x2 = " << (x2 > 0 ? "True" : "False") << std::endl;
    }
    else
    {
        std::cout << "Solving was interrupted and no solution was found." << std::endl;
    }

    ipamir_release(solver);

#pragma omp parallel for
    for (int i = 1; i <= 100; i++)
    {
        int tid = omp_get_thread_num();
        printf("The thread %d  executes i = %d\n", tid, i);
    }

    return 0;
}