#pragma once

#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

#include <vector>

namespace alkaidsd {
  class RouteContext {
  public:
    Node Head(Node route_index) const { return routes_[route_index].head; }
    Node Tail(Node route_index) const { return routes_[route_index].tail; }
    int Load(Node route_index) const { return routes_[route_index].load; }
    int PreLoad(Node node_index) const { return pre_loads_[node_index]; }
    void SetHead(Node route_index, Node head) { routes_[route_index].head = head; }
    void AddLoad(Node route_index, int load) { routes_[route_index].load += load; }
    Node NumRoutes() const { return routes_.size(); }
    void SetNumRoutes(Node num_routes) { routes_.resize(num_routes); }
    void AddRoute(Node head, Node tail, int load) {
      routes_.emplace_back(RouteData{head, tail, load});
    }
    void CalcRouteContext(const AlkaidSolution &solution);
    void UpdateRouteContext(const AlkaidSolution &solution, Node route_index, Node predecessor);
    void MoveRouteContext(Node dest_route_index, Node src_route_index);

  private:
    struct RouteData {
      Node head;
      Node tail;
      int load;
    };
    std::vector<RouteData> routes_;
    std::vector<int> pre_loads_;
  };
}  // namespace alkaidsd
