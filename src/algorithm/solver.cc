#include "algorithm/solver.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

#include "algorithm/base_star.h"
#include "algorithm/construction.h"
#include "algorithm/operators.h"
#include "util/mcmf.h"
#include "util/solution_utils.h"
#include "util/submit.h"
#include "util/utils.h"

namespace alkaid_sd {

void IntraRouteSearch(const Problem &problem, const Config &config,
                      const DistanceMatrix &distance_matrix, Node route_index,
                      Solution &solution, RouteContext &context,
                      Random &random) {
  Repair(distance_matrix, route_index, solution, context);
  std::vector<Node> intra_neighborhoods(config.intra_operators.size());
  std::iota(intra_neighborhoods.begin(), intra_neighborhoods.end(), 0);
  while (true) {
    random.Shuffle(intra_neighborhoods.begin(), intra_neighborhoods.end());
    bool improved = false;
    for (Node neighborhood : intra_neighborhoods) {
      improved = config.intra_operators[neighborhood](
          problem, distance_matrix, route_index, solution, context, random);
      if (improved) {
        break;
      }
    }
    if (!improved) {
      break;
    }
  }
}

void RandomizedVariableNeighborhoodDescent(
    const Problem &problem, const Config &config,
    const DistanceMatrix &distance_matrix, Solution &solution,
    RouteContext &context, Random &random,
    std::vector<std::vector<OperatorCaches>> &operators_caches) {
  static std::vector<std::vector<Node>> routes;
  std::set<Node> associated_routes;
  for (auto tier = 0; tier < config.inter_operators.size(); ++tier) {
    for (auto i = 0; i < config.inter_operators[tier].size(); ++i) {
      operators_caches[tier][i].Init(
          context, kInterOperatorCacheMap.at(config.inter_operators[tier][i]));
    }
  }
  for (Node route_index = 0; route_index < routes.size(); ++route_index) {
    bool same_route = false;
    if (route_index < context.num_routes) {
      same_route = true;
      Node head = context.heads[route_index];
      for (Node node : routes[route_index]) {
        if (head != node) {
          same_route = false;
          break;
        }
        head = solution.successor(head);
      }
      if (head != 0) {
        same_route = false;
      }
    }
    if (!same_route) {
      for (Node i = 0; i < problem.num_customers; ++i) {
        star_caches[route_index].clear();
      }
    }
  }
  for (auto tier = 0; tier < config.inter_operators.size();) {
    std::vector<int> inter_neighborhoods(config.inter_operators[tier].size());
    std::iota(inter_neighborhoods.begin(), inter_neighborhoods.end(), 0);
    random.Shuffle(inter_neighborhoods.begin(), inter_neighborhoods.end());
    bool improved = false;
    for (int neighborhood : inter_neighborhoods) {
      Node original_num_routes = context.num_routes;
      improved = config.inter_operators[tier][neighborhood](
          problem, distance_matrix, solution, context, associated_routes,
          random, operators_caches[tier][neighborhood]);
      if (improved) {
        std::vector<Node> heads;
        for (Node route_index : associated_routes) {
          Node head = context.heads[route_index];
          if (head) {
            heads.emplace_back(head);
          }
          if (route_index < original_num_routes) {
            for (auto &&tier_operator_caches : operators_caches) {
              for (auto &&operator_cache : tier_operator_caches) {
                operator_cache.RemoveRoute(route_index);
              }
            }
            for (Node i = 0; i < problem.num_customers; ++i) {
              star_caches[route_index].clear();
            }
          }
        }
        Node num_routes = 0;
        for (Node route_index = 0; route_index < context.num_routes;
             ++route_index) {
          if (!associated_routes.count(route_index)) {
            MoveRouteContext(num_routes, route_index, context);
            for (auto &&tier_operator_caches : operators_caches) {
              for (auto &&operator_cache : tier_operator_caches) {
                operator_cache.MoveRoute(num_routes, route_index);
              }
            }
            star_caches[num_routes].swap(star_caches[route_index]);
            ++num_routes;
          }
        }
        for (Node head : heads) {
          context.heads[num_routes] = head;
          UpdateRouteContext(solution, num_routes, 0, context);
          IntraRouteSearch(problem, config, distance_matrix, num_routes,
                           solution, context, random);
          for (auto &&tier_operator_caches : operators_caches) {
            for (auto &&operator_cache : tier_operator_caches) {
              operator_cache.AddRoute(num_routes);
            }
          }
          ++num_routes;
        }
        context.num_routes = num_routes;
        associated_routes.clear();
        break;
      }
    }
    if (!improved) {
      tier += 1;
    } else if (tier > 0) {
      tier = 0;
    }
  }
  routes.resize(context.num_routes);
  for (Node route_index = 0; route_index < routes.size(); ++route_index) {
    auto &route = routes[route_index];
    route.clear();
    for (Node node = context.heads[route_index]; node;
         node = solution.successor(node)) {
      route.push_back(node);
    }
  }
}

void Perturb(const Problem &problem, const DistanceMatrix &distance_matrix,
             const Config &config, Solution &solution, RouteContext &context,
             Random &random) {
  CalcRouteContext(solution, context);
  std::vector<Node> customers = config.ruin_method->Ruin(
      problem, distance_matrix, solution, context, random);
  config.sorter.Sort(problem, distance_matrix, customers, random);
  for (Node customer : customers) {
    for (Node route_index = 0; route_index < context.num_routes;
         ++route_index) {
      Node node_index = context.heads[route_index];
      while (node_index) {
        Node successor = solution.successor(node_index);
        if (solution.customer(node_index) == customer) {
          RemoveNode(distance_matrix, node_index, route_index, solution,
                     context);
        }
        node_index = successor;
      }
    }
  }
  std::set<Node> associated_routes;
  for (Node customer : customers) {
    SplitReinsertion(problem, distance_matrix, customer,
                     problem.customers[customer].demand, config.blink_rate,
                     solution, context, associated_routes, random);
    associated_routes.clear();
  }
}

std::unique_ptr<Solution> Solve(const Config &config, const Problem &problem,
                                const DistanceMatrix &distance_matrix) {
  Random random(config.random_seed);
  auto context = std::make_unique<RouteContext>();
  std::vector<std::vector<OperatorCaches>> operators_caches;
  for (auto &&operators : config.inter_operators) {
    operators_caches.emplace_back(operators.size());
  }
  auto best_solution = std::make_unique<Solution>();
  int best_objective = std::numeric_limits<int>::max();
  auto timer = CreateTimer(config.time_limit_type);
  timer->Set(config.time_limit);
  const int kMaxStagnation =
      std::min(5000, static_cast<int>(problem.num_customers) *
                         static_cast<int>(CalcFleetLowerBound(problem)));
  int counter = 0;
  while (!timer->IsTimeOut()) {
    auto solution = Construct(problem, distance_matrix, random);
    int objective = CalcObjective(*solution, distance_matrix);
    int iter_best_objective = objective;
    auto new_solution = std::make_unique<Solution>(*solution);
    auto acceptance_rule = config.acceptance_rule();
    int num_stagnation = 0;
    while (num_stagnation < kMaxStagnation && !timer->IsTimeOut()) {
      ++counter;
      ++num_stagnation;
      CalcRouteContext(*new_solution, *context);
      for (Node i = 0; i < context->num_routes; ++i) {
        IntraRouteSearch(problem, config, distance_matrix, i, *new_solution,
                         *context, random);
      }
      RandomizedVariableNeighborhoodDescent(problem, config, distance_matrix,
                                            *new_solution, *context, random,
                                            operators_caches);
      int new_objective = CalcObjective(*new_solution, distance_matrix);
      if (new_objective < iter_best_objective) {
        num_stagnation = 0;
        iter_best_objective = new_objective;
      }
      if (new_objective < best_objective) {
        Submit(*timer, new_objective);
        best_objective = new_objective;
        *best_solution = *new_solution;
      }
      if (acceptance_rule->Accept(objective, new_objective, random)) {
        objective = new_objective;
        *solution = *new_solution;
      } else {
        *new_solution = *solution;
      }
      Perturb(problem, distance_matrix, config, *new_solution, *context,
              random);
    }
  }
  std::cout << counter << std::endl;
  return best_solution;
}

void Recover(const DistanceMatrix &distance_matrix, Solution &solution, Node i,
             Node j) {
  Node customer =
      distance_matrix
          .previous_node_indices[solution.customer(i)][solution.customer(j)];
  if (customer != -1) {
    Node k = solution.Insert(customer, 0, i, j);
    Recover(distance_matrix, solution, i, k);
    Recover(distance_matrix, solution, k, j);
  }
}

void RestoreByFloyd(const DistanceMatrix &distance_matrix, Solution &solution) {
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node node_index = solution.node_pool()[i];
    if (!solution.predecessor(node_index)) {
      Node predecessor = 0;
      while (node_index) {
        Recover(distance_matrix, solution, predecessor, node_index);
        predecessor = node_index;
        node_index = solution.successor(node_index);
      }
      Recover(distance_matrix, solution, predecessor, 0);
    }
  }
}

} // namespace alkaid_sd
