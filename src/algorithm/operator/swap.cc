#include "algorithm/operators.h"

#include "util/solution_utils.h"

namespace alkaid_sd {

struct SwapMove {
  Node route_x, route_y;
  int direction_x, direction_y;
  Node left_x, left_y;
  Node right_x, right_y;
};

inline void SegmentInsertion(Solution &solution, RouteContext &context,
                             Node left, Node right, Node predecessor,
                             Node successor, Node route_index, int direction) {
  if (direction) {
    ReversedLink(solution, left, right, predecessor, successor);
  } else {
    solution.Link(predecessor, left);
    solution.Link(right, successor);
  }
  if (predecessor == 0) {
    context.heads[route_index] = direction ? right : left;
  }
}

inline void DoSwap(SwapMove &move, Solution &solution, RouteContext &context) {
  if (move.direction_y == -1) {
    Node predecessor = solution.predecessor(move.left_x);
    Node successor = solution.successor(move.right_x);
    solution.Link(predecessor, successor);
    if (predecessor == 0) {
      context.heads[move.route_x] = successor;
    }
    SegmentInsertion(solution, context, move.left_x, move.right_x, move.left_y,
                     move.right_y, move.route_y, move.direction_x);
  } else {
    Node predecessor_x = solution.predecessor(move.left_x);
    Node successor_x = solution.successor(move.right_x);
    Node predecessor_y = solution.predecessor(move.left_y);
    Node successor_y = solution.successor(move.right_y);
    SegmentInsertion(solution, context, move.left_x, move.right_x,
                     predecessor_y, successor_y, move.route_y,
                     move.direction_x);
    SegmentInsertion(solution, context, move.left_y, move.right_y,
                     predecessor_x, successor_x, move.route_x,
                     move.direction_y);
  }
}

inline void UpdateShift(const DistanceMatrix &distance_matrix,
                        const Solution &solution, Node route_x, Node route_y,
                        Node left, Node right, Node predecessor, Node successor,
                        Node base_x, SwapMove &best_move,
                        Delta<int> &best_delta, Random &random) {
  Node customer_left = solution.customer(left);
  Node customer_predecessor = solution.customer(predecessor);
  Node customer_right = solution.customer(right);
  Node customer_successor = solution.customer(successor);
  int d1 = distance_matrix[customer_left][customer_predecessor] +
           distance_matrix[customer_right][customer_successor];
  int d2 = distance_matrix[customer_left][customer_successor] +
           distance_matrix[customer_right][customer_predecessor];
  int direction = d1 >= d2;
  int delta = base_x + (direction ? d2 : d1) -
              distance_matrix[customer_predecessor][customer_successor];
  if (best_delta.Update(delta, random)) {
    best_move = {route_x, route_y,     direction, -1,
                 left,    predecessor, right,     successor};
  }
}

inline void UpdateSwap(const DistanceMatrix &distance_matrix,
                       const Solution &solution, Node route_x, Node route_y,
                       Node left_x, Node right_x, Node left_y, Node right_y,
                       int base_x, SwapMove &best_move, Delta<int> &best_delta,
                       Random &random) {
  Node customer_left_x = solution.customer(left_x);
  Node customer_right_x = solution.customer(right_x);
  Node customer_left_y = solution.customer(left_y);
  Node customer_right_y = solution.customer(right_y);
  Node predecessor_x = solution.customer(solution.predecessor(left_x));
  Node successor_x = solution.customer(solution.successor(right_x));
  Node predecessor_y = solution.customer(solution.predecessor(left_y));
  Node successor_y = solution.customer(solution.successor(right_y));
  int d1 = distance_matrix[customer_left_x][predecessor_y] +
           distance_matrix[customer_right_x][successor_y];
  int d2 = distance_matrix[customer_left_x][successor_y] +
           distance_matrix[customer_right_x][predecessor_y];
  int d3 = distance_matrix[customer_left_y][predecessor_x] +
           distance_matrix[customer_right_y][successor_x];
  int d4 = distance_matrix[customer_left_y][successor_x] +
           distance_matrix[customer_right_y][predecessor_x];
  int direction_x = d1 >= d2;
  int direction_y = d3 >= d4;
  int delta = base_x + (direction_x ? d2 : d1) + (direction_y ? d4 : d3) -
              distance_matrix[customer_left_y][predecessor_y] -
              distance_matrix[customer_right_y][successor_y];
  if (best_delta.Update(delta, random)) {
    best_move = {route_x, route_y, direction_x, direction_y,
                 left_x,  left_y,  right_x,     right_y};
  }
}

template <int num_x, int num_y>
void Swap(const Problem &problem, const DistanceMatrix &distance_matrix,
          Solution &solution, RouteContext &context, Node route_x, Node route_y,
          SwapMove &best_move, Delta<int> &best_delta, Random &random) {
  Node left_x = context.heads[route_x];
  Load load_x = solution.load(left_x);
  Node right_x = left_x;
  for (int i = 1; right_x && i < num_x; ++i) {
    right_x = solution.successor(right_x);
    load_x += solution.load(right_x);
  }
  while (right_x) {
    int base_x =
        -distance_matrix[solution.customer(left_x)]
                        [solution.customer(solution.predecessor(left_x))] -
        distance_matrix[solution.customer(right_x)]
                       [solution.customer(solution.successor(right_x))];
    if (num_y == 0) {
      base_x += distance_matrix[solution.customer(solution.predecessor(left_x))]
                               [solution.customer(solution.successor(right_x))];
    }
    Load load_y_lower =
        -problem.capacity + context.route_loads[route_y] + load_x;
    if (num_y == 0) {
      if (load_y_lower <= 0) {
        Node predecessor = 0;
        Node successor = context.heads[route_y];
        while (true) {
          UpdateShift(distance_matrix, solution, route_x, route_y, left_x,
                      right_x, predecessor, successor, base_x, best_move,
                      best_delta, random);
          if (!successor) {
            break;
          }
          predecessor = successor;
          successor = solution.successor(successor);
        }
      }
    } else {
      Load load_y_upper =
          problem.capacity - context.route_loads[route_x] + load_x;
      Node left_y = context.heads[route_y];
      Load load_y = solution.load(left_y);
      Node right_y = left_y;
      for (int i = 1; right_y && i < num_y; ++i) {
        right_y = solution.successor(right_y);
        load_y += solution.load(right_y);
      }
      while (right_y) {
        if (load_y >= load_y_lower && load_y <= load_y_upper) {
          UpdateSwap(distance_matrix, solution, route_x, route_y, left_x,
                     right_x, left_y, right_y, base_x, best_move, best_delta,
                     random);
        }
        load_y -= solution.load(left_y);
        left_y = solution.successor(left_y);
        right_y = solution.successor(right_y);
        load_y += solution.load(right_y);
      }
    }
    load_x -= solution.load(left_x);
    left_x = solution.successor(left_x);
    right_x = solution.successor(right_x);
    load_x += solution.load(right_x);
  }
}

template <int num_x, int num_y>
bool Swap(const Problem &problem, const DistanceMatrix &distance_matrix,
          Solution &solution, RouteContext &context,
          std::set<Node> &associated_routes, Random &random,
          OperatorCaches &operator_caches) {
  SwapMove best_move{};
  Delta<int> best_delta;
  for (Node route_x = 0; route_x < context.num_routes; ++route_x) {
    for (Node route_y = num_x != num_y ? 0 : route_x + 1;
         route_y < context.num_routes; ++route_y) {
      if (num_x != num_y && route_x == route_y) {
        continue;
      }
      auto &cache = operator_caches.Get(route_x, route_y);
      auto &move = *reinterpret_cast<SwapMove *>(cache.move);
      if (!cache.TryReuse()) {
        Swap<num_x, num_y>(problem, distance_matrix, solution, context, route_x,
                           route_y, move, cache.delta, random);
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
    DoSwap(best_move, solution, context);
    associated_routes.insert(best_move.route_x);
    associated_routes.insert(best_move.route_y);
    return true;
  }
  return false;
}

template bool Swap<1, 0>(const Problem &problem,
                         const DistanceMatrix &distance_matrix,
                         Solution &solution, RouteContext &context,
                         std::set<Node> &associated_routes, Random &random,
                         OperatorCaches &operator_caches);

template bool Swap<2, 0>(const Problem &problem,
                         const DistanceMatrix &distance_matrix,
                         Solution &solution, RouteContext &context,
                         std::set<Node> &associated_routes, Random &random,
                         OperatorCaches &operator_caches);

template bool Swap<1, 1>(const Problem &problem,
                         const DistanceMatrix &distance_matrix,
                         Solution &solution, RouteContext &context,
                         std::set<Node> &associated_routes, Random &random,
                         OperatorCaches &operator_caches);

template bool Swap<2, 1>(const Problem &problem,
                         const DistanceMatrix &distance_matrix,
                         Solution &solution, RouteContext &context,
                         std::set<Node> &associated_routes, Random &random,
                         OperatorCaches &operator_caches);

template bool Swap<2, 2>(const Problem &problem,
                         const DistanceMatrix &distance_matrix,
                         Solution &solution, RouteContext &context,
                         std::set<Node> &associated_routes, Random &random,
                         OperatorCaches &operator_caches);

} // namespace alkaid_sd
