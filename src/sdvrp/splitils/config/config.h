#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_CONFIG_CONFIG_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_CONFIG_CONFIG_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "sdvrp/splitils/algorithm/operators.h"
#include "sdvrp/splitils/algorithm/ruin_method.h"
#include "sdvrp/splitils/algorithm/sorter.h"
#include "util/acceptance_rule.h"
#include "util/timer.h"

namespace vrp::sdvrp::splitils {

struct Config {
  std::string problem_dir;
  std::string output_dir;
  uint32_t random_seed;
  double time_limit;
  TimerType time_limit_type;
  std::function<std::unique_ptr<AcceptanceRule<int>>()> acceptance_rule;
  std::unique_ptr<RuinMethod> ruin_method;
  Sorter sorter;
  double blink_rate;
  std::vector<std::vector<InterOperatorFunction *>> inter_operators;
  std::vector<IntraOperatorFunction *> intra_operators;
};

Config ReadConfigFromFile(const std::string &config_path,
                          const nlohmann::json &default_value,
                          const nlohmann::json &overwrite_value);

std::string ConvertConfigToString(const Config &config);

} // namespace vrp::sdvrp::splitils

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_CONFIG_CONFIG_H_
