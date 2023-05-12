#include "model/route_context.h"

namespace alkaid_sd {

void GetRoutes(const Solution &solution, Node &num_routes, Node *heads) {
  num_routes = 0;
  const Node *node_pool = solution.node_pool();
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node node_index = node_pool[i];
    if (solution.predecessor(node_index) == 0) {
      heads[num_routes++] = node_index;
    }
  }
}

void CalcRouteContext(const Solution &solution, RouteContext &route_context) {
  GetRoutes(solution, route_context.num_routes, route_context.heads);
  route_context.loads[0] = 0;
  for (Node i = 0; i < route_context.num_routes; ++i) {
    UpdateRouteContext(solution, i, 0, route_context);
  }
}

void UpdateRouteContext(const Solution &solution, Node route_index,
                        Node predecessor, RouteContext &route_context) {
  Load load = route_context.loads[predecessor];
  Node node_index = predecessor ? solution.successor(predecessor)
                                : route_context.heads[route_index];
  while (node_index) {
    load += solution.load(node_index);
    route_context.loads[node_index] = load;
    predecessor = node_index;
    node_index = solution.successor(node_index);
  }
  route_context.tails[route_index] = predecessor;
  route_context.route_loads[route_index] = load;
}

void MoveRouteContext(Node dest_route_index, Node src_route_index,
                      RouteContext &route_context) {
  route_context.heads[dest_route_index] = route_context.heads[src_route_index];
  route_context.tails[dest_route_index] = route_context.tails[src_route_index];
  route_context.route_loads[dest_route_index] =
      route_context.route_loads[src_route_index];
}

} // namespace alkaid_sd
