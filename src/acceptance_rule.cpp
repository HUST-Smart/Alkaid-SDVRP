#include <alkaidsd/acceptance_rule.h>

#include <cmath>

#include "random.h"

namespace alkaidsd::acceptance_rule {
  bool HillClimbing::Accept(int old_value, int new_value, [[maybe_unused]] Random &random) {
    return new_value < old_value;
  }

  bool HillClimbingWithEqual::Accept(int old_value, int new_value,
                                     [[maybe_unused]] Random &random) {
    return new_value <= old_value;
  }

  bool LateAcceptanceHillClimbing::Accept(int old_value, int new_value,
                                          [[maybe_unused]] Random &random) {
    bool accepted = false;
    if (new_value <= old_value || new_value < values_[position_]) {
      old_value = new_value;
      accepted = true;
    }
    if (old_value < values_[position_]) {
      values_[position_] = old_value;
    }
    position_ = (position_ + 1) % length_;
    return accepted;
  }

  bool SimulatedAnnealing::Accept(int old_value, int new_value, Random &random) {
    bool accepted = new_value <= old_value
                    || random.NextFloat() < exp((old_value - new_value) / temperature_);
    temperature_ *= decay_;
    return accepted;
  }
}  // namespace alkaidsd::acceptance_rule
