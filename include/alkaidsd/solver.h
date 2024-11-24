#pragma once

#include <alkaidsd/config.h>
#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

namespace alkaidsd {
  template <typename SolutionType, typename ConfigType, typename InstanceType> class Solver {
  public:
    /// @brief Main function for solving the problem instance.
    /// @param config The configuration.
    /// @param instance The problem instance.
    /// @return The solution to the problem instance.
    virtual SolutionType Solve(const ConfigType &config, const InstanceType &instance) = 0;
  };

  class AlkaidSolver : public Solver<AlkaidSolution, AlkaidConfig, Instance> {
  public:
    /// @brief Main function for solving the problem instance.
    /// @param config The configuration.
    /// @param instance The problem instance.
    /// @return The solution to the problem instance.
    AlkaidSolution Solve(const AlkaidConfig &config, const Instance &instance) override;
  };
}  // namespace alkaidsd
