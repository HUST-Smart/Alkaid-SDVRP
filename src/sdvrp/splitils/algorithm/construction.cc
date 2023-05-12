#include "sdvrp/splitils/algorithm/construction.h"

#include <limits>
#include <utility>
#include <vector>

#include "sdvrp/splitils/model/route_context.h"
#include "sdvrp/splitils/util/utils.h"

namespace vrp::sdvrp::splitils {

enum InsertionCriterion { kMcfic, kNfic };

enum InsertionStrategies { kSis, kPis };

using CandidateList = std::vector<Node>;

int AddRoute(const Problem &problem, CandidateList &candidate_list,
             Random &random, Solution &solution, RouteContext &context) {
  int position = random.NextInt(0, static_cast<int>(candidate_list.size()) - 1);
  Node customer = candidate_list[position];
  Node node_index =
      solution.Insert(customer, problem.customers[customer].demand, 0, 0);
  candidate_list[position] = candidate_list.back();
  candidate_list.pop_back();
  context.heads[context.num_routes] = node_index;
  context.route_loads[context.num_routes] = problem.customers[customer].demand;
  ++context.num_routes;
  return position;
}

template <class Func>
void SequentialInsertion(const Problem &problem, const Func &func,
                         CandidateList &candidate_list, Random &random,
                         Solution &solution, RouteContext &context) {
  InsertionWithCost<float> best_insertion{};
  std::vector<bool> is_full(context.num_routes, false);
  while (!candidate_list.empty()) {
    bool inserted = false;
    for (Node route_index = 0; route_index < context.num_routes;
         ++route_index) {
      if (is_full[route_index]) {
        continue;
      }
      int candidate_position = -1;
      best_insertion.cost = Delta(std::numeric_limits<float>::max(), -1);
      for (int i = 0; i < static_cast<int>(candidate_list.size()); ++i) {
        Node customer = candidate_list[i];
        int demand = problem.customers[customer].demand;
        if (context.route_loads[route_index] + demand > problem.capacity) {
          continue;
        }
        auto insertion = CalcBestInsertion(solution, func, context, route_index,
                                           customer, random);
        if (best_insertion.Update(insertion, random)) {
          candidate_position = i;
        }
      }
      if (candidate_position == -1) {
        is_full[route_index] = true;
      } else {
        Node customer = candidate_list[candidate_position];
        candidate_list[candidate_position] = candidate_list.back();
        candidate_list.pop_back();
        int demand = problem.customers[customer].demand;
        Node node_index =
            solution.Insert(customer, demand, best_insertion.predecessor,
                            best_insertion.successor);
        if (best_insertion.predecessor == 0) {
          context.heads[route_index] = node_index;
        }
        context.route_loads[route_index] += demand;
        inserted = true;
      }
    }
    if (!inserted) {
      AddRoute(problem, candidate_list, random, solution, context);
      is_full.push_back(false);
    }
  }
}

template <class Func>
void ParallelInsertion(const Problem &problem, const Func &func,
                       CandidateList &candidate_list, Random &random,
                       Solution &solution, RouteContext &context) {
  std::vector<std::vector<InsertionWithCost<float>>> best_insertions(
      candidate_list.size());
  for (int i = 0; i < static_cast<int>(candidate_list.size()); ++i) {
    for (Node j = 0; j < context.num_routes; ++j) {
      best_insertions[i].push_back(CalcBestInsertion(
          solution, func, context, j, candidate_list[i], random));
    }
  }
  std::vector<bool> updated(context.num_routes, false);
  InsertionWithCost<float> best_insertion{};
  while (!candidate_list.empty()) {
    int candidate_position = -1;
    best_insertion.cost = Delta(std::numeric_limits<float>::max(), -1);
    for (int i = 0; i < static_cast<int>(best_insertions.size()); ++i) {
      for (int j = 0; j < static_cast<int>(best_insertions[i].size()); ++j) {
        Node route_index = best_insertions[i][j].route_index;
        int demand = problem.customers[candidate_list[i]].demand;
        if (context.route_loads[route_index] + demand > problem.capacity) {
          best_insertions[i][j] = best_insertions[i].back();
          best_insertions[i].pop_back();
          --j;
        } else {
          if (updated[route_index]) {
            best_insertions[i][j] =
                CalcBestInsertion(solution, func, context, route_index,
                                  candidate_list[i], random);
          }
          if (best_insertion.Update(best_insertions[i][j], random)) {
            candidate_position = i;
          }
        }
      }
    }
    if (candidate_position == -1) {
      int position =
          AddRoute(problem, candidate_list, random, solution, context);
      best_insertions[position] = std::move(best_insertions.back());
      best_insertions.pop_back();
      for (int i = 0; i < static_cast<int>(candidate_list.size()); ++i) {
        Node customer = candidate_list[i];
        best_insertions[i].push_back(CalcBestInsertion(
            solution, func, context, context.num_routes - 1, customer, random));
      }
      updated.push_back(false);
    } else {
      Node customer = candidate_list[candidate_position];
      candidate_list[candidate_position] = candidate_list.back();
      candidate_list.pop_back();
      best_insertions[candidate_position] = std::move(best_insertions.back());
      best_insertions.pop_back();
      int demand = problem.customers[customer].demand;
      Node node_index =
          solution.Insert(customer, demand, best_insertion.predecessor,
                          best_insertion.successor);
      Node route_index = best_insertion.route_index;
      if (best_insertion.predecessor == 0) {
        context.heads[route_index] = node_index;
      }
      context.route_loads[route_index] += demand;
      updated[route_index] = true;
    }
  }
}

template <class Func>
void InsertCandidates(const Problem &problem, const Func &func,
                      CandidateList &candidate_list, Random &random,
                      Solution &solution, RouteContext &context) {
  int strategy = random.NextInt(0, 1);
  if (strategy == kSis) {
    SequentialInsertion(problem, func, candidate_list, random, solution,
                        context);
  } else {
    ParallelInsertion(problem, func, candidate_list, random, solution, context);
  }
}

std::unique_ptr<Solution> Construct(const Problem &problem,
                                    const DistanceMatrix &distance_matrix,
                                    Random &random) {
  std::vector<Node> candidate_list(problem.num_customers - 1);
  std::iota(candidate_list.begin(), candidate_list.end(), 1);
  Node num_fleets = CalcFleetLowerBound(problem);
  auto solution = std::make_unique<Solution>();
  auto context = std::make_unique<RouteContext>();
  for (Node i = 0; i < num_fleets; ++i) {
    AddRoute(problem, candidate_list, random, *solution, *context);
  }
  int criterion = random.NextInt(0, 1);
  if (criterion == kMcfic) {
    float gamma = static_cast<float>(random.NextInt(0, 34)) * 0.05f;
    auto func = [&](Node predecessor, Node successor, Node customer) {
      Node pre_customer = solution->customer(predecessor);
      Node suc_customer = solution->customer(successor);
      return static_cast<float>(distance_matrix[pre_customer][customer] +
                                distance_matrix[customer][suc_customer] -
                                distance_matrix[pre_customer][suc_customer]) -
             2 * gamma * distance_matrix[0][customer];
    };
    InsertCandidates(problem, func, candidate_list, random, *solution,
                     *context);
  } else {
    auto func = [&](Node predecessor, Node successor, Node customer) {
      Node pre_customer = solution->customer(predecessor);
      if (pre_customer == 0) {
        return std::numeric_limits<float>::max();
      } else {
        return static_cast<float>(distance_matrix[pre_customer][customer]);
      }
    };
    InsertCandidates(problem, func, candidate_list, random, *solution,
                     *context);
  }
  return solution;
}

} // namespace vrp::sdvrp::splitils
