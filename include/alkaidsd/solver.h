#pragma once

#include <alkaidsd/config.h>
#include <alkaidsd/problem.h>
#include <alkaidsd/solution.h>

namespace alkaidsd {
  template <typename SolutionType, typename ConfigType, typename ProblemType> class Solver {
  public:
    /// @brief Main function for solving the problem.
    /// @param config The configuration.
    /// @param problem The problem instance.
    /// @return The solution to the problem.
    virtual SolutionType Solve(const ConfigType &config, const ProblemType &problem) = 0;
  };

  class AlkaidSolver : public Solver<AlkaidSolution, AlkaidConfig, Problem> {
  public:
    /// @brief Main function for solving the problem.
    /// @param config The configuration.
    /// @param problem The problem instance.
    /// @return The solution to the problem.
    AlkaidSolution Solve(const AlkaidConfig &config, const Problem &problem) override;
  };
}  // namespace alkaidsd
