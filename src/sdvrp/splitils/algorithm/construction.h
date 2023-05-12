#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_CONSTRUCTION_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_CONSTRUCTION_H_

#include <memory>

#include "sdvrp/distance_matrix.h"
#include "sdvrp/problem.h"
#include "sdvrp/splitils/model/solution.h"
#include "util/random.h"

namespace vrp::sdvrp::splitils {

std::unique_ptr<Solution> Construct(const Problem &problem,
                                    const DistanceMatrix &distance_matrix,
                                    Random &random);

} // namespace vrp::sdvrp::splitils

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_CONSTRUCTION_H_
