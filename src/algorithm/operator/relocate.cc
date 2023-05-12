#include "algorithm/operators.h"

#include "algorithm/base_star.h"
#include "model/route_head_guard.h"

namespace alkaid_sd {

struct RelocateMove {
  Node route_x, route_y;
  Node node_x, predecessor_x, successor_x;
};

inline void DoRelocate(RelocateMove &move, Solution &solution,
                       RouteContext &context) {
  Node predecessor_x = solution.predecessor(move.node_x);
  Node successor_x = solution.successor(move.node_x);
  {
    RouteHeadGuard guard(solution, context, move.route_x);
    solution.Link(predecessor_x, successor_x);
  }
  {
    RouteHeadGuard guard(solution, context, move.route_y);
    solution.Link(move.predecessor_x, move.node_x);
    solution.Link(move.node_x, move.successor_x);
  }
}

void Relocate(const Problem &problem, const DistanceMatrix &distance_matrix,
              const Solution &solution, const RouteContext &context,
              Node route_x, Node route_y, RelocateMove &best_move,
              Delta<int> &best_delta, Random &random) {
  PreprocessStarInsertions(problem, distance_matrix, solution, context, route_y,
                           random);
  Node node_x = context.heads[route_x];
  while (node_x) {
    if (context.route_loads[route_y] + solution.load(node_x) <=
        problem.capacity) {
      auto insertion =
          star_caches[route_y][solution.customer(node_x)].FindBest();
      Node predecessor_x = solution.predecessor(node_x);
      Node successor_x = solution.successor(node_x);
      int delta =
          insertion->delta.value - CalcDelta(distance_matrix, solution, node_x,
                                             predecessor_x, successor_x);
      if (best_delta.Update(delta, random)) {
        best_move = {route_x, route_y, node_x, insertion->predecessor,
                     insertion->successor};
      }
    }
    node_x = solution.successor(node_x);
  }
}

bool Relocate(const Problem &problem, const DistanceMatrix &distance_matrix,
              Solution &solution, RouteContext &context,
              std::set<Node> &associated_routes, Random &random,
              OperatorCaches &operator_caches) {
  RelocateMove best_move{};
  Delta<int> best_delta;
  for (Node route_x = 0; route_x < context.num_routes; ++route_x) {
    for (Node route_y = 0; route_y < context.num_routes; ++route_y) {
      if (route_x == route_y) {
        continue;
      }
      auto &cache = operator_caches.Get(route_x, route_y);
      auto &move = *reinterpret_cast<RelocateMove *>(cache.move);
      if (!cache.TryReuse()) {
        Relocate(problem, distance_matrix, solution, context, route_x, route_y,
                 move, cache.delta, random);
      } else {
        move.route_x = route_x;
        move.route_y = route_y;
      }
      if (best_delta.Update(cache.delta, random)) {
        best_move = move;
      }
    }
  }
  if (best_delta.value < 0) {
    DoRelocate(best_move, solution, context);
    associated_routes.insert(best_move.route_x);
    associated_routes.insert(best_move.route_y);
    return true;
  }
  return false;
}

} // namespace alkaid_sd
