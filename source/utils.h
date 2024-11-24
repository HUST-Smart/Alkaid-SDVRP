#pragma once

#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

#include "delta.h"
#include "route_context.h"

namespace alkaidsd {
  template <typename T> struct InsertionWithCost {
    Node predecessor;
    Node successor;
    Node route_index;
    Delta<T> cost;

    bool Update(const InsertionWithCost<T> &insertion, Random &random) {
      if (cost.Update(insertion.cost, random)) {
        predecessor = insertion.predecessor;
        successor = insertion.successor;
        route_index = insertion.route_index;
        return true;
      }
      return false;
    }
  };

  template <class Func> auto CalcBestInsertion(const AlkaidSolution &solution, const Func &func,
                                               const RouteContext &context, Node route_index,
                                               Node customer, Random &random) {
    Node head = context.Head(route_index);
    auto head_cost = func(0, head, customer);
    InsertionWithCost<decltype(head_cost)> best_insertion{0, head, route_index,
                                                          Delta(head_cost, 1)};
    Node node_index = head;
    while (node_index) {
      auto cost = func(node_index, solution.Successor(node_index), customer);
      if (best_insertion.cost.Update(cost, random)) {
        best_insertion.predecessor = node_index;
        best_insertion.successor = solution.Successor(node_index);
      }
      node_index = solution.Successor(node_index);
    }
    return best_insertion;
  }

  inline Node CalcFleetLowerBound(const Instance &instance) {
    int sum_demands = 0;
    for (Node i = 1; i < instance.num_customers; ++i) {
      sum_demands += instance.demands[i];
    }
    return (sum_demands + instance.capacity - 1) / instance.capacity;
  }
}  // namespace alkaidsd
