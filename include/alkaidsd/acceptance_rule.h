#pragma once

#include <limits>
#include <vector>

namespace alkaidsd {
  class Random;
}  // namespace alkaidsd

namespace alkaidsd::acceptance_rule {
  /**
   * @brief This class defines the interface for acceptance rules used
   * in algorithms that involve accepting or rejecting new values based
   * on certain criteria.
   */
  class AcceptanceRule {
  public:
    /**
     * @brief Destructor for the AcceptanceRule class.
     */
    virtual ~AcceptanceRule() = default;

    /**
     * @brief Determines whether a new value should be accepted or rejected.
     *
     * This method is called to determine whether a new value should be accepted
     * or rejected based on the old value, the new value, and a random number generator.
     *
     * @param old_value The old value.
     * @param new_value The new value.
     * @param random The random number generator.
     * @return true if the new value should be accepted, false otherwise.
     */
    virtual bool Accept(int old_value, int new_value, Random &random) = 0;
  };

  /**
   * @brief A class comparing the new value with the old value and accepts it if
   * the new value is smaller than the old value.
   */
  class HillClimbing : public AcceptanceRule {
  public:
    bool Accept(int old_value, int new_value, [[maybe_unused]] Random &random) override;
  };

  /**
   * @brief A class comparing the new value with the old value and
   * accepts it if the new value is smaller than or equal to the old value.
   */
  class HillClimbingWithEqual : public AcceptanceRule {
  public:
    bool Accept(int old_value, int new_value, [[maybe_unused]] Random &random) override;
  };

  /**
   * @brief A class representing the Late Acceptance Hill Climbing acceptance rule.
   */
  class LateAcceptanceHillClimbing : public AcceptanceRule {
  public:
    explicit LateAcceptanceHillClimbing(int length)
        : length_(length), position_(0), values_(length, std::numeric_limits<int>::max()) {}

    bool Accept(int old_value, int new_value, [[maybe_unused]] Random &random) override;

  private:
    int length_;
    int position_;
    std::vector<int> values_;
  };

  /**
   * @brief A class representing the Simulated Annealing acceptance rule.
   */
  class SimulatedAnnealing : public AcceptanceRule {
  public:
    SimulatedAnnealing(double initial_temperature, double decay)
        : temperature_(initial_temperature), decay_(decay) {}

    bool Accept(int old_value, int new_value, Random &random) override;

  private:
    double temperature_;
    double decay_;
  };
}  // namespace alkaidsd::acceptance_rule
