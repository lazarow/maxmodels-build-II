#ifndef GIT_VERSION
#define GIT_VERSION "{placeholder-git-version}"
#endif

#ifndef EXTERNAL_SOLVER_PATH
#define EXTERNAL_SOLVER_PATH ""
#endif

#include <iostream>
#include <random>

#include "argparse.h"
#include "internal_representation.hpp"
#include "simplification.hpp"
#include "solving.hpp"
#include "metric-driven-optimization.hpp"

using namespace std;

const string VERSION = "% maxmodels (build II) " + string(GIT_VERSION);
const string COPYRIGHT = "% Copyright (c) 2026, Arkadiusz Nowakowski and Wojciech Wieczorek";

int main(int argc, char *argv[])
{
    SolvingBenchmark benchmark;
    bool print_benchmark = false;

    try
    {
        argparse::Parser parser;
        auto simplification = parser.AddFlag("simplify", "Simplify the program");
        auto solving = parser.AddFlag("solve", "Encode and solve the program");
        auto external_solver_path = parser.AddArg<string>("external-solver", "The path to an external solver").Default("");
        auto use_metrics = parser.AddFlag("use-metrics", "Use metric-driven optimization");
        auto max_metrics_weight = parser.AddArg<int>("max-metrics-weight", "The maximum metrics weight").Default(5);
        auto metric_weights = parser.AddArg<string>("metric-weights", "The weights of the metrics").Default("");
        auto random_metric_weights = parser.AddFlag("use-random-metric-weights", "Generate random metric weights in the range -2 to 2");
        auto benchmark_flag = parser.AddFlag("benchmark", "Benchmark the program");
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
            cout << "% Use metrics = " << (solving_configuration.use_metrics ? "true" : "false") << endl;
            cout << "% Has level ranking = " << (program.extended_atoms.empty() == false ? "true" : "false") << endl;
            solving_configuration.max_metrics_weight = *max_metrics_weight;
            cout << "% Maximum metrics weight = " << solving_configuration.max_metrics_weight << endl;

            vector<double> metric_weights_vector;
            metric_weights_vector.reserve(NOF_METRICS);
            stringstream ss(*metric_weights);
            string weight;
            while (getline(ss, weight, ','))
                metric_weights_vector.push_back(stod(weight));
            for (size_t i = 0; i < metric_weights_vector.size(); i++)
                solving_configuration.metric_weights[i] = metric_weights_vector[i];
            if (solving_configuration.metric_weights.size() != NOF_METRICS)
                throw runtime_error("The number of metric weights is not equal to the number of metrics. Expected " + to_string(NOF_METRICS) + " weights, got " + to_string(solving_configuration.metric_weights.size()));

            // Generate random metric weights in the range -2 to 2
            if (*random_metric_weights)
            {
                random_device rd;
                mt19937 gen(rd());
                uniform_real_distribution<double> dist(-2.0, 2.0);
                solving_configuration.metric_weights.resize(NOF_METRICS);
                for (size_t i = 0; i < NOF_METRICS; ++i)
                {
                    solving_configuration.metric_weights[i] = dist(gen);
                }
            }

            cout << "% Metric weights = ";
            for (const auto &weight : solving_configuration.metric_weights)
                cout << weight << " ";
            cout << endl;

            print_benchmark = *benchmark_flag;

            solve(program, solving_configuration, benchmark);
        }
        return 0;
    }
    catch (const unsatisfied_exception &error)
    {
        if (print_benchmark)
        {
            cerr << fixed;
            cerr.precision(3);
            cerr << benchmark.time << " " << benchmark.nof_iterations << "\t";
        }
        return 0;
    }
    catch (const satisfied_exception &error)
    {
        if (print_benchmark)
        {
            cerr << fixed;
            cerr.precision(3);
            cerr << benchmark.time << " " << benchmark.nof_iterations << " " << error.cost << "\t";
        }
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