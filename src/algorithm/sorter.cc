#include "algorithm/sorter.h"

#include <algorithm>
#include <map>
#include <string>

#include "util/utils.h"

namespace alkaid_sd {

void Sorter::AddSortFunction(SortFunction *sort_function, double weight) {
  sum_weights_ += weight;
  sort_functions_.emplace_back(sort_function, weight);
}

void Sorter::Sort(const Problem &problem, const DistanceMatrix &distance_matrix,
                  std::vector<Node> &customers, Random &random) const {
  double r = random.NextFloat() * sum_weights_;
  for (auto &&[sort_function, weight] : sort_functions_) {
    r -= weight;
    if (r < 0) {
      sort_function(problem, distance_matrix, customers, random);
      return;
    }
  }
}

void SortByRandom(const Problem &problem, const DistanceMatrix &distance_matrix,
                  std::vector<Node> &customers, Random &random) {
  random.Shuffle(customers.begin(), customers.end());
}

void SortByDemand(const Problem &problem, const DistanceMatrix &distance_matrix,
                  std::vector<Node> &customers, Random &random) {
  std::stable_sort(customers.begin(), customers.end(), [&](Node lhs, Node rhs) {
    return problem.customers[lhs].demand > problem.customers[rhs].demand;
  });
}

void SortByFar(const Problem &problem, const DistanceMatrix &distance_matrix,
               std::vector<Node> &customers, Random &random) {
  std::stable_sort(customers.begin(), customers.end(), [&](Node lhs, Node rhs) {
    return distance_matrix[0][lhs] > distance_matrix[0][rhs];
  });
}

void SortByClose(const Problem &problem, const DistanceMatrix &distance_matrix,
                 std::vector<Node> &customers, Random &random) {
  std::stable_sort(customers.begin(), customers.end(), [&](Node lhs, Node rhs) {
    return distance_matrix[0][lhs] < distance_matrix[0][rhs];
  });
}

static const std::map<std::string, SortFunction *> kSortFunctionMap = {
    {"random", SortByRandom},
    {"demand", SortByDemand},
    {"far", SortByFar},
    {"close", SortByClose}};

} // namespace alkaid_sd

void nlohmann::adl_serializer<alkaid_sd::Sorter>::from_json(
    const json &j, alkaid_sd::Sorter &sorter) {
  for (auto &[key, value] : j.items()) {
    sorter.AddSortFunction(alkaid_sd::kSortFunctionMap.at(key), value);
  }
}
