#pragma once

#include <alkaidsd/problem.h>
#include <alkaidsd/solution.h>

#include "random.h"
#include "route_context.h"

namespace alkaidsd {
  void SplitReinsertion(const Problem &problem, Node customer, int demand, double blink_rate,
                        Solution &solution, RouteContext &context, Random &random);

}  // namespace alkaidsd
