#pragma once

#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

#include <memory>

#include "random.h"

namespace alkaidsd {
  AlkaidSolution Construct(const Instance &instance, Random &random);
}  // namespace alkaidsd
