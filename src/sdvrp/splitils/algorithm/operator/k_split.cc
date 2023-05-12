#include "sdvrp/splitils/algorithm/operators.h"

#include "sdvrp/splitils/util/solution_utils.h"

namespace vrp::sdvrp::splitils {

struct NodeInfo {
  Node node_index;
  Node route_index;
  NodeInfo(Node node_index, Node route_index)
      : node_index(node_index), route_index(route_index) {}
};

int DoKSplit(const Problem &problem, const DistanceMatrix &distance_matrix,
             const std::vector<NodeInfo> &customer_nodes, Solution &solution,
             RouteContext &context, std::set<Node> &associated_routes,
             Random &random) {
  int delta = 0;
  for (const auto &customer_info : customer_nodes) {
    Node node_index = customer_info.node_index;
    Node route_index = customer_info.route_index;
    delta +=
        RemoveNode(distance_matrix, node_index, route_index, solution, context);
    associated_routes.insert(route_index);
  }
  Node customer = solution.customer(customer_nodes[0].node_index);
  delta += SplitReinsertion(problem, distance_matrix, customer,
                            problem.customers[customer].demand, 0.0, solution,
                            context, associated_routes, random);
  return delta;
}

bool KSplit(const Problem &problem, const DistanceMatrix &distance_matrix,
            Solution &solution, RouteContext &context,
            std::set<Node> &associated_routes, Random &random,
            OperatorCaches &operator_caches) {
  Delta<int> best_delta;
  Node best_customer;
  auto solution_bak = std::make_unique<Solution>(solution);
  auto context_bak = std::make_unique<RouteContext>(context);
  std::vector<std::vector<NodeInfo>> customers(problem.num_customers);
  for (Node route_index = 0; route_index < context.num_routes; ++route_index) {
    Node node_index = context.heads[route_index];
    while (node_index) {
      customers[solution.customer(node_index)].emplace_back(node_index,
                                                            route_index);
      node_index = solution.successor(node_index);
    }
  }
  for (Node i = 1; i < problem.num_customers; ++i) {
    int delta = DoKSplit(problem, distance_matrix, customers[i], solution,
                         context, associated_routes, random);
    associated_routes.clear();
    if (best_delta.Update(delta, random)) {
      best_customer = i;
    }
    solution = *solution_bak;
    context = *context_bak;
  }
  if (best_delta.value < 0) {
    DoKSplit(problem, distance_matrix, customers[best_customer], solution,
             context, associated_routes, random);
    return true;
  }
  return false;
}

} // namespace vrp::sdvrp::splitils
