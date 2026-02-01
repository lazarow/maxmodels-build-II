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
    SUPPORT_COUNT,
    MIN_SUPPORT_BODY_SIZE,
    WEIGHT,
    WEIGHT_DENSITY,
    SUPPORT_WEIGHT_SUM,
    ALT_SUPPORT_COUNT,
    MIN_ALT_COST,
};

unordered_map<Literal, MetricProfile> compute_metric_profiles(const Program &program, const vector<double> &metric_weights);