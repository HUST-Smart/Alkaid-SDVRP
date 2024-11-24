#pragma once

#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

#include "random.h"
#include "route_context.h"

namespace alkaidsd {
  void SplitReinsertion(const Instance &instance, Node customer, int demand, double blink_rate,
                        AlkaidSolution &solution, RouteContext &context, Random &random);

}  // namespace alkaidsd
