#ifndef ALKAID_SD_SRC_CONFIG_CONFIG_H_
#define ALKAID_SD_SRC_CONFIG_CONFIG_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "algorithm/operators.h"
#include "algorithm/ruin_method.h"
#include "algorithm/sorter.h"
#include "util/acceptance_rule.h"
#include "util/timer.h"

namespace alkaid_sd {

struct Config {
  std::string problem_dir;
  uint32_t random_seed;
  std::string processor;
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

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_CONFIG_CONFIG_H_
