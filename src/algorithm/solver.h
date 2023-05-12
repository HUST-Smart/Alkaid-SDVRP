#ifndef ALKAID_SD_SRC_ALGORITHM_SOLVER_H_
#define ALKAID_SD_SRC_ALGORITHM_SOLVER_H_

#include <memory>
#include <string>

#include "config/config.h"
#include "model/solution.h"
#include "problem.h"

namespace alkaid_sd {

std::unique_ptr<Solution> Solve(const Config &config, const Problem &problem,
                                const DistanceMatrix &distance_matrix);

void RestoreByFloyd(const DistanceMatrix &distance_matrix, Solution &solution);

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_ALGORITHM_SOLVER_H_
