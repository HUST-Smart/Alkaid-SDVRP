#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_SORTER_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_SORTER_H_

#include <algorithm>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "sdvrp/distance_matrix.h"
#include "sdvrp/problem.h"
#include "util/random.h"

namespace vrp::sdvrp::splitils {

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

} // namespace vrp::sdvrp::splitils

namespace nlohmann {

template <> struct adl_serializer<vrp::sdvrp::splitils::Sorter> {
  static void from_json(const json &j, vrp::sdvrp::splitils::Sorter &sorter);
  static void to_json(json &, const vrp::sdvrp::splitils::Sorter &) {}
};

} // namespace nlohmann

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_SORTER_H_
