#pragma once

#include <alkaidsd/acceptance_rule.h>
#include <alkaidsd/inter_operator.h>
#include <alkaidsd/intra_operator.h>
#include <alkaidsd/ruin_method.h>
#include <alkaidsd/sorter.h>

#include <functional>
#include <memory>
#include <vector>

namespace alkaidsd {
  /**
   * @class Listener
   * @brief Interface for listening to events during the optimization process.
   */
  class Listener {
  public:
    /**
     * @brief Virtual destructor for the Listener class.
     */
    virtual ~Listener() = default;

    /**
     * @brief Called when the optimization process starts.
     */
    virtual void OnStart() = 0;

    /**
     * @brief Called when the solution is updated during the optimization process.
     * @param solution The updated solution.
     * @param objective The objective value of the updated solution.
     */
    virtual void OnUpdated(const AlkaidSolution &solution, int objective) = 0;

    /**
     * @brief Called when the optimization process ends.
     * @param solution The final solution.
     * @param objective The objective value of the final solution.
     */
    virtual void OnEnd(const AlkaidSolution &solution, int objective) = 0;
  };

  /**
   * @struct Config
   * @brief General configuration options for the optimization process.
   */
  struct Config {
    uint32_t random_seed; /**< The random seed for the optimization process. */
    double time_limit;    /**< The time limit (in seconds) for the optimization process. */
  };

  /**
   * @struct AlkaidConfig
   * @brief Configuration options for the AlkaidSolver optimization process.
   */
  struct AlkaidConfig : public Config {
    double blink_rate;    /**< The blink rate for the SplitReinsertion process. */
    std::vector<std::unique_ptr<inter_operator::InterOperator>>
        inter_operators; /**< The inter-operators for optimizing the solution. */
    std::vector<std::unique_ptr<intra_operator::IntraOperator>>
        intra_operators; /**< The intra-operators for optimizing the solution. */
    std::function<std::unique_ptr<acceptance_rule::AcceptanceRule>()>
        acceptance_rule; /**< The acceptance rule for accepting new solutions. */
    std::unique_ptr<ruin_method::RuinMethod>
        ruin_method;       /**< The ruin method for destroying parts of the solution. */
    sorter::Sorter sorter; /**< The sorter for sorting customers during the perturbation process. */
    std::unique_ptr<Listener> listener; /**< The listener for receiving optimization events. */
  };
}  // namespace alkaidsd
