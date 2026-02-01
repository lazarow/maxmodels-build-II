#include <limits>
#include <algorithm>
#include <cmath>

#include "metric-driven-optimization.hpp"

using namespace std;

MetricProfile::MetricProfile()
{
    metrics.resize(NOF_METRICS, 0.0);
    metrics[MIN_BODY_SIZE] = numeric_limits<double>::max();
    metrics[MIN_SUPPORT_BODY_SIZE] = numeric_limits<double>::max();
    metrics[SUPPORT_WEIGHT_SUM] = numeric_limits<double>::max();
}

unordered_map<Literal, MetricProfile> compute_metric_profiles(const Program &program, const vector<double> &metric_weights, const int max_metrics_weight)
{
    unordered_map<Literal, MetricProfile> metric_profiles;

    unordered_set<Literal> literals;
    unordered_set<Atom> atoms;
    unordered_map<Literal, unsigned int> occ;
    unordered_map<Literal, vector<double>> support_weight_sums;
    for (const auto &[literal, weight] : program.weights)
    {
        MetricProfile metric_profile;
        metric_profiles[literal] = metric_profile;
        literals.insert(literal);
        atoms.insert(literal < 0 ? -literal : literal);
        occ[literal] = 0;
    }

    for (const auto &[head, body_indices] : program.heads)
    {
        for (const auto &body_index : body_indices)
        {
            const auto &body = program.bodies[body_index];
            if (body[0] <= 0)
                continue;
            unsigned int body_size = body.size();

            // support_count
            if (atoms.contains(head))
            {
                if (literals.contains(head))
                    metric_profiles[head].metrics[SUPPORT_COUNT]++;
                if (literals.contains(-head))
                    metric_profiles[-head].metrics[SUPPORT_COUNT]++;
            }

            // min_support_body_size, note: body[0] denotes the number of undetermined literals
            if (literals.contains(head) && body[0] < metric_profiles[head].metrics[MIN_SUPPORT_BODY_SIZE])
                metric_profiles[head].metrics[MIN_SUPPORT_BODY_SIZE] = body[0];
            if (literals.contains(-head) && body[0] < metric_profiles[-head].metrics[MIN_SUPPORT_BODY_SIZE])
                metric_profiles[-head].metrics[MIN_SUPPORT_BODY_SIZE] = body[0];

            Weight support_weight_sum = 0;

            for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
            {
                Literal literal = body[literal_index];
                if (literal == 0)
                    continue;

                if (program.weights.contains(literal))
                    support_weight_sum += program.weights.at(literal);

                Atom atom = literal < 0 ? -literal : literal;
                if (atoms.contains(atom))
                {
                    // min_body_size
                    if (literals.contains(literal) && body[0] < metric_profiles[literal].metrics[MIN_BODY_SIZE])
                        metric_profiles[literal].metrics[MIN_BODY_SIZE] = body[0];
                    if (literals.contains(-literal) && body[0] < metric_profiles[-literal].metrics[MIN_BODY_SIZE])
                        metric_profiles[-literal].metrics[MIN_BODY_SIZE] = body[0];

                    if (literal > 0)
                    {
                        // occ_body_pos
                        if (literals.contains(literal))
                            metric_profiles[literal].metrics[OCC_BODY_POS]++;
                        if (literals.contains(-literal))
                            metric_profiles[-literal].metrics[OCC_BODY_POS]++;
                    }
                    else
                    {
                        // occ_body_neg
                        if (literals.contains(literal))
                            metric_profiles[literal].metrics[OCC_BODY_NEG]++;
                        if (literals.contains(-literal))
                            metric_profiles[-literal].metrics[OCC_BODY_NEG]++;
                    }

                    // occ
                    if (literals.contains(literal))
                        occ[literal]++;
                }
            }

            // support_weight_sum
            if (literals.contains(head) && support_weight_sum < metric_profiles[head].metrics[SUPPORT_WEIGHT_SUM])
                metric_profiles[head].metrics[SUPPORT_WEIGHT_SUM] = support_weight_sum;
            if (literals.contains(-head) && support_weight_sum < metric_profiles[-head].metrics[SUPPORT_WEIGHT_SUM])
                metric_profiles[-head].metrics[SUPPORT_WEIGHT_SUM] = support_weight_sum;

            if (literals.contains(head))
                support_weight_sums[head].push_back(support_weight_sum);
            if (literals.contains(-head))
                support_weight_sums[-head].push_back(support_weight_sum);
        }
    }

    for (const auto &body_index : program.constraints)
    {
        const auto &body = program.bodies[body_index];
        if (body[0] <= 0)
            continue;
        unsigned int body_size = body.size();
        for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        {
            Literal literal = body[literal_index];
            if (literal == 0)
                continue;
            // occ_constraint
            if (literals.contains(literal))
                metric_profiles[literal].metrics[OCC_CONSTRAINT]++;
            if (literals.contains(-literal))
                metric_profiles[-literal].metrics[OCC_CONSTRAINT]++;
        }
    }

    for (const auto &[literal, weight] : program.weights)
    {
        // weight_density
        metric_profiles[literal].metrics[WEIGHT_DENSITY] = weight / (occ[literal] + 1);

        // alt_support_count
        metric_profiles[literal].metrics[ALT_SUPPORT_COUNT] = max(.0, metric_profiles[literal].metrics[SUPPORT_COUNT] - 1);

        // min_alt_cost
        auto it = find(support_weight_sums[literal].begin(), support_weight_sums[literal].end(), metric_profiles[literal].metrics[SUPPORT_WEIGHT_SUM]);
        if (it != support_weight_sums[literal].end())
            support_weight_sums[literal].erase(it);
        if (support_weight_sums[literal].empty())
        {
            metric_profiles[literal].metrics[MIN_ALT_COST] = numeric_limits<double>::max();
        }
        else
        {
            double min_value = support_weight_sums[literal][0];
            for (const auto &weight : support_weight_sums[literal])
            {
                if (weight < min_value)
                    min_value = weight;
            }
            metric_profiles[literal].metrics[MIN_ALT_COST] = min_value;
        }
    }

    // Normalize all metrics in metric_profiles to 0-1 by (m - min) / (max - min)
    for (int metric_idx = 0; metric_idx < NOF_METRICS; ++metric_idx)
    {
        double min_value = numeric_limits<double>::max();
        double max_value = numeric_limits<double>::lowest();

        for (const auto &[literal, profile] : metric_profiles)
        {
            double value = profile.metrics[metric_idx];
            if (value < min_value)
                min_value = value;
            if (value > max_value)
                max_value = value;
        }
        double denom = max_value - min_value;
        if (denom == 0.0)
        {
            for (auto &[literal, profile] : metric_profiles)
                profile.metrics[metric_idx] = 0.0;
        }
        else
        {
            for (auto &[literal, profile] : metric_profiles)
                profile.metrics[metric_idx] = (profile.metrics[metric_idx] - min_value) / denom;
        }
    }

    for (auto &[literal, profile] : metric_profiles)
    {
        profile.score = 0.0;
        for (int metric_idx = 0; metric_idx < NOF_METRICS; ++metric_idx)
        {
            profile.score += metric_weights[metric_idx] * profile.metrics[metric_idx];
        }
    }
    double max_score = numeric_limits<double>::lowest();
    for (const auto &[literal, profile] : metric_profiles)
    {
        if (profile.score > max_score)
            max_score = profile.score;
    }
    for (auto &[literal, profile] : metric_profiles)
    {
        if (max_score == 0.0)
            profile.bucket_score = 0;
        else
            profile.bucket_score = static_cast<int>(floor(max_metrics_weight * profile.score / max_score));
    }

    return metric_profiles;
}