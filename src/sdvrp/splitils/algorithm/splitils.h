#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_SPLITILS_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_SPLITILS_H_

#include <memory>
#include <string>

#include "sdvrp/problem.h"
#include "sdvrp/splitils/config/config.h"
#include "sdvrp/splitils/model/solution.h"

namespace vrp::sdvrp::splitils {

std::unique_ptr<Solution>
SplitIlsAlgorithm(const Config &config, const Problem &problem,
                  const DistanceMatrix &distance_matrix, int lower_bound);

void RecoverFloyd(const DistanceMatrix &distance_matrix, Solution &solution);

void RecoverDijkstra(const Problem &problem,
                     const DistanceMatrix &distance_matrix, Solution &solution);

void RecoverDijkstraForce(const Problem &problem,
                          const DistanceMatrix &distance_matrix,
                          Solution &solution);

void Reflow(const Problem &problem, Solution &solution);

} // namespace vrp::sdvrp::splitils

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_SPLITILS_H_
