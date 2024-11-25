#include "route_context.h"

namespace alkaidsd {
  void RouteContext::CalcRouteContext(const AlkaidSolution &solution) {
    routes_.clear();
    for (Node node_index : solution.NodeIndices()) {
      if (solution.Predecessor(node_index) == 0) {
        AddRoute(node_index, node_index, 0);
      }
    }
    pre_loads_.resize(solution.MaxNodeIndex() + 1);
    for (Node route_index = 0; route_index < NumRoutes(); ++route_index) {
      UpdateRouteContext(solution, route_index, 0);
    }
  }

  void RouteContext::UpdateRouteContext(const AlkaidSolution &solution, Node route_index,
                                        Node predecessor) {
    pre_loads_.resize(solution.MaxNodeIndex() + 1);
    int load = pre_loads_[predecessor];
    Node node_index = predecessor ? solution.Successor(predecessor) : Head(route_index);
    while (node_index) {
      load += solution.Load(node_index);
      pre_loads_[node_index] = load;
      predecessor = node_index;
      node_index = solution.Successor(node_index);
    }
    routes_[route_index].tail = predecessor;
    routes_[route_index].load = load;
  }

  void RouteContext::MoveRouteContext(Node dest_route_index, Node src_route_index) {
    routes_[dest_route_index] = routes_[src_route_index];
  }
}  // namespace alkaidsd
