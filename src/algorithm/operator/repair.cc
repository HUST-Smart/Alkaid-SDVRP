#include "algorithm/operators.h"

#include <map>

namespace alkaid_sd {

void MergeAdjacentSameCustomers(const DistanceMatrix &distance_matrix,
                                Node route_index, Solution &solution,
                                RouteContext &context) {
  Node node_index = context.heads[route_index];
  while (true) {
    Node successor = solution.successor(node_index);
    if (!successor) {
      break;
    }
    if (solution.customer(node_index) == solution.customer(successor)) {
      solution.set_load(node_index,
                        solution.load(node_index) + solution.load(successor));
      solution.Remove(successor);
    } else {
      node_index = successor;
    }
  }
}

int CalcRemovalDelta(const DistanceMatrix &distance_matrix,
                     const Solution &solution, Node node_index) {
  Node predecessor = solution.predecessor(node_index);
  Node successor = solution.successor(node_index);
  return distance_matrix[solution.customer(predecessor)]
                        [solution.customer(successor)] -
         distance_matrix[solution.customer(predecessor)]
                        [solution.customer(node_index)] -
         distance_matrix[solution.customer(node_index)]
                        [solution.customer(successor)];
}

void Repair(const DistanceMatrix &distance_matrix, Node route_index,
            Solution &solution, RouteContext &context) {
  if (!context.heads[route_index]) {
    return;
  }
  MergeAdjacentSameCustomers(distance_matrix, route_index, solution, context);
  std::map<Node, Node> customer_node_map;
  Node node_index = context.heads[route_index];
  solution.set_successor(0, node_index);
  while (node_index) {
    Node successor = solution.successor(node_index);
    Node customer = solution.customer(node_index);
    auto it = customer_node_map.find(customer);
    if (it == customer_node_map.end()) {
      customer_node_map[customer] = node_index;
    } else {
      Node &last_node_index = it->second;
      if (CalcRemovalDelta(distance_matrix, solution, last_node_index) <
          CalcRemovalDelta(distance_matrix, solution, node_index)) {
        std::swap(last_node_index, node_index);
      }
      solution.set_load(last_node_index, solution.load(last_node_index) +
                                             solution.load(node_index));
      solution.Remove(node_index);
    }
    node_index = successor;
  }
  context.heads[route_index] = solution.successor(0);
  UpdateRouteContext(solution, route_index, 0, context);
}

} // namespace alkaid_sd
