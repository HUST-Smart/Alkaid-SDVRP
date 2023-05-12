#include "sdvrp/splitils/algorithm/operators.h"

#include "sdvrp/splitils/algorithm/base_star.h"
#include "sdvrp/splitils/model/route_head_guard.h"

namespace vrp::sdvrp::splitils {

struct SwapStarMove {
  Node route_x, route_y;
  Node node_x, predecessor_x, successor_x;
  Node node_y, predecessor_y, successor_y;
};

inline void DoSwapStar(SwapStarMove &move, Solution &solution,
                       RouteContext &context) {
  Node predecessor_x = solution.predecessor(move.node_x);
  Node successor_x = solution.successor(move.node_x);
  Node predecessor_y = solution.predecessor(move.node_y);
  Node successor_y = solution.successor(move.node_y);
  {
    RouteHeadGuard guard(solution, context, move.route_x);
    solution.Link(predecessor_x, successor_x);
    solution.Link(move.predecessor_y, move.node_y);
    solution.Link(move.node_y, move.successor_y);
  }
  {
    RouteHeadGuard guard(solution, context, move.route_y);
    solution.Link(predecessor_y, successor_y);
    solution.Link(move.predecessor_x, move.node_x);
    solution.Link(move.node_x, move.successor_x);
  }
}

void SwapStar(const Problem &problem, const DistanceMatrix &distance_matrix,
              const Solution &solution, const RouteContext &context,
              Node route_x, Node route_y, SwapStarMove &best_move,
              Delta<int> &best_delta, Random &random) {
  PreprocessStarInsertions(problem, distance_matrix, solution, context, route_x,
                           random);
  PreprocessStarInsertions(problem, distance_matrix, solution, context, route_y,
                           random);
  Node node_x = context.heads[route_x];
  while (node_x) {
    auto &&insertion_x = star_caches[route_y][solution.customer(node_x)];
    Load load_x = solution.load(node_x);
    Load load_y_lower =
        -problem.capacity + context.route_loads[route_y] + load_x;
    Load load_y_upper =
        problem.capacity - context.route_loads[route_x] + load_x;
    Node node_y = context.heads[route_y];
    while (node_y) {
      Load load_y = solution.load(node_y);
      if (load_y >= load_y_lower && load_y <= load_y_upper) {
        auto &&insertion_y = star_caches[route_x][solution.customer(node_y)];
        Node predecessor_x = solution.predecessor(node_x);
        Node successor_x = solution.successor(node_x);
        Node predecessor_y = solution.predecessor(node_y);
        Node successor_y = solution.successor(node_y);
        int delta = -CalcDelta(distance_matrix, solution, node_x, predecessor_x,
                               successor_x) -
                    CalcDelta(distance_matrix, solution, node_y, predecessor_y,
                              successor_y);
        int delta_x = CalcDelta(distance_matrix, solution, node_x,
                                predecessor_y, successor_y);
        int delta_y = CalcDelta(distance_matrix, solution, node_y,
                                predecessor_x, successor_x);
        auto best_insertion_x = insertion_x.FindBestWithoutNode(node_y);
        if (best_insertion_x && best_insertion_x->delta.value < delta_x) {
          delta_x = best_insertion_x->delta.value;
          predecessor_y = best_insertion_x->predecessor;
          successor_y = best_insertion_x->successor;
        }
        auto best_insertion_y = insertion_y.FindBestWithoutNode(node_x);
        if (best_insertion_y && best_insertion_y->delta.value < delta_y) {
          delta_y = best_insertion_y->delta.value;
          predecessor_x = best_insertion_y->predecessor;
          successor_x = best_insertion_y->successor;
        }
        delta += delta_x + delta_y;
        if (best_delta.Update(delta, random)) {
          best_move = {route_x,     route_y, node_x,        predecessor_y,
                       successor_y, node_y,  predecessor_x, successor_x};
        }
      }
      node_y = solution.successor(node_y);
    }
    node_x = solution.successor(node_x);
  }
}

bool SwapStar(const Problem &problem, const DistanceMatrix &distance_matrix,
              Solution &solution, RouteContext &context,
              std::set<Node> &associated_routes, Random &random,
              OperatorCaches &operator_caches) {
  SwapStarMove best_move{};
  Delta<int> best_delta;
  for (Node route_x = 0; route_x < context.num_routes; ++route_x) {
    for (Node route_y = route_x + 1; route_y < context.num_routes; ++route_y) {
      auto &cache = operator_caches.Get(route_x, route_y);
      auto &move = *reinterpret_cast<SwapStarMove *>(cache.move);
      if (!cache.TryReuse()) {
        SwapStar(problem, distance_matrix, solution, context, route_x, route_y,
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
    DoSwapStar(best_move, solution, context);
    associated_routes.insert(best_move.route_x);
    associated_routes.insert(best_move.route_y);
    return true;
  }
  return false;
}

} // namespace vrp::sdvrp::splitils
