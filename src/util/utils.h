#ifndef ALKAID_SD_SRC_UTIL_UTILS_H_
#define ALKAID_SD_SRC_UTIL_UTILS_H_

#include "model/route_context.h"
#include "model/solution.h"
#include "util/delta.h"

namespace alkaid_sd {

template <typename T> struct InsertionWithCost {
  Node predecessor;
  Node successor;
  Node route_index;
  Delta<T> cost;

  inline bool Update(const InsertionWithCost<T> &insertion, Random &random) {
    if (cost.Update(insertion.cost, random)) {
      predecessor = insertion.predecessor;
      successor = insertion.successor;
      route_index = insertion.route_index;
      return true;
    }
    return false;
  }
};

template <class Func>
auto CalcBestInsertion(const Solution &solution, const Func &func,
                       const RouteContext &context, Node route_index,
                       Node customer, Random &random) {
  Node head = context.heads[route_index];
  auto head_cost = func(0, head, customer);
  InsertionWithCost<decltype(head_cost)> best_insertion{0, head, route_index,
                                                        Delta(head_cost, 1)};
  Node node_index = head;
  while (node_index) {
    auto cost = func(node_index, solution.successor(node_index), customer);
    if (best_insertion.cost.Update(cost, random)) {
      best_insertion.predecessor = node_index;
      best_insertion.successor = solution.successor(node_index);
    }
    node_index = solution.successor(node_index);
  }
  return best_insertion;
}

inline Node CalcFleetLowerBound(const Problem &problem) {
  int sum_demands = 0;
  for (Node i = 1; i < problem.num_customers; ++i) {
    sum_demands += problem.customers[i].demand;
  }
  return (sum_demands + problem.capacity - 1) / problem.capacity;
}

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_UTIL_UTILS_H_
