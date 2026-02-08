#include <limits>
#include <algorithm>
#include <cmath>
#include <iostream>

#include "metric-driven-optimization.hpp"

using namespace std;

MetricProfile::MetricProfile()
{
    metrics.resize(NOF_METRICS, 0.0);
    metrics[MIN_BODY_SIZE] = numeric_limits<double>::max();
    metrics[MIN_SUPPORT_BODY_SIZE] = numeric_limits<double>::max();
    metrics[SUPPORT_WEIGHT_SUM] = numeric_limits<double>::max();
}

<<<<<<< HEAD
unordered_map<Literal, MetricProfile> compute_metric_profiles(const Program &program, const SolvingConfiguration &solving_configuration) == == == =
                                                                                                                                                      unordered_map<Literal, MetricProfile> compute_metric_profiles(const Program &program, const vector<double> &metric_weights)
>>>>>>> 5b59b8b6064a1adc66a4213c8a3f55b620354dd8
{
    unordered_map<Literal, MetricProfile> metric_profiles;

    unordered_set<Literal> literals;
    unordered_set<Atom> atoms;
    unordered_map<Literal, unsigned int> occ;
    unordered_map<Literal, vector<double>> support_weight_sums;
    unordered_map<Atom, unsigned int> avg_body_size_count;
    unordered_map<Atom, unsigned int> avg_support_body_size_count;
    for (const auto &[literal, weight] : program.weights)
    {
        MetricProfile metric_profile;
        Atom atom = literal < 0 ? -literal : literal;
        metric_profiles[literal] = metric_profile;
        literals.insert(literal);
        atoms.insert(atom);
        occ[literal] = 0;
        avg_body_size_count[atom] = 0;
        avg_support_body_size_count[atom] = 0;
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

            // avg_support_body_size
            if (literals.contains(head))
            {
                metric_profiles[head].metrics[AVG_SUPPORT_BODY_SIZE] += body[0];
                avg_support_body_size_count[head]++;
            }
            if (literals.contains(-head))
            {
                metric_profiles[-head].metrics[AVG_SUPPORT_BODY_SIZE] += body[0];
                avg_support_body_size_count[-head]++;
            }

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

                    // avg_body_size
                    if (literals.contains(literal))
                    {
                        metric_profiles[literal].metrics[AVG_BODY_SIZE] += body[0];
                        avg_body_size_count[atom]++;
                    }
                    if (literals.contains(-literal))
                    {
                        metric_profiles[-literal].metrics[AVG_BODY_SIZE] += body[0];
                        avg_body_size_count[atom]++;
                    }

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

    unordered_map<Literal, unordered_set<Literal>> co_occurs;
    unordered_map<Literal, double> constraint_pressures;
    for (const auto &body_index : program.constraints)
    {
        const auto &body = program.bodies[body_index];
        if (body[0] <= 0)
            continue;
        unsigned int body_size = body.size();
        vector<Literal> co_occurring_literals;
        for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        {
            Literal literal = body[literal_index];
            if (literal == 0)
                continue;

            // co_occurs
            if (co_occurring_literals.size() > 0)
            {
                for (const auto &other_literal : co_occurring_literals)
                {
                    if (literal != other_literal)
                    {
                        co_occurs[literal].insert(other_literal);
                        co_occurs[other_literal].insert(literal);
                    }
                }
            }
            co_occurring_literals.push_back(literal);

            // occ_constraint
            if (literals.contains(literal))
                metric_profiles[literal].metrics[OCC_CONSTRAINT]++;
            if (literals.contains(-literal))
                metric_profiles[-literal].metrics[OCC_CONSTRAINT]++;
        }

        // constraint_pressure
        double contrib = 1.0 / co_occurring_literals.size();
        for (const auto &literal : co_occurring_literals)
        {
            if (constraint_pressures.contains(literal))
                constraint_pressures[literal] += contrib;
            else
                constraint_pressures[literal] = contrib;
        }
    }

    for (const auto &[literal, weight] : program.weights)
    {
        // weight_density
        metric_profiles[literal].metrics[WEIGHT_DENSITY] = weight / (occ[literal] + 1);

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

        // avg_body_size
        Atom atom = literal < 0 ? -literal : literal;
        if (avg_body_size_count[atom] > 0)
            metric_profiles[literal].metrics[AVG_BODY_SIZE] /= avg_body_size_count[atom];
        if (avg_support_body_size_count[atom] > 0)
            metric_profiles[literal].metrics[AVG_SUPPORT_BODY_SIZE] /= avg_support_body_size_count[atom];

        // global_impact
        if (co_occurs.contains(literal))
            metric_profiles[literal].metrics[GLOBAL_IMPACT] = co_occurs[literal].size();
        else
            metric_profiles[literal].metrics[GLOBAL_IMPACT] = 0;

        // constraint_pressure
        if (constraint_pressures.contains(literal))
            metric_profiles[literal].metrics[CONSTRAINT_PRESSURE] = constraint_pressures[literal];
        else
            metric_profiles[literal].metrics[CONSTRAINT_PRESSURE] = 0;
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

        if (solving_configuration.debug_metrics && denom != 0.0)
        {
            cout << "% Metric " << metric_idx << " min value = " << min_value << ", max value = " << max_value << ", denom = " << denom << endl;
        }
    }

    for (auto &[literal, profile] : metric_profiles)
    {
        profile.score = 0.0;
        for (int metric_idx = 0; metric_idx < NOF_METRICS; ++metric_idx)
        {
            profile.score += solving_configuration.metric_weights[metric_idx] * profile.metrics[metric_idx];
        }
    }
    double max_score = numeric_limits<double>::lowest();
    double min_score = numeric_limits<double>::max();
    for (const auto &[literal, profile] : metric_profiles)
    {
        if (profile.score > max_score)
            max_score = profile.score;
        if (profile.score < min_score)
            min_score = profile.score;
    }
    double delta = 0;
    for (const auto &weight : solving_configuration.metric_weights)
    {
        delta += abs(weight);
    }
    double rho = (max_score - min_score) / delta;
    unsigned int max_metrics_weight = 0;
    if (rho < 0.05)
        max_metrics_weight = 1;
    else if (rho < 0.15)
        max_metrics_weight = 3;
    else
        max_metrics_weight = 5;

    for (auto &[literal, profile] : metric_profiles)
    {
        if (min_score == max_score)
            profile.bucket_score = 0;
        else
            profile.bucket_score = static_cast<int>(floor(max_metrics_weight * (1 - (profile.score - min_score) / (max_score - min_score))));
    }

    if (solving_configuration.debug_metrics)
    {
        cout << "% Max metrics weight = " << max_metrics_weight << endl;
        throw logic_error("Debug metrics");
    }

    return metric_profiles;
}