#include "repair.h"

#include <map>

namespace alkaidsd {
  void MergeAdjacentSameCustomers([[maybe_unused]] const Instance &instance, Node route_index,
                                  AlkaidSolution &solution, RouteContext &context) {
    Node node_index = context.Head(route_index);
    while (true) {
      Node successor = solution.Successor(node_index);
      if (!successor) {
        break;
      }
      if (solution.Customer(node_index) == solution.Customer(successor)) {
        solution.SetLoad(node_index, solution.Load(node_index) + solution.Load(successor));
        solution.Remove(successor);
      } else {
        node_index = successor;
      }
    }
  }

  int CalcRemovalDelta(const Instance &instance, const AlkaidSolution &solution, Node node_index) {
    Node predecessor = solution.Predecessor(node_index);
    Node successor = solution.Successor(node_index);
    return instance.distance_matrix[solution.Customer(predecessor)][solution.Customer(successor)]
           - instance.distance_matrix[solution.Customer(predecessor)][solution.Customer(node_index)]
           - instance.distance_matrix[solution.Customer(node_index)][solution.Customer(successor)];
  }

  void Repair(const Instance &instance, Node route_index, AlkaidSolution &solution, RouteContext &context) {
    if (!context.Head(route_index)) {
      return;
    }
    MergeAdjacentSameCustomers(instance, route_index, solution, context);
    std::map<Node, Node> customer_node_map;
    Node node_index = context.Head(route_index);
    solution.SetSuccessor(0, node_index);
    while (node_index) {
      Node successor = solution.Successor(node_index);
      Node customer = solution.Customer(node_index);
      auto it = customer_node_map.find(customer);
      if (it == customer_node_map.end()) {
        customer_node_map[customer] = node_index;
      } else {
        Node &last_node_index = it->second;
        if (CalcRemovalDelta(instance, solution, last_node_index)
            < CalcRemovalDelta(instance, solution, node_index)) {
          std::swap(last_node_index, node_index);
        }
        solution.SetLoad(last_node_index,
                         solution.Load(last_node_index) + solution.Load(node_index));
        solution.Remove(node_index);
      }
      node_index = successor;
    }
    context.SetHead(route_index, solution.Successor(0));
    context.UpdateRouteContext(solution, route_index, 0);
  }
}  // namespace alkaidsd
