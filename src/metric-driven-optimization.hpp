#pragma once

#include "internal_representation.hpp"

struct MetricProfile
{
    vector<double> metrics;
    double score;
    int bucket_score;

    MetricProfile();
};

const int NOF_METRICS = 10;
enum Metric
{
    OCC_BODY_POS,
    OCC_BODY_NEG,
    OCC_CONSTRAINT,
    MIN_BODY_SIZE,
    SUPPORT_COUNT,
    MIN_SUPPORT_BODY_SIZE,
    ALT_SUPPORT_COUNT,
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
        (void)metric_profiles;
    }
};