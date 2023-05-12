#include "sdvrp/splitils/config/config.h"

#include <fstream>
#include <iostream>

namespace vrp::sdvrp::splitils {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config, problem_dir, output_dir, random_seed,
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
#ifdef DEBUG
  std::cerr << j.dump(4) << std::endl;
#endif
  return config;
}

std::string ConvertConfigToString(const Config &config) {
  nlohmann::json j;
  to_json(j, config);
  return j.dump();
}

} // namespace vrp::sdvrp::splitils
