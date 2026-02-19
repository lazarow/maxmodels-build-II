#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>
#include <chrono>
#include "solving.hpp"
#include "stable_model.hpp"
#include "loop_formulas.hpp"
#include "wcnf.hpp"
#include "encoding.hpp"

using namespace std;
using namespace chrono;

void solve(const Program &program, SolvingConfiguration &solving_configuration, SolvingBenchmark &benchmark)
{
    auto start_time = high_resolution_clock::now();

    unique_ptr<WCNF> wcnf = make_unique<ExternalSolverWrapperWCNF>();
    wcnf->init();

    AtomMapper atom_mapper;
    unordered_map<BodyIndex, unsigned int> body_to_variable;

    // Cost Clauses Encoding
    vector<vector<Literal>> cost_conflict_clauses;
    unordered_set<Literal> cost_conflict_literals;

    // #region Encoding
    if (program.extended_atoms.empty() == false)
        lp2sat_like(program, wcnf, body_to_variable, atom_mapper);
    else
        clark_completion(program, wcnf, body_to_variable, atom_mapper);
    // #endregion

    // #region Constraints

    // If the program has weights, we need to encode cost conflict encoding if it is not already enabled.
    solving_configuration.cost_conflict_encoding = solving_configuration.cost_conflict_encoding && program.weights.size() > 0;

    for (const auto &body_index : program.constraints)
    {
        const auto &body = program.bodies[body_index];
        if (body[0] < 0)
            throw logic_error("A constraint should be removed during the simplification.");
        else if (body[0] == 0)
            throw logic_error("A constraint is satisfied.");

        unsigned int body_size = body.size();
        bool all_weighted = true;
        vector<Literal> constraint_cost_conflict_literals;
        for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        {
            // Skip if a literal is determined.
            if (body[literal_index] != 0)
            {
                if (all_weighted && program.weights.contains(-body[literal_index]))
                    constraint_cost_conflict_literals.push_back(body[literal_index]);
                else
                    all_weighted = false;
                wcnf->add_hard(-atom_mapper.get_variable(body[literal_index]));
            }
        }
        wcnf->add_hard(0);

        if (solving_configuration.cost_conflict_encoding && all_weighted && constraint_cost_conflict_literals.size() > 0)
        {
            vector<Literal> cost_conflict_clause;
            for (const auto &literal : constraint_cost_conflict_literals)
            {
                if (cost_conflict_literals.contains(-literal) == false)
                {
                    Atom atom = literal < 0 ? -literal : literal;
                    if (program.heads.contains(atom) && program.required_atoms.contains(atom) == false)
                        cost_conflict_literals.insert(-literal);
                    else
                    {
                        cost_conflict_clause.clear();
                        break;
                    }
                }
                cost_conflict_clause.push_back(literal);
            }
            if (cost_conflict_clause.size() > 0)
                cost_conflict_clauses.push_back(cost_conflict_clause);
        }
    }
    // #endregion

    // #region Soft Clauses
    if (program.weights.size() > 0)
    {

        // if (cost_conflict_clauses.empty() == false)
        //  vector<vector<Literal>> filtered_cost_conflict_clauses;
        //  unordered_set<Literal> filtered_cost_conflict_literals;
        //  if (solving_configuration.cost_conflict_encoding && cost_conflict_clauses.size() > 0)
        //  {
        //      vector<unsigned int> cost_conflict_clause_overlaps;
        //      cost_conflict_clause_overlaps.reserve(cost_conflict_clauses.size());
        //      for (unsigned int i = 0; i < cost_conflict_clauses.size(); i++)
        //      {
        //          const auto &clause_i = cost_conflict_clauses[i];
        //          unordered_set<Literal> literals_i(clause_i.begin(), clause_i.end());
        //          unsigned int overlap = 0;
        //          for (const auto &body_index : program.constraints)
        //          {
        //              const auto &body = program.bodies[body_index];
        //              unordered_set<Literal> body_literals;
        //              unsigned int body_size = body.size();
        //              for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        //              {
        //                  if (body[literal_index] != 0)
        //                      body_literals.insert(body[literal_index]);
        //              }
        //              bool shares_literal = false;
        //              for (const auto &literal : literals_i)
        //              {
        //                  if (body_literals.contains(literal))
        //                  {
        //                      shares_literal = true;
        //                      break;
        //                  }
        //              }
        //              if (shares_literal)
        //                  overlap++;
        //          }
        //          cost_conflict_clause_overlaps.push_back(overlap - 1); // -1 to exclude self-overlap
        //      }
        //      vector<pair<unsigned int, unsigned int>> overlap_sorted_indices;
        //      for (unsigned int i = 0; i < cost_conflict_clauses.size(); i++)
        //      {
        //          overlap_sorted_indices.push_back({cost_conflict_clause_overlaps[i], i});
        //      }
        //      sort(overlap_sorted_indices.begin(), overlap_sorted_indices.end(),
        //           [](const auto &a, const auto &b)
        //           { return a.first > b.first; });

        //     unsigned int top_count = max(1u, static_cast<unsigned int>(cost_conflict_clauses.size() * 0.2));
        //     cout << "% Overlap-based selection: top " << top_count << " of " << cost_conflict_clauses.size() << " cost conflict clauses (20%)" << endl;
        //     for (unsigned int k = 0; k < top_count; k++)
        //     {
        //         unsigned int i = overlap_sorted_indices[k].second;
        //         for (const auto &literal : cost_conflict_clauses[i])
        //         {
        //             filtered_cost_conflict_literals.insert(-literal);
        //         }
        //         filtered_cost_conflict_clauses.push_back(cost_conflict_clauses[i]);
        //     }

        //     // Statistics: Number of soft literals, number of constraints with >=2 soft literals, average length of those constraints, average overlap between such constraints

        //     // 1. Get the set of all soft literals
        //     unordered_set<Literal> soft_literals;
        //     for (const auto &[literal, weight] : program.weights)
        //     {
        //         Atom atom = literal < 0 ? -literal : literal;
        //         if (
        //             program.heads.contains(atom) && !program.required_atoms.contains(atom))
        //         {
        //             soft_literals.insert(literal);
        //         }
        //     }
        //     size_t S = soft_literals.size();

        //     // 2. Identify constraints with any soft literal(s) and collect stats
        //     vector<vector<Literal>> constraints_with_soft_literals;
        //     size_t sum_length = 0;
        //     for (const auto &body_index : program.constraints)
        //     {
        //         const auto &body = program.bodies[body_index];
        //         unordered_set<Literal> body_soft_literals;
        //         unsigned int body_size = body.size();

        //         for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        //         {
        //             if (body[literal_index] != 0 && soft_literals.contains(-body[literal_index]))
        //             {
        //                 body_soft_literals.insert(body[literal_index]);
        //             }
        //         }

        //         if (!body_soft_literals.empty())
        //         {
        //             // Save constraint's literals for later overlap calculation
        //             vector<Literal> body_literals;
        //             for (unsigned int literal_index = 1; literal_index < body_size; literal_index++)
        //             {
        //                 if (body[literal_index] != 0)
        //                     body_literals.push_back(body[literal_index]);
        //             }
        //             constraints_with_soft_literals.push_back(body_literals);
        //             sum_length += body_literals.size();
        //         }
        //     }
        //     size_t C_soft = constraints_with_soft_literals.size();

        //     // 3. Compute average length of such constraints
        //     double avg_length = C_soft > 0 ? static_cast<double>(sum_length) / C_soft : 0.0;

        //     // 4. Compute average overlap between such constraints (pairwise intersection size)
        //     double avg_overlap = 0.0;
        //     size_t overlap_pairs = 0;
        //     size_t overlap_sum = 0;
        //     for (size_t i = 0; i < constraints_with_soft_literals.size(); i++)
        //     {
        //         const auto &ci = constraints_with_soft_literals[i];
        //         unordered_set<Literal> ci_set(ci.begin(), ci.end());
        //         for (size_t j = i + 1; j < constraints_with_soft_literals.size(); j++)
        //         {
        //             const auto &cj = constraints_with_soft_literals[j];
        //             size_t overlap = 0;
        //             for (const auto &lit : cj)
        //             {
        //                 if (ci_set.count(lit))
        //                     overlap++;
        //             }
        //             overlap_sum += overlap;
        //             overlap_pairs++;
        //         }
        //     }
        //     if (overlap_pairs > 0)
        //         avg_overlap = static_cast<double>(overlap_sum) / overlap_pairs;

        //     // 5. Output statistics
        //     cout << "% |S| (number of soft literals): " << S << endl;
        //     cout << "% |C_soft| (number of constraints with >=1 soft literal): " << C_soft << endl;
        //     cout << "% avg_length (average length of such constraints): " << avg_length << endl;
        //     cout << "% avg_overlap (average overlap between such constraints): " << avg_overlap << endl;

        //     // INSERT_YOUR_CODE

        //     // Build the soft-literal graph

        //     // 1. Map literals to node indices for denser storage, maintain reverse as well
        //     unordered_map<Literal, size_t> literal_to_node;
        //     vector<Literal> node_to_literal;
        //     for (const auto &lit : soft_literals)
        //     {
        //         literal_to_node[-lit] = node_to_literal.size();
        //         node_to_literal.push_back(-lit);
        //     }
        //     size_t N = node_to_literal.size(); // Number of nodes

        //     // 2. Build adjacency list: undirected
        //     vector<unordered_set<size_t>> adj(N); // Use set to avoid multi-edges

        //     for (const auto &clause : constraints_with_soft_literals)
        //     {
        //         // Add edges between all pairs in the clause
        //         for (size_t i = 0; i < clause.size(); ++i)
        //         {
        //             auto it_i = literal_to_node.find(clause[i]);
        //             if (it_i == literal_to_node.end())
        //                 continue;
        //             size_t u = it_i->second;
        //             for (size_t j = i + 1; j < clause.size(); ++j)
        //             {
        //                 auto it_j = literal_to_node.find(clause[j]);
        //                 if (it_j == literal_to_node.end())
        //                     continue;
        //                 size_t v = it_j->second;
        //                 if (u != v)
        //                 {
        //                     adj[u].insert(v);
        //                     adj[v].insert(u);
        //                 }
        //             }
        //         }
        //     }

        //     // 3. Compute degree distribution and statistics
        //     size_t sum_deg = 0;
        //     unordered_map<size_t, size_t> degree_distribution; // degree -> how many nodes
        //     vector<size_t> degrees(N, 0);
        //     for (size_t i = 0; i < N; ++i)
        //     {
        //         size_t deg = adj[i].size();
        //         degrees[i] = deg;
        //         sum_deg += deg;
        //         degree_distribution[deg]++;
        //     }
        //     double avg_deg = N > 0 ? static_cast<double>(sum_deg) / N : 0.0;

        //     // 4. Count connected components (plain BFS/DFS)
        //     vector<bool> visited(N, false);
        //     size_t n_components = 0;
        //     for (size_t i = 0; i < N; ++i)
        //     {
        //         if (!visited[i])
        //         {
        //             n_components++;
        //             // BFS or DFS
        //             vector<size_t> q;
        //             q.push_back(i);
        //             visited[i] = true;
        //             while (!q.empty())
        //             {
        //                 size_t u = q.back();
        //                 q.pop_back();
        //                 for (size_t v : adj[u])
        //                 {
        //                     if (!visited[v])
        //                     {
        //                         visited[v] = true;
        //                         q.push_back(v);
        //                     }
        //                 }
        //             }
        //         }
        //     }

        //     // 5. Density: d = (2*|E|)/(|V|*(|V|-1)) for undirected
        //     size_t num_edges = 0;
        //     for (size_t i = 0; i < N; ++i)
        //         num_edges += adj[i].size();
        //     num_edges /= 2; // Each edge counted twice

        //     double density = (N > 1) ? (2.0 * num_edges) / (N * (N - 1)) : 0.0;

        //     // 6. Output all statistics
        //     cout << "% Soft-literal graph: |V|=" << N << " |E|=" << num_edges << endl;
        //     cout << "% average degree: " << avg_deg << endl;
        //     cout << "% number of connected components: " << n_components << endl;
        //     cout << "% degree distribution:" << endl;
        //     for (const auto &[deg, cnt] : degree_distribution)
        //     {
        //         cout << "%   degree " << deg << ": " << cnt << endl;
        //     }
        //     cout << "% density: " << density << endl;
        // }

        unordered_map<Literal, unsigned int> relaxation_var;
        unsigned int nof_cost_conflict_clauses = 0;
        unsigned int nof_cost_conflict_literals = 0;
        unsigned int nof_soft_clauses = 0;
        for (const auto &[literal, weight] : program.weights)
        {
            Atom atom = literal < 0 ? -literal : literal;
            if (
                program.heads.contains(atom) && program.required_atoms.contains(atom) == false)
            {
                if (solving_configuration.cost_conflict_encoding && cost_conflict_literals.contains(literal))
                {
                    unsigned int atom_var = atom_mapper.get_variable(literal);
                    unsigned int r = atom_mapper.get_next_variable();
                    relaxation_var[literal] = r;
                    wcnf->add_hard(-atom_var);
                    wcnf->add_hard(r);
                    wcnf->add_hard(0);
                    wcnf->add_soft(-r, weight);
                    nof_cost_conflict_literals++;
                }
                else
                {
                    wcnf->add_soft(-atom_mapper.get_variable(literal), weight);
                }
                nof_soft_clauses++;
            }
        }
        if (solving_configuration.cost_conflict_encoding)
        {
            for (const auto &cost_conflict_clause : cost_conflict_clauses)
            {
                for (const auto &literal : cost_conflict_clause)
                {
                    wcnf->add_hard(relaxation_var[-literal]);
                }
                wcnf->add_hard(0);
                nof_cost_conflict_clauses++;
            }
            if (cost_conflict_clauses.empty() == false)
            {
                cout << "% The number of cost conflict literals: " << nof_cost_conflict_literals << endl;
                cout << "% The number of cost conflict clauses: " << nof_cost_conflict_clauses << endl;
            }
            // throw logic_error("Stop here");
        }
    }
    // #endregion

    // #region Step 4: Solving
    unsigned int nof_iterations = 0;
    while (true)
    {
        int32_t result = wcnf->solve(solving_configuration);
        nof_iterations++;
        if (result == 10)
        {
            benchmark.time = duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count() / 1000.0;
            throw unsatisfied_exception("The program is unsatisfied.");
        }
        else if (result == 20 || result == 30)
        {
            Model supporting_model;
            for (const auto &[head, body_indices] : program.heads)
            {
                if (
                    program.required_atoms.contains(head) || wcnf->val_lit(atom_mapper.get_variable(head)) > 0)
                {
                    supporting_model.insert(head);
                }
            }

            /**
             * When using level ranking (extended_atoms), the solver returns a stable model
             * in a single call, so we skip loop formula checking.
             */
            if (program.extended_atoms.empty() == false)
            {
                benchmark.time = duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count() / 1000.0;
                throw satisfied_exception(program, supporting_model);
            }

            Model consequences = compute_consequences(program, supporting_model);
            Model M_minus;
            for (Atom atom : supporting_model)
                if (consequences.contains(atom) == false)
                    M_minus.insert(atom);
            if (M_minus.empty())
            {
                benchmark.time = duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count() / 1000.0;
                throw satisfied_exception(program, supporting_model);
            }
            else
            {
                vector<Model> loop_formulas;
                loop_formulas = compute_maximal_loop_formulas(program, M_minus);
                for (const auto &loop_formula : loop_formulas)
                {
                    for (Atom atom : loop_formula)
                    {
                        for (BodyIndex body_index : program.heads.at(atom))
                        {
                            const Body &body = program.bodies[body_index];
                            if (body[0] < 0)
                                continue;
                            bool touches_loop = false;
                            for (unsigned int literal_index = 1; literal_index < body.size(); literal_index++)
                            {
                                if (loop_formula.contains(abs(body[literal_index])))
                                {
                                    touches_loop = true;
                                    break;
                                }
                            }
                            if (touches_loop == false)
                            {
                                wcnf->add_hard(body_to_variable[body_index]);
                            }
                        }
                        wcnf->add_hard(-atom_mapper.get_variable(atom));
                    }
                    wcnf->add_hard(0);
                }
                cout << "% Iteration " << nof_iterations << " completed. " << loop_formulas.size() << " loops found." << endl;
                benchmark.nof_iterations++;
            }
        }
        else
            throw logic_error("Unknown result of the solver.");
    }
    // #endregion
    wcnf->clear();
}

unsigned int AtomMapper::get_next_variable()
{
    return current_variable++;
}

unsigned int AtomMapper::get_variable(Literal literal)
{
    Atom atom = literal < 0 ? -literal : literal;
    if (atom_to_variable.contains(atom) == false)
    {
        atom_to_variable[atom] = get_next_variable();
    }
    return literal < 0 ? -atom_to_variable.at(atom) : atom_to_variable.at(atom);
}
