#pragma once

#include <alkaidsd/problem.h>
#include <alkaidsd/solution.h>

#include <memory>

#include "random.h"

namespace alkaidsd {
  AlkaidSolution Construct(const Problem &problem, Random &random);
}  // namespace alkaidsd
