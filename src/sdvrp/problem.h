#ifndef DIMACS_12_SRC_SDVRP_PROBLEM_H_
#define DIMACS_12_SRC_SDVRP_PROBLEM_H_

#include <string>

#include "sdvrp/limit.h"

namespace vrp::sdvrp {

using Node = short;
using Load = short;

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

} // namespace vrp::sdvrp

#endif // DIMACS_12_SRC_SDVRP_PROBLEM_H_
