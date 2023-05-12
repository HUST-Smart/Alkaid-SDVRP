#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_UTIL_SOLUTION_UTILS_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_UTIL_SOLUTION_UTILS_H_

#include <istream>
#include <ostream>
#include <vector>

#include "sdvrp/distance_matrix.h"
#include "sdvrp/splitils/model/route_context.h"
#include "sdvrp/splitils/model/solution.h"

namespace vrp::sdvrp::splitils {

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
                           const Solution &solution, double time,
                           std::ostream &stream);

} // namespace vrp::sdvrp::splitils

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_UTIL_SOLUTION_UTILS_H_
