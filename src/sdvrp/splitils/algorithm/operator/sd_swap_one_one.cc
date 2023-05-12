#include "sdvrp/splitils/algorithm/operators.h"

#include "sdvrp/splitils/algorithm/base_star.h"
#include "sdvrp/splitils/model/route_head_guard.h"
#include "sdvrp/splitils/util/solution_utils.h"

namespace vrp::sdvrp::splitils {

struct SdSwapOneOneMove {
  bool swapped;
  Node route_x, route_y;
  Node node_x, predecessor_x, successor_x;
  Node node_y, predecessor_y, successor_y;
  int split_load;
};

void DoSdSwapOneOne(const SdSwapOneOneMove &move, Solution &solution,
                    RouteContext &context) {
  Node predecessor_y = solution.predecessor(move.node_y);
  Node successor_y = solution.successor(move.node_y);
  {
    RouteHeadGuard guard(solution, context, move.route_x);
    solution.set_load(move.node_x, move.split_load);
    solution.Link(move.predecessor_y, move.node_y);
    solution.Link(move.node_y, move.successor_y);
  }
  {
    RouteHeadGuard guard(solution, context, move.route_y);
    solution.Link(predecessor_y, successor_y);
    solution.Insert(solution.customer(move.node_x), solution.load(move.node_y),
                    move.predecessor_x, move.successor_x);
  }
}

inline void SdSwapOneOne(const Problem &problem,
                         const DistanceMatrix &distance_matrix,
                         const Solution &solution, const RouteContext &context,
                         bool swapped, Node route_x, Node route_y, Node node_x,
                         Node node_y, int split_load,
                         SdSwapOneOneMove &best_move, Delta<int> &best_delta,
                         Random &random) {
  Node predecessor_x = solution.predecessor(node_x);
  Node successor_x = solution.successor(node_x);
  Node predecessor_y = solution.predecessor(node_y);
  Node successor_y = solution.successor(node_y);
  int delta =
      -CalcDelta(distance_matrix, solution, node_y, predecessor_y, successor_y);
  int delta_x =
      CalcDelta(distance_matrix, solution, node_x, predecessor_y, successor_y);
  int before =
      CalcDelta(distance_matrix, solution, node_y, predecessor_x, node_x);
  int after = CalcDelta(distance_matrix, solution, node_y, node_x, successor_x);
  int delta_y;
  Node predecessor;
  Node successor;
  if (before <= after) {
    predecessor = predecessor_x;
    successor = node_x;
  } else {
    predecessor = node_x;
    successor = successor_x;
  }
  delta += delta_x + delta_y;
  if (best_delta.Update(delta, random)) {
    best_move = {swapped,     route_x, route_y,     node_x,    predecessor_y,
                 successor_y, node_y,  predecessor, successor, split_load};
  }
}

void SdSwapOneOne(const Problem &problem, const DistanceMatrix &distance_matrix,
                  const Solution &solution, const RouteContext &context,
                  Node route_x, Node route_y, SdSwapOneOneMove &best_move,
                  Delta<int> &best_delta, Random &random) {
  for (Node node_x = context.heads[route_x]; node_x;
       node_x = solution.successor(node_x)) {
    Load load_x = solution.load(node_x);
    for (Node node_y = context.heads[route_y]; node_y;
         node_y = solution.successor(node_y)) {
      Load load_y = solution.load(node_y);
      if (load_x > load_y) {
        SdSwapOneOne(problem, distance_matrix, solution, context, false,
                     route_x, route_y, node_x, node_y, load_x - load_y,
                     best_move, best_delta, random);
      } else if (load_y > load_x) {
        SdSwapOneOne(problem, distance_matrix, solution, context, true, route_y,
                     route_x, node_y, node_x, load_y - load_x, best_move,
                     best_delta, random);
      }
    }
  }
}

bool SdSwapOneOne(const Problem &problem, const DistanceMatrix &distance_matrix,
                  Solution &solution, RouteContext &context,
                  std::set<Node> &associated_routes, Random &random,
                  OperatorCaches &operator_caches) {
  SdSwapOneOneMove best_move{};
  Delta<int> best_delta;
  for (Node route_x = 0; route_x < context.num_routes; ++route_x) {
    for (Node route_y = route_x + 1; route_y < context.num_routes; ++route_y) {
      auto &cache = operator_caches.Get(route_x, route_y);
      auto &move = *reinterpret_cast<SdSwapOneOneMove *>(cache.move);
      if (!cache.TryReuse()) {
        SdSwapOneOne(problem, distance_matrix, solution, context, route_x,
                     route_y, move, cache.delta, random);
      } else {
        if (!move.swapped) {
          move.route_x = route_x;
          move.route_y = route_y;
        } else {
          move.route_x = route_y;
          move.route_y = route_x;
        }
      }
      if (best_delta.Update(cache.delta, random)) {
        best_move = move;
      }
    }
  }
  if (best_delta.value < 0) {
    DoSdSwapOneOne(best_move, solution, context);
    associated_routes.insert(best_move.route_x);
    associated_routes.insert(best_move.route_y);
    return true;
  }
  return false;
}

} // namespace vrp::sdvrp::splitils
