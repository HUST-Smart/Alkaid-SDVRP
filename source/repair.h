#pragma once

#include <alkaidsd/problem.h>
#include <alkaidsd/solution.h>

#include "route_context.h"

namespace alkaidsd {
  void Repair(const Problem &problem, Node route_index, AlkaidSolution &solution, RouteContext &context);
}  // namespace alkaidsd
