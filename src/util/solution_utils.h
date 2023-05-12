#ifndef ALKAID_SD_SRC_UTIL_SOLUTION_UTILS_H_
#define ALKAID_SD_SRC_UTIL_SOLUTION_UTILS_H_

#include <istream>
#include <ostream>
#include <vector>

#include "distance_matrix.h"
#include "model/route_context.h"
#include "model/solution.h"

namespace alkaid_sd {

int RemoveNode(const DistanceMatrix &distance_matrix, Node node_index,
               Node route_index, Solution &solution, RouteContext &context);

int CalcObjective(const Solution &solution,
                  const DistanceMatrix &distance_matrix);

void CheckFeasible(const Problem &problem, const Solution &solution);

void CheckRouteContextHeads(const Solution &solution,
                            const RouteContext &context);

void ReversedLink(Solution &solution, Node left, Node right, Node predecessor,
                  Node successor);

Solution ReadSolutionToStream(std::istream &stream);

void WriteSolutionToStream(const DistanceMatrix &distance_matrix,
                           const Solution &solution,
                           const std::string &processor, double time,
                           std::ostream &stream);

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_UTIL_SOLUTION_UTILS_H_
