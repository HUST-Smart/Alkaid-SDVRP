#pragma once

#include <alkaidsd/solution.h>

#include "../route_context.h"

namespace alkaidsd {
  struct RouteHeadGuard {
    Solution &solution;
    RouteContext &context;
    Node route_index;
    RouteHeadGuard(Solution &solution, RouteContext &context, Node route_index)
        : solution(solution), context(context), route_index(route_index) {
      solution.SetSuccessor(0, context.Head(route_index));
    }
    ~RouteHeadGuard() { context.SetHead(route_index, solution.Successor(0)); }
  };
}  // namespace alkaidsd
