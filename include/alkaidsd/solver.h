#pragma once

#include <alkaidsd/config.h>
#include <alkaidsd/problem.h>
#include <alkaidsd/solution.h>

namespace alkaidsd {
  /// @brief Main function for solving the problem.
  /// @param config The configuration.
  /// @param problem The problem instance.
  /// @return The solution to the problem.
  Solution Solve(const Config &config, const Problem &problem);
}  // namespace alkaidsd
