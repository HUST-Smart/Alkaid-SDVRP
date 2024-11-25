#pragma once

#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

#include "route_context.h"

namespace alkaidsd {
  void Repair(const Instance &instance, Node route_index, AlkaidSolution &solution, RouteContext &context);
}  // namespace alkaidsd
