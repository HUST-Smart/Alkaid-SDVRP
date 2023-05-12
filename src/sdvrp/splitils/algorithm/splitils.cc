#include "sdvrp/splitils/algorithm/splitils.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

#include "sdvrp/splitils/algorithm/base_star.h"
#include "sdvrp/splitils/algorithm/construction.h"
#include "sdvrp/splitils/algorithm/operators.h"
#include "sdvrp/splitils/util/mcmf.h"
#include "sdvrp/splitils/util/solution_utils.h"
#include "sdvrp/splitils/util/submit.h"
#include "sdvrp/splitils/util/utils.h"
#include "util/utils.h"

namespace vrp::sdvrp::splitils {

void IntraRouteSearch(const Problem &problem, const Config &config,
                      const DistanceMatrix &distance_matrix, Node route_index,
                      Solution &solution, RouteContext &context,
                      Random &random) {
  Repair(distance_matrix, route_index, solution, context);
  std::vector<Node> intra_neighborhoods(config.intra_operators.size());
  std::iota(intra_neighborhoods.begin(), intra_neighborhoods.end(), 0);
  while (true) {
    Shuffle(intra_neighborhoods.begin(), intra_neighborhoods.end(), random);
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
    Shuffle(inter_neighborhoods.begin(), inter_neighborhoods.end(), random);
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

std::unique_ptr<Solution>
SplitIlsAlgorithm(const Config &config, const Problem &problem,
                  const DistanceMatrix &distance_matrix, int lower_bound) {
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
  while (!timer->IsTimeOut() && best_objective > lower_bound) {
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
        if (best_objective <= lower_bound) {
          break;
        }
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

void RecoverFloyd(const DistanceMatrix &distance_matrix, Solution &solution) {
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

void RecoverDijkstra(const Problem &problem,
                     const DistanceMatrix &distance_matrix,
                     Solution &solution) {
  std::vector<std::tuple<int, int, int>> distances(problem.num_customers);
  std::vector<Node> predecessors(problem.num_customers);
  std::vector<bool> visited(problem.num_customers);
  std::vector<bool> forbid(problem.num_customers);
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node node_index = solution.node_pool()[i];
    if (!solution.predecessor(node_index)) {
      std::vector<Node> route;
      route.push_back(0);
      std::fill(forbid.begin(), forbid.end(), false);
      while (node_index) {
        route.push_back(node_index);
        forbid[solution.customer(node_index)] = true;
        node_index = solution.successor(node_index);
      }
      route.push_back(0);
      for (auto j = 0; j + 1 < route.size(); ++j) {
        Node predecessor = route[j];
        Node successor = route[j + 1];
        Node s = solution.customer(predecessor);
        Node t = solution.customer(successor);
        for (Node k = t > 0; k < problem.num_customers; ++k) {
          distances[k] = {distance_matrix.original[s][k], forbid[k], 0};
          predecessors[k] = s;
          visited[k] = false;
        }
        while (true) {
          Node u = -1;
          for (Node k = t > 0; k < problem.num_customers; ++k) {
            if (!visited[k]) {
              if (u == -1 || distances[k] < distances[u]) {
                u = k;
              }
            }
          }
          if (u == t) {
            break;
          }
          visited[u] = true;
          for (Node k = t > 0; k < problem.num_customers; ++k) {
            if (!visited[k]) {
              std::tuple<int, int, int> new_distance{
                  std::get<0>(distances[u]) + distance_matrix.original[u][k],
                  std::get<1>(distances[u]) + forbid[k],
                  std::get<2>(distances[u]) + 1};
              if (distances[k] > new_distance) {
                distances[k] = new_distance;
                predecessors[k] = u;
              }
            }
          }
        }
        Node u = predecessors[t];
        while (u != s) {
          forbid[u] = true;
          successor = solution.Insert(u, 0, predecessor, successor);
          u = predecessors[u];
        }
      }
    }
  }
}

void RecoverDijkstraForce(const Problem &problem,
                          const DistanceMatrix &distance_matrix,
                          Solution &solution) {
  std::vector<std::tuple<int, int>> distances(problem.num_customers);
  std::vector<Node> predecessors(problem.num_customers);
  std::vector<bool> visited(problem.num_customers);
  std::vector<bool> forbid(problem.num_customers);
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node node_index = solution.node_pool()[i];
    if (!solution.predecessor(node_index)) {
      std::vector<Node> route;
      route.push_back(0);
      std::fill(forbid.begin(), forbid.end(), false);
      forbid[0] = true;
      while (node_index) {
        route.push_back(node_index);
        forbid[solution.customer(node_index)] = true;
        node_index = solution.successor(node_index);
      }
      route.push_back(0);
      for (auto j = 0; j + 1 < route.size(); ++j) {
        Node predecessor = route[j];
        Node successor = route[j + 1];
        Node s = solution.customer(predecessor);
        Node t = solution.customer(successor);
        for (Node k = 0; k < problem.num_customers; ++k) {
          if (!forbid[k] || k == t) {
            distances[k] = {distance_matrix.original[s][k], 0};
            predecessors[k] = s;
            visited[k] = false;
          }
        }
        while (true) {
          Node u = -1;
          for (Node k = 0; k < problem.num_customers; ++k) {
            if (!visited[k] && (!forbid[k] || k == t)) {
              if (u == -1 || distances[k] < distances[u]) {
                u = k;
              }
            }
          }
          if (u == t) {
            break;
          }
          visited[u] = true;
          for (Node k = 0; k < problem.num_customers; ++k) {
            if (!visited[k] && (!forbid[k] || k == t)) {
              std::tuple<int, int> new_distance{
                  std::get<0>(distances[u]) + distance_matrix.original[u][k],
                  std::get<1>(distances[u]) + 1};
              if (distances[k] > new_distance) {
                distances[k] = new_distance;
                predecessors[k] = u;
              }
            }
          }
        }
        Node u = predecessors[t];
        while (u != s) {
          forbid[u] = true;
          successor = solution.Insert(u, 0, predecessor, successor);
          u = predecessors[u];
        }
      }
    }
  }
}

void Reflow(const Problem &problem, Solution &solution) {
  std::vector<Node> heads;
  Node max_index = 0;
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node index = solution.node_pool()[i];
    max_index = std::max(max_index, index);
    if (!solution.predecessor(index)) {
      heads.push_back(index);
    }
  }
  int num_routes = static_cast<int>(heads.size());
  int num_customers = problem.num_customers;
  int s = num_routes + num_customers;
  int t = s + 1;
  MCMF mcmf(t + 1);
  for (int i = 0; i < num_routes; ++i) {
    mcmf.Link(s, i, problem.capacity, 0);
  }
  for (int i = 0; i < num_customers; ++i) {
    mcmf.Link(num_routes + i, t, problem.customers[i].demand, 0);
  }
  std::vector<int> edges(max_index + 1);
  for (int i = 0; i < num_routes; ++i) {
    Node u = heads[i];
    while (u) {
      int j = num_routes + solution.customer(u);
      edges[u] = mcmf.Link(i, j, 1, 0);
      mcmf.Link(i, j, problem.capacity, 1);
      u = solution.successor(u);
    }
  }
  mcmf.Solve(s, t);
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node index = solution.node_pool()[i];
    int edge = edges[index];
    int load = mcmf.e[edge].flow + mcmf.e[edge + 2].flow;
    solution.set_load(index, load);
  }
}

} // namespace vrp::sdvrp::splitils
