#ifndef DIMACS_12_SRC_SDVRP_VISUALIZATION_H_
#define DIMACS_12_SRC_SDVRP_VISUALIZATION_H_

#include <string>

#include "sdvrp/distance_matrix.h"
#include "sdvrp/problem.h"
#include "sdvrp/splitils/model/solution.h"

namespace vrp::sdvrp {

std::string SolutionToGraph(const Problem &problem,
                            const DistanceMatrix &distance_matrix,
                            const splitils::Solution &solution,
                            float scale = 0.5, bool load = true,
                            bool open = true);

void ShowGraphUrl(const std::string &graph);

} // namespace vrp::sdvrp

#endif // DIMACS_12_SRC_SDVRP_VISUALIZATION_H_
