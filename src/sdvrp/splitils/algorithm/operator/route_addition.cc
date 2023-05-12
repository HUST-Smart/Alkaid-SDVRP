#include "sdvrp/splitils/algorithm/operators.h"

#include <map>
#include <vector>

#include "sdvrp/splitils/util/solution_utils.h"

namespace vrp::sdvrp::splitils {

struct RouteAdditionMove {
  Node customer_index;
  Node before;
  Node after;
};

struct RouteNodeLoadDelta {
  Node route_index;
  Node node_index;
  int load;
  int delta;
};

std::vector<RouteNodeLoadDelta> RouteAdditionPreprocess(
    const Problem &problem, const DistanceMatrix &distance_matrix,
    const Solution &solution, const RouteContext &context, Node customer_index,
    const std::vector<std::pair<Node, Node>> &route_node_indices) {
  std::vector<RouteNodeLoadDelta> route_node_load_deltas;
  route_node_load_deltas.reserve(route_node_indices.size() * 2);
  int load_limit = problem.capacity - problem.customers[customer_index].demand;
  for (auto &&[route_index, node_index] : route_node_indices) {
    Node predecessor = solution.predecessor(node_index);
    Node successor = solution.successor(node_index);
    int before_load = context.loads[predecessor];
    int after_load =
        context.route_loads[route_index] - context.loads[node_index];
    if (predecessor && before_load <= load_limit) {
      int delta =
          distance_matrix[solution.customer(predecessor)][customer_index] +
          distance_matrix[0][solution.customer(successor)] -
          distance_matrix[solution.customer(predecessor)]
                         [solution.customer(successor)];
      route_node_load_deltas.emplace_back(
          RouteNodeLoadDelta{route_index, predecessor, before_load, delta});
    }
    if (successor && after_load <= load_limit) {
      int delta =
          distance_matrix[solution.customer(successor)][customer_index] +
          distance_matrix[0][solution.customer(predecessor)] -
          distance_matrix[solution.customer(predecessor)]
                         [solution.customer(successor)];
      route_node_load_deltas.emplace_back(
          RouteNodeLoadDelta{route_index, successor, after_load, delta});
    }
  }
  return route_node_load_deltas;
}

void RouteAdditionType0(const Problem &problem,
                        const DistanceMatrix &distance_matrix,
                        const Solution &solution, const RouteContext &context,
                        Node customer_index, int base_delta,
                        RouteAdditionMove &best_move, Delta<int> &best_delta,
                        Random &random) {
  int delta = base_delta + distance_matrix[0][customer_index] * 2;
  if (best_delta.Update(delta, random)) {
    best_move = {customer_index, 0, 0};
  }
}

void RouteAdditionType1(
    const Problem &problem, const DistanceMatrix &distance_matrix,
    const Solution &solution, const RouteContext &context, Node customer_index,
    const std::vector<RouteNodeLoadDelta> &route_node_load_deltas,
    int base_delta, RouteAdditionMove &best_move, Delta<int> &best_delta,
    Random &random) {
  auto it = std::min_element(
      route_node_load_deltas.begin(), route_node_load_deltas.end(),
      [](auto &&lhs, auto &&rhs) { return lhs.delta < rhs.delta; });
  if (it != route_node_load_deltas.end()) {
    int delta = base_delta + distance_matrix[0][customer_index] + it->delta;
    if (best_delta.Update(delta, random)) {
      best_move = {customer_index, it->node_index, 0};
    }
  }
}

void RouteAdditionType2(
    const Problem &problem, const DistanceMatrix &distance_matrix,
    const Solution &solution, const RouteContext &context, Node customer_index,
    const std::vector<RouteNodeLoadDelta> &route_node_load_deltas,
    int base_delta, RouteAdditionMove &best_move, Delta<int> &best_delta,
    Random &random) {
  Load load_limit = problem.capacity - problem.customers[customer_index].demand;
  int size = route_node_load_deltas.size();
  for (int i = 0; i < size; ++i) {
    auto &&route_node_load_delta1 = route_node_load_deltas[i];
    for (int j = i + 1; j < size; ++j) {
      auto &&route_node_load_delta2 = route_node_load_deltas[j];
      if (route_node_load_delta1.route_index ==
              route_node_load_delta2.route_index ||
          route_node_load_delta1.load + route_node_load_delta2.load >
              load_limit) {
        continue;
      }
      int delta = base_delta + route_node_load_delta1.delta +
                  route_node_load_delta2.delta;
      if (best_delta.Update(delta, random)) {
        best_move = {customer_index, route_node_load_delta1.node_index,
                     route_node_load_delta2.node_index};
      }
    }
  }
}

bool RouteAddition(const Problem &problem,
                   const DistanceMatrix &distance_matrix, Solution &solution,
                   RouteContext &context, std::set<Node> &associated_routes,
                   Random &random, OperatorCaches &operator_caches) {
  RouteAdditionMove best_move{};
  Delta<int> best_delta;
  std::map<Node, std::vector<std::pair<Node, Node>>>
      customer_to_route_node_indices;
  for (Node route_index = 0; route_index < context.num_routes; ++route_index) {
    for (Node node_index = context.heads[route_index]; node_index;
         node_index = solution.successor(node_index)) {
      Node customer_index = solution.customer(node_index);
      customer_to_route_node_indices[customer_index].emplace_back(route_index,
                                                                  node_index);
    }
  }
  for (auto &&[customer_index, route_node_indices] :
       customer_to_route_node_indices) {
    if (route_node_indices.size() <= 1) {
      continue;
    }
    int base_delta = 0;
    for (auto &&[route_index, node_index] : route_node_indices) {
      Node predecessor = solution.predecessor(node_index);
      Node successor = solution.successor(node_index);
      base_delta +=
          distance_matrix[solution.customer(predecessor)]
                         [solution.customer(successor)] -
          distance_matrix[solution.customer(predecessor)][customer_index] -
          distance_matrix[customer_index][solution.customer(successor)];
    }
    auto route_node_load_delta =
        RouteAdditionPreprocess(problem, distance_matrix, solution, context,
                                customer_index, route_node_indices);
    RouteAdditionType0(problem, distance_matrix, solution, context,
                       customer_index, base_delta, best_move, best_delta,
                       random);
    RouteAdditionType1(problem, distance_matrix, solution, context,
                       customer_index, route_node_load_delta, base_delta,
                       best_move, best_delta, random);
    RouteAdditionType2(problem, distance_matrix, solution, context,
                       customer_index, route_node_load_delta, base_delta,
                       best_move, best_delta, random);
  }
  if (best_delta.value < 0) {
    Node customer_index = best_move.customer_index;
    auto &&route_node_indices = customer_to_route_node_indices[customer_index];
    Node new_node_index = solution.NewNode(
        customer_index, problem.customers[customer_index].demand);
    Node new_head = new_node_index;
    for (auto &&[route_index, node_index] : route_node_indices) {
      Node predecessor = solution.predecessor(node_index);
      Node successor = solution.successor(node_index);
      solution.Remove(node_index);
      solution.Link(predecessor, successor);
      if (successor) {
        if (successor == best_move.before) {
          solution.Link(predecessor, 0);
          ReversedLink(solution, successor, context.tails[route_index], 0,
                       new_node_index);
          new_head = context.tails[route_index];
          successor = 0;
        } else if (successor == best_move.after) {
          solution.Link(predecessor, 0);
          successor = 0;
        }
      }
      if (predecessor) {
        if (predecessor == best_move.before) {
          solution.Link(0, successor);
          new_head = context.heads[route_index];
          context.heads[route_index] = successor;
        } else if (predecessor == best_move.after) {
          solution.Link(0, successor);
          ReversedLink(solution, context.heads[route_index], predecessor,
                       new_node_index, 0);
          context.heads[route_index] = successor;
        }
      } else {
        context.heads[route_index] = successor;
      }
      associated_routes.insert(route_index);
    }
    solution.Link(best_move.before, new_node_index);
    solution.Link(new_node_index, best_move.after);
    associated_routes.insert(context.num_routes);
    context.heads[context.num_routes++] = new_head;
    return true;
  }
  return false;
}

} // namespace vrp::sdvrp::splitils
