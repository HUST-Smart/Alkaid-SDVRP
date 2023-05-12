#ifndef ALKAID_SD_SRC_ALGORITHM_SORTER_H_
#define ALKAID_SD_SRC_ALGORITHM_SORTER_H_

#include <algorithm>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "distance_matrix.h"
#include "problem.h"
#include "util/random.h"

namespace alkaid_sd {

using SortFunction = void(const Problem &, const DistanceMatrix &,
                          std::vector<Node> &, Random &);

class Sorter {
public:
  void AddSortFunction(SortFunction *sort_function, double weight);
  void Sort(const Problem &problem, const DistanceMatrix &distance_matrix,
            std::vector<Node> &customers, Random &random) const;

private:
  double sum_weights_ = 0;
  std::vector<std::pair<SortFunction *, double>> sort_functions_;
};

} // namespace alkaid_sd

namespace nlohmann {

template <> struct adl_serializer<alkaid_sd::Sorter> {
  static void from_json(const json &j, alkaid_sd::Sorter &sorter);
  static void to_json(json &, const alkaid_sd::Sorter &) {}
};

} // namespace nlohmann

#endif // ALKAID_SD_SRC_ALGORITHM_SORTER_H_
