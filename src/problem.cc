#include "problem.h"

#include <fstream>
#include <sstream>

namespace alkaid_sd {

Problem ReadProblemFromFile(const std::string &problem_path) {
  std::ifstream ifs(problem_path);
  if (ifs.fail()) {
    throw std::invalid_argument("Cannot open problem.");
  }
  Problem problem{};
  ifs >> problem.num_customers >> problem.capacity;
  ++problem.num_customers;
  problem.customers[0].demand = 0;
  for (Node i = 1; i < problem.num_customers; ++i) {
    ifs >> problem.customers[i].demand;
  }
  for (Node i = 0; i < problem.num_customers; ++i) {
    ifs >> problem.customers[i].x >> problem.customers[i].y;
  }
  return problem;
}

std::string GetProblemPath(const std::string &problem_dir,
                           const std::string &instance_name) {
  if (instance_name.empty()) {
    throw std::invalid_argument("Invalid instance name.");
  }
  std::stringstream ss;
  ss << problem_dir;
  if (instance_name.rfind("SD", 0) == 0) {
    ss << "SET-1/" << instance_name << ".txt";
  } else if (instance_name[0] == 'S') {
    ss << "SET-2/" << instance_name << ".sd";
  } else if (instance_name[0] == 'p') {
    ss << "SET-3/" << instance_name << ".cri";
  } else {
    ss << "SET-4/" << instance_name << ".sd";
  }
  return ss.str();
}

} // namespace alkaid_sd
