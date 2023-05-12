#ifndef ALKAID_SD_SRC_MODEL_ROUTE_CONTEXT_H_
#define ALKAID_SD_SRC_MODEL_ROUTE_CONTEXT_H_

#include "model/solution.h"
#include "problem.h"


namespace alkaid_sd {

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

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_MODEL_ROUTE_CONTEXT_H_
