#include "construction.h"

#include <limits>
#include <numeric>
#include <utility>
#include <vector>

#include "route_context.h"
#include "utils.h"

namespace alkaidsd {
  enum InsertionCriterion { kMcfic, kNfic };

  enum InsertionStrategies { kSis, kPis };

  using CandidateList = std::vector<std::pair<int, int>>;

  int AddRoute(CandidateList &candidate_list, Random &random, AlkaidSolution &solution,
               RouteContext &context) {
    int position = random.NextInt(0, static_cast<int>(candidate_list.size()) - 1);
    auto [customer, demand] = candidate_list[position];
    Node node_index = solution.Insert(customer, demand, 0, 0);
    candidate_list[position] = candidate_list.back();
    candidate_list.pop_back();
    context.AddRoute(node_index, node_index, demand);
    return position;
  }

  template <class Func> void SequentialInsertion(const Instance &instance, const Func &func,
                                                 CandidateList &candidate_list, Random &random,
                                                 AlkaidSolution &solution, RouteContext &context) {
    InsertionWithCost<float> best_insertion{};
    std::vector<bool> is_full(context.NumRoutes(), false);
    while (!candidate_list.empty()) {
      bool inserted = false;
      for (Node route_index = 0; route_index < context.NumRoutes(); ++route_index) {
        if (is_full[route_index]) {
          continue;
        }
        int candidate_position = -1;
        best_insertion.cost = Delta(std::numeric_limits<float>::max(), -1);
        for (int i = 0; i < static_cast<int>(candidate_list.size()); ++i) {
          auto [customer, demand] = candidate_list[i];
          if (context.Load(route_index) + demand > instance.capacity) {
            continue;
          }
          auto insertion
              = CalcBestInsertion(solution, func, context, route_index, customer, random);
          if (best_insertion.Update(insertion, random)) {
            candidate_position = i;
          }
        }
        if (candidate_position == -1) {
          is_full[route_index] = true;
        } else {
          auto [customer, demand] = candidate_list[candidate_position];
          candidate_list[candidate_position] = candidate_list.back();
          candidate_list.pop_back();
          Node node_index = solution.Insert(customer, demand, best_insertion.predecessor,
                                            best_insertion.successor);
          if (best_insertion.predecessor == 0) {
            context.SetHead(route_index, node_index);
          }
          context.AddLoad(route_index, demand);
          inserted = true;
        }
      }
      if (!inserted) {
        AddRoute(candidate_list, random, solution, context);
        is_full.push_back(false);
      }
    }
  }

  template <class Func> void ParallelInsertion(const Instance &instance, const Func &func,
                                               CandidateList &candidate_list, Random &random,
                                               AlkaidSolution &solution, RouteContext &context) {
    std::vector<std::vector<InsertionWithCost<float>>> best_insertions(candidate_list.size());
    for (int i = 0; i < static_cast<int>(candidate_list.size()); ++i) {
      for (Node j = 0; j < context.NumRoutes(); ++j) {
        best_insertions[i].push_back(
            CalcBestInsertion(solution, func, context, j, candidate_list[i].first, random));
      }
    }
    std::vector<bool> updated(context.NumRoutes(), false);
    InsertionWithCost<float> best_insertion{};
    while (!candidate_list.empty()) {
      int candidate_position = -1;
      best_insertion.cost = Delta(std::numeric_limits<float>::max(), -1);
      for (int i = 0; i < static_cast<int>(best_insertions.size()); ++i) {
        for (int j = 0; j < static_cast<int>(best_insertions[i].size()); ++j) {
          Node route_index = best_insertions[i][j].route_index;
          auto [customer, demand] = candidate_list[i];
          if (context.Load(route_index) + demand > instance.capacity) {
            best_insertions[i][j] = best_insertions[i].back();
            best_insertions[i].pop_back();
            --j;
          } else {
            if (updated[route_index]) {
              best_insertions[i][j]
                  = CalcBestInsertion(solution, func, context, route_index, customer, random);
            }
            if (best_insertion.Update(best_insertions[i][j], random)) {
              candidate_position = i;
            }
          }
        }
      }
      if (candidate_position == -1) {
        int position = AddRoute(candidate_list, random, solution, context);
        best_insertions[position] = std::move(best_insertions.back());
        best_insertions.pop_back();
        for (int i = 0; i < static_cast<int>(candidate_list.size()); ++i) {
          auto [customer, demand] = candidate_list[i];
          best_insertions[i].push_back(CalcBestInsertion(
              solution, func, context, context.NumRoutes() - 1, customer, random));
        }
        updated.push_back(false);
      } else {
        auto [customer, demand] = candidate_list[candidate_position];
        candidate_list[candidate_position] = candidate_list.back();
        candidate_list.pop_back();
        best_insertions[candidate_position] = std::move(best_insertions.back());
        best_insertions.pop_back();
        Node node_index = solution.Insert(customer, demand, best_insertion.predecessor,
                                          best_insertion.successor);
        Node route_index = best_insertion.route_index;
        if (best_insertion.predecessor == 0) {
          context.SetHead(route_index, node_index);
        }
        context.AddLoad(route_index, demand);
        updated[route_index] = true;
      }
    }
  }

  template <class Func> void InsertCandidates(const Instance &instance, const Func &func,
                                              CandidateList &candidate_list, Random &random,
                                              AlkaidSolution &solution, RouteContext &context) {
    int strategy = random.NextInt(0, 1);
    if (strategy == kSis) {
      SequentialInsertion(instance, func, candidate_list, random, solution, context);
    } else {
      ParallelInsertion(instance, func, candidate_list, random, solution, context);
    }
  }

  AlkaidSolution Construct(const Instance &instance, Random &random) {
    CandidateList candidate_list;
    Node num_fleets = CalcFleetLowerBound(instance);
    for (Node i = 1; i < instance.num_customers; ++i) {
      int demand = instance.demands[i];
      while (demand > 0) {
        int split_demand = std::min(demand, instance.capacity);
        candidate_list.emplace_back(i, split_demand);
        demand -= split_demand;
      }
    }
    AlkaidSolution solution;
    RouteContext context;
    for (Node i = 0; i < num_fleets && !candidate_list.empty(); ++i) {
      AddRoute(candidate_list, random, solution, context);
    }
    int criterion = random.NextInt(0, 1);
    if (criterion == kMcfic) {
      float gamma = static_cast<float>(random.NextInt(0, 34)) * 0.05f;
      auto func = [&](Node predecessor, Node successor, Node customer) {
        Node pre_customer = solution.Customer(predecessor);
        Node suc_customer = solution.Customer(successor);
        return static_cast<float>(instance.distance_matrix[pre_customer][customer]
                                  + instance.distance_matrix[customer][suc_customer]
                                  - instance.distance_matrix[pre_customer][suc_customer])
               - 2 * gamma * instance.distance_matrix[0][customer];
      };
      InsertCandidates(instance, func, candidate_list, random, solution, context);
    } else {
      auto func = [&](Node predecessor, [[maybe_unused]] Node successor, Node customer) {
        Node pre_customer = solution.Customer(predecessor);
        if (pre_customer == 0) {
          return std::numeric_limits<float>::max();
        } else {
          return static_cast<float>(instance.distance_matrix[pre_customer][customer]);
        }
      };
      InsertCandidates(instance, func, candidate_list, random, solution, context);
    }
    return solution;
  }
}  // namespace alkaidsd
