#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_ROUTE_CONTEXT_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_ROUTE_CONTEXT_H_

#include "sdvrp/limit.h"
#include "sdvrp/problem.h"
#include "sdvrp/splitils/limit.h"
#include "sdvrp/splitils/model/solution.h"

namespace vrp::sdvrp::splitils {

struct RouteContext {
  Node num_routes;
  Node heads[kMaxNumRoutes];
  Node tails[kMaxNumRoutes];
  int route_loads[kMaxNumRoutes];
  Load loads[kMaxNumNodes];
};

void GetRoutes(const Solution &solution, Node &num_routes, Node *heads);

void CalcRouteContext(const Solution &solution, RouteContext &route_context);

void UpdateRouteContext(const Solution &solution, Node route_index,
                        Node predecessor, RouteContext &route_context);

void MoveRouteContext(Node dest_route_index, Node src_route_index,
                      RouteContext &route_context);

} // namespace vrp::sdvrp::splitils

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_ROUTE_CONTEXT_H_
