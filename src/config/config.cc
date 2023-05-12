#include "config/config.h"

#include <fstream>
#include <iostream>

namespace alkaid_sd {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config, problem_dir, random_seed, processor,
                                   time_limit, time_limit_type, acceptance_rule,
                                   ruin_method, sorter, blink_rate,
                                   inter_operators, intra_operators)

Config ReadConfigFromFile(const std::string &config_path,
                          const nlohmann::json &default_value,
                          const nlohmann::json &overwrite_value) {
  std::ifstream ifs(config_path);
  if (ifs.fail()) {
    throw std::invalid_argument("Cannot open config.");
  }
  nlohmann::json j;
  ifs >> j;
  j.insert(default_value.begin(), default_value.end());
  if (!overwrite_value.empty()) {
    j.update(overwrite_value);
  }
  Config config{};
  from_json(j, config);
  return config;
}

std::string ConvertConfigToString(const Config &config) {
  nlohmann::json j;
  to_json(j, config);
  return j.dump();
}

} // namespace alkaid_sd
