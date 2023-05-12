#ifndef ALKAID_SD_SRC_MODEL_ROUTE_HEAD_GUARD_H_
#define ALKAID_SD_SRC_MODEL_ROUTE_HEAD_GUARD_H_

#include "model/route_context.h"
#include "model/solution.h"

namespace alkaid_sd {

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

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_MODEL_ROUTE_HEAD_GUARD_H_
