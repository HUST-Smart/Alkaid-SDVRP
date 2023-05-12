#ifndef ALKAID_SD_SRC_ALGORITHM_CONSTRUCTION_H_
#define ALKAID_SD_SRC_ALGORITHM_CONSTRUCTION_H_

#include <memory>

#include "distance_matrix.h"
#include "model/solution.h"
#include "problem.h"
#include "util/random.h"


namespace alkaid_sd {

std::unique_ptr<Solution> Construct(const Problem &problem,
                                    const DistanceMatrix &distance_matrix,
                                    Random &random);

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_ALGORITHM_CONSTRUCTION_H_
