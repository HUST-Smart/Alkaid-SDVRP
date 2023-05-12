#ifndef ALKAID_SD_SRC_PROBLEM_H_
#define ALKAID_SD_SRC_PROBLEM_H_

#include <string>

namespace alkaid_sd {

using Node = short;
using Load = short;

constexpr auto kMaxNumCustomers = 289;
constexpr auto kMaxNumRoutes = kMaxNumCustomers - 1;
const Node kMaxNumNodes = kMaxNumCustomers * 3;

struct Customer {
  int x, y;
  Load demand;
};

struct Problem {
  Load capacity;
  Node num_customers;
  Customer customers[kMaxNumCustomers];
};

Problem ReadProblemFromFile(const std::string &problem_path);

std::string GetProblemPath(const std::string &problem_dir,
                           const std::string &instance_name);

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_PROBLEM_H_
