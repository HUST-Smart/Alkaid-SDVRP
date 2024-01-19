#include <alkaidsd/solver.h>

#include <algorithm>
#include <chrono>
#include <limits>
#include <numeric>
#include <vector>

#include "cache.h"
#include "construction.h"
#include "repair.h"
#include "split_reinsertion.h"
#include "utils.h"

namespace alkaidsd {
  void IntraRouteSearch(const Problem &problem, const Config &config, Node route_index,
                        Solution &solution, RouteContext &context, Random &random) {
    Repair(problem, route_index, solution, context);
    std::vector<Node> intra_neighborhoods(config.intra_operators.size());
    std::iota(intra_neighborhoods.begin(), intra_neighborhoods.end(), 0);
    while (true) {
      random.Shuffle(intra_neighborhoods.begin(), intra_neighborhoods.end());
      bool improved = false;
      for (Node neighborhood : intra_neighborhoods) {
        improved = (*config.intra_operators[neighborhood])(problem, route_index, solution, context,
                                                           random);
        if (improved) {
          break;
        }
      }
      if (!improved) {
        break;
      }
    }
  }

  void RandomizedVariableNeighborhoodDescent(const Problem &problem, const Config &config,
                                             Solution &solution, RouteContext &context,
                                             Random &random, CacheMap &cache_map) {
    cache_map.Reset(solution, context);
    while (true) {
      std::vector<int> inter_neighborhoods(config.inter_operators.size());
      std::iota(inter_neighborhoods.begin(), inter_neighborhoods.end(), 0);
      random.Shuffle(inter_neighborhoods.begin(), inter_neighborhoods.end());
      bool improved = false;
      for (int neighborhood : inter_neighborhoods) {
        Node original_num_routes = context.NumRoutes();
        auto routes = (*config.inter_operators[neighborhood])(problem, solution, context, random,
                                                              cache_map);
        if (!routes.empty()) {
          std::sort(routes.begin(), routes.end());
          improved = true;
          std::vector<Node> heads;
          for (Node route_index : routes) {
            Node head = context.Head(route_index);
            if (head) {
              heads.emplace_back(head);
            }
            if (route_index < original_num_routes) {
              cache_map.RemoveRoute(route_index);
            }
          }
          Node num_routes = 0;
          for (Node route_index = 0; route_index < context.NumRoutes(); ++route_index) {
            if (std::find(routes.begin(), routes.end(), route_index) == routes.end()) {
              context.MoveRouteContext(num_routes, route_index);
              cache_map.MoveRoute(num_routes, route_index);
              ++num_routes;
            }
          }
          for (Node head : heads) {
            context.SetHead(num_routes, head);
            context.UpdateRouteContext(solution, num_routes, 0);
            cache_map.AddRoute(num_routes);
            IntraRouteSearch(problem, config, num_routes, solution, context, random);
            ++num_routes;
          }
          context.SetNumRoutes(num_routes);
          break;
        }
      }
      if (!improved) {
        break;
      }
    }
    cache_map.Save(solution, context);
  }

  void Perturb(const Problem &problem, const Config &config, Solution &solution,
               RouteContext &context, Random &random) {
    context.CalcRouteContext(solution);
    std::vector<Node> customers = config.ruin_method->Ruin(problem, solution, context, random);
    config.sorter.Sort(problem, customers, random);
    for (Node customer : customers) {
      for (Node route_index = 0; route_index < context.NumRoutes(); ++route_index) {
        Node node_index = context.Head(route_index);
        while (node_index) {
          Node successor = solution.Successor(node_index);
          if (solution.Customer(node_index) == customer) {
            Node predecessor = solution.Predecessor(node_index);
            solution.Remove(node_index);
            if (predecessor == 0) {
              context.SetHead(route_index, successor);
            }
            context.UpdateRouteContext(solution, route_index, predecessor);
          }
          node_index = successor;
        }
      }
    }
    for (Node customer : customers) {
      SplitReinsertion(problem, customer, problem.demands[customer], config.blink_rate, solution,
                       context, random);
    }
  }

  double ElapsedTime(std::chrono::time_point<std::chrono::high_resolution_clock> start_time) {
    return std::chrono::duration_cast<std::chrono::duration<double>>(
               std::chrono::high_resolution_clock::now() - start_time)
        .count();
  }

  Solution Solve(const Config &config, const Problem &problem) {
    if (config.listener != nullptr) {
      config.listener->OnStart();
    }
    Random random(config.random_seed);
    RouteContext context;
    CacheMap cache_map;
    Solution best_solution;
    int best_objective = std::numeric_limits<int>::max();
    auto start_time = std::chrono::high_resolution_clock::now();
    const int kMaxStagnation = std::min(5000, static_cast<int>(problem.num_customers)
                                                  * static_cast<int>(CalcFleetLowerBound(problem)));
    while (ElapsedTime(start_time) < config.time_limit) {
      auto solution = Construct(problem, random);
      int objective = solution.CalcObjective(problem);
      int iter_best_objective = objective;
      auto new_solution = solution;
      auto acceptance_rule = config.acceptance_rule();
      int num_stagnation = 0;
      while (num_stagnation < kMaxStagnation && ElapsedTime(start_time) < config.time_limit) {
        ++num_stagnation;
        context.CalcRouteContext(new_solution);
        for (Node i = 0; i < context.NumRoutes(); ++i) {
          IntraRouteSearch(problem, config, i, new_solution, context, random);
        }
        RandomizedVariableNeighborhoodDescent(problem, config, new_solution, context, random,
                                              cache_map);
        int new_objective = new_solution.CalcObjective(problem);
        if (new_objective < iter_best_objective) {
          num_stagnation = 0;
          iter_best_objective = new_objective;
        }
        if (new_objective < best_objective) {
          best_objective = new_objective;
          best_solution = new_solution;
          if (config.listener != nullptr) {
            config.listener->OnUpdated(best_solution, best_objective);
          }
        }
        if (acceptance_rule->Accept(objective, new_objective, random)) {
          objective = new_objective;
          solution = new_solution;
        } else {
          new_solution = solution;
        }
        Perturb(problem, config, new_solution, context, random);
      }
    }
    if (config.listener != nullptr) {
      config.listener->OnEnd(best_solution, best_objective);
    }
    return best_solution;
  }
}  // namespace alkaidsd
