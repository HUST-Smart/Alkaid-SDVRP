#ifndef ALKAID_SD_SRC_UTIL_ACCEPTANCE_RULE_H_
#define ALKAID_SD_SRC_UTIL_ACCEPTANCE_RULE_H_

#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "util/random.h"

namespace alkaid_sd {

template <class T> class AcceptanceRule {
public:
  virtual ~AcceptanceRule() = default;
  virtual bool Accept(T old_value, T new_value, Random &random) = 0;
};

template <class T> class HillClimbing : public AcceptanceRule<T> {
public:
  bool Accept(T old_value, T new_value, Random &random) override {
    return new_value < old_value;
  }
};

template <class T> class HillClimbingWithEqual : public AcceptanceRule<T> {
public:
  bool Accept(T old_value, T new_value, Random &random) override {
    return new_value <= old_value;
  }
};

template <class T> class LateAcceptanceHillClimbing : public AcceptanceRule<T> {
public:
  explicit LateAcceptanceHillClimbing(int length)
      : length_(length), position_(0),
        values_(length, std::numeric_limits<T>::max()) {}

  bool Accept(T old_value, T new_value, Random &random) override {
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

private:
  int length_;
  int position_;
  std::vector<T> values_;
};

template <class T> class SimulatedAnnealing : public AcceptanceRule<T> {
public:
  SimulatedAnnealing(double initial_temperature, double decay)
      : temperature_(initial_temperature), decay_(decay) {}

  bool Accept(T old_value, T new_value, Random &random) override {
    bool accepted =
        new_value <= old_value ||
        random.NextFloat() < exp((old_value - new_value) / temperature_);
    temperature_ *= decay_;
    return accepted;
  }

private:
  double temperature_;
  double decay_;
};

} // namespace alkaid_sd

namespace nlohmann {
template <class T>
struct adl_serializer<
    std::function<std::unique_ptr<alkaid_sd::AcceptanceRule<T>>()>> {
  static void
  from_json(const json &j,
            std::function<std::unique_ptr<alkaid_sd::AcceptanceRule<T>>()>
                &acceptance_criteria) {
    if (j["type"].get<std::string>() == "LAHC") {
      auto length = j["length"].get<int>();
      acceptance_criteria = [length]() {
        return std::make_unique<alkaid_sd::LateAcceptanceHillClimbing<T>>(
            length);
      };
    } else if (j["type"].get<std::string>() == "SA") {
      auto initial_temperature = j["initial_temperature"].get<double>();
      auto decay = j["decay"].get<double>();
      acceptance_criteria = [initial_temperature, decay]() {
        return std::make_unique<alkaid_sd::SimulatedAnnealing<T>>(
            initial_temperature, decay);
      };
    } else if (j["type"].get<std::string>() == "HCWE") {
      acceptance_criteria = []() {
        return std::make_unique<alkaid_sd::HillClimbingWithEqual<T>>();
      };
    } else {
      acceptance_criteria = []() {
        return std::make_unique<alkaid_sd::HillClimbing<T>>();
      };
    }
  }

  static void to_json(
      json &,
      const std::function<std::unique_ptr<alkaid_sd::AcceptanceRule<T>>()> &) {}
};

} // namespace nlohmann

#endif // ALKAID_SD_SRC_UTIL_ACCEPTANCE_RULE_H_
