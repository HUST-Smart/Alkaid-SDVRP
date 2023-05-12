#include "sdvrp/splitils/algorithm/operators.h"

#include "sdvrp/splitils/util/solution_utils.h"

namespace vrp::sdvrp::splitils {

struct CrossMove {
  bool reversed;
  Node route_x, route_y;
  Node left_x, left_y;
};

void DoCross(const CrossMove &move, Solution &solution, RouteContext &context) {
  Node right_x = move.left_x ? solution.successor(move.left_x)
                             : context.heads[move.route_x];
  Node right_y = move.left_y ? solution.successor(move.left_y)
                             : context.heads[move.route_y];
  if (!move.reversed) {
    solution.Link(move.left_x, right_y);
    solution.Link(move.left_y, right_x);
    if (move.left_x == 0) {
      context.heads[move.route_x] = right_y;
    }
    if (move.left_y == 0) {
      context.heads[move.route_y] = right_x;
    }
  } else {
    Node head_y = context.heads[move.route_y];
    if (right_x) {
      Node tail_x = context.tails[move.route_x];
      ReversedLink(solution, right_x, tail_x, 0, right_y);
      context.heads[move.route_y] = tail_x;
    } else {
      solution.Link(0, right_y);
      context.heads[move.route_y] = right_y;
    }
    solution.set_successor(0, context.heads[move.route_x]);
    if (move.left_y) {
      ReversedLink(solution, head_y, move.left_y, move.left_x, 0);
    } else {
      solution.Link(move.left_x, 0);
    }
    context.heads[move.route_x] = solution.successor(0);
  }
}

void Cross(const Problem &problem, const DistanceMatrix &distance_matrix,
           const Solution &solution, const RouteContext &context, Node route_x,
           Node route_y, CrossMove &best_move, Delta<int> &best_delta,
           Random &random) {
  Node left_x = 0;
  do {
    Node successor_x =
        left_x ? solution.successor(left_x) : context.heads[route_x];
    Node left_y = 0;
    do {
      Node predecessor_y = left_y;
      Node successor_y =
          left_y ? solution.successor(left_y) : context.heads[route_y];
      int predecessor_load_x = context.loads[left_x];
      int successor_load_x = context.route_loads[route_x] - predecessor_load_x;
      int predecessor_load_y = context.loads[left_y];
      int successor_load_y = context.route_loads[route_y] - predecessor_load_y;
      int base = -distance_matrix[solution.customer(left_x)]
                                 [solution.customer(successor_x)] -
                 distance_matrix[solution.customer(left_y)]
                                [solution.customer(successor_y)];
      for (bool reversed : {false, true}) {
        if (predecessor_load_x + successor_load_y <= problem.capacity &&
            successor_load_x + predecessor_load_y <= problem.capacity) {
          int delta = base +
                      distance_matrix[solution.customer(left_x)]
                                     [solution.customer(successor_y)] +
                      distance_matrix[solution.customer(successor_x)]
                                     [solution.customer(predecessor_y)];
          if (best_delta.Update(delta, random)) {
            best_move = {reversed, route_x, route_y, left_x, left_y};
          }
        }
        std::swap(predecessor_y, successor_y);
        std::swap(predecessor_load_y, successor_load_y);
      }
      left_y = successor_y;
    } while (left_y);
    left_x = successor_x;
  } while (left_x);
}

bool Cross(const Problem &problem, const DistanceMatrix &distance_matrix,
           Solution &solution, RouteContext &context,
           std::set<Node> &associated_routes, Random &random,
           OperatorCaches &operator_caches) {
  CrossMove best_move{};
  Delta<int> best_delta;
  for (Node route_x = 0; route_x < context.num_routes; ++route_x) {
    for (Node route_y = route_x + 1; route_y < context.num_routes; ++route_y) {
      auto &cache = operator_caches.Get(route_x, route_y);
      auto &move = *reinterpret_cast<CrossMove *>(cache.move);
      if (!cache.TryReuse()) {
        Cross(problem, distance_matrix, solution, context, route_x, route_y,
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
    DoCross(best_move, solution, context);
    associated_routes.insert(best_move.route_x);
    associated_routes.insert(best_move.route_y);
    return true;
  }
  return false;
}

} // namespace vrp::sdvrp::splitils
