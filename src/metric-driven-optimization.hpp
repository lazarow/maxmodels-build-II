#pragma once

#include "internal_representation.hpp"

struct MetricProfile
{
    vector<double> metrics;
    double score;
    int bucket_score;

    MetricProfile();
};

const int NOF_METRICS = 11;
enum Metric
{
    OCC_BODY_POS,
    OCC_BODY_NEG,
    OCC_CONSTRAINT,
    MIN_BODY_SIZE,
    AVG_BODY_SIZE,
    SUPPORT_COUNT,
    MIN_SUPPORT_BODY_SIZE,
    AVG_SUPPORT_BODY_SIZE,
    WEIGHT_DENSITY,
    SUPPORT_WEIGHT_SUM,
    MIN_ALT_COST,
};

unordered_map<Literal, MetricProfile> compute_metric_profiles(const Program &program, const vector<double> &metric_weights, const int max_metrics_weight);

class satisfied_with_metrics_exception : public satisfied_exception
{
public:
    Weight cost;
    satisfied_with_metrics_exception(const Program &program, const Model &stable_model, const unordered_map<Literal, MetricProfile> &metric_profiles) : satisfied_exception(program, stable_model)
    {
        if (program.weights.size() > 0 && metric_profiles.size() > 0)
        {
            unordered_set<Atom> answer_set;
            for (const auto &atom : program.facts)
                answer_set.insert(atom);
            for (const auto &atom : program.required_atoms)
                answer_set.insert(atom);
            for (const auto &atom : stable_model)
                answer_set.insert(atom);
            double total_metric_score = 0;
            for (const auto &[literal, weight] : program.weights)
            {
                if (literal > 0 && answer_set.contains(literal))
                    total_metric_score += metric_profiles.at(literal).bucket_score;
                else if (literal < 0 && answer_set.contains(-literal) == false)
                    total_metric_score += metric_profiles.at(literal).bucket_score;
            }
            cout << "% Metric score in the answer set = " << total_metric_score << endl;
        }
    }
};