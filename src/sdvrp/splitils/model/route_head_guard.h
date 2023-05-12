#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_ROUTE_HEAD_GUARD_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_ROUTE_HEAD_GUARD_H_

#include "sdvrp/splitils/model/route_context.h"
#include "sdvrp/splitils/model/solution.h"

namespace vrp::sdvrp::splitils {

struct RouteHeadGuard {
  Solution &solution;
  RouteContext &context;
  Node route_index;
  RouteHeadGuard(Solution &solution, RouteContext &context, Node route_index)
      : solution(solution), context(context), route_index(route_index) {
    solution.set_successor(0, context.heads[route_index]);
  }
  ~RouteHeadGuard() { context.heads[route_index] = solution.successor(0); }
};

} // namespace vrp::sdvrp::splitils

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_ROUTE_HEAD_GUARD_H_
