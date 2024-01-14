#pragma once

#include <alkaidsd/problem.h>
#include <alkaidsd/solution.h>

#include <memory>

#include "random.h"

namespace alkaidsd {
  Solution Construct(const Problem &problem, Random &random);
}  // namespace alkaidsd
