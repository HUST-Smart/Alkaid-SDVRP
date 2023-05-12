#include "sdvrp/splitils/algorithm/operators.h"

#include "sdvrp/splitils/util/solution_utils.h"

namespace vrp::sdvrp::splitils {

struct SdSwapOneInMove {
  bool direction_one;
  bool reversed_in;
  Node route_one;
  Node node_one;
  Node route_in;
  Node head_in;
  Node tail_in;
  int one_split_load;
};

void DoSdSwapOneIn(const SdSwapOneInMove &move, Solution &solution,
                   RouteContext &context) {
  Node node_one = move.node_one;
  Node head_in = move.head_in;
  Node tail_in = move.tail_in;
  Node predecessor_one = solution.predecessor(node_one);
  Node successor_one = solution.successor(node_one);
  Node predecessor_head_in = solution.predecessor(head_in);
  Node successor_tail_in = solution.successor(tail_in);
  Node split_index =
      solution.Insert(solution.customer(node_one),
                      solution.load(node_one) - move.one_split_load,
                      predecessor_head_in, successor_tail_in);
  if (!predecessor_head_in) {
    context.heads[move.route_in] = split_index;
  }
  solution.set_load(node_one, move.one_split_load);
  Node predecessor, successor;
  if (!move.direction_one) {
    predecessor = predecessor_one;
    successor = node_one;
  } else {
    predecessor = node_one;
    successor = successor_one;
  }
  if (move.direction_one ^ move.reversed_in) {
    ReversedLink(solution, head_in, tail_in, predecessor, successor);
  } else {
    solution.Link(predecessor, head_in);
    solution.Link(tail_in, successor);
  }
  if (!predecessor) {
    context.heads[move.route_one] = solution.successor(0);
  }
}

inline void SdSwapOneIn(const Problem &problem,
                        const DistanceMatrix &distance_matrix,
                        const Solution &solution, const RouteContext &context,
                        Node route_one, Node node_one, Node route_in,
                        Node head_in, Node tail_in, int one_split_load,
                        SdSwapOneInMove &best_move, Delta<int> &best_delta,
                        Random &random) {
  Node customer_node_one = solution.customer(node_one);
  Node customer_head_in = solution.customer(head_in);
  Node customer_tail_in = solution.customer(tail_in);
  Node predecessor_one = solution.customer(solution.predecessor(node_one));
  Node successor_one = solution.customer(solution.successor(node_one));
  Node predecessor_head_in = solution.customer(solution.predecessor(head_in));
  Node successor_tail_in = solution.customer(solution.successor(tail_in));
  int delta = distance_matrix[customer_node_one][predecessor_head_in] +
              distance_matrix[customer_node_one][successor_tail_in] -
              distance_matrix[customer_head_in][predecessor_head_in] -
              distance_matrix[customer_tail_in][successor_tail_in];
  bool best_direction_one, best_reversed_in;
  int min_delta = std::numeric_limits<int>::max();
  for (bool direction_one : {false, true}) {
    int link_delta = -distance_matrix[customer_node_one][predecessor_one];
    for (bool reversed_in : {false, true}) {
      int current_delta = distance_matrix[customer_head_in][predecessor_one] +
                          distance_matrix[customer_node_one][customer_tail_in] +
                          link_delta;
      if (current_delta < min_delta) {
        min_delta = current_delta;
        best_direction_one = direction_one;
        best_reversed_in = reversed_in;
      }
      std::swap(customer_head_in, customer_tail_in);
    }
    std::swap(predecessor_one, successor_one);
  }
  delta += min_delta;
  if (best_delta.Update(delta, random)) {
    best_move = {
        best_direction_one, best_reversed_in, route_one, node_one,
        route_in,           head_in,          tail_in,   one_split_load};
  }
}

void SdSwapOneIn(const Problem &problem, const DistanceMatrix &distance_matrix,
                 const Solution &solution, const RouteContext &context,
                 Node route_one, Node node_one, Node route_in,
                 SdSwapOneInMove &best_move, Delta<int> &best_delta,
                 Random &random) {
  Load load_one = solution.load(node_one);
  Node head_in = context.heads[route_in];
  while (head_in) {
    Load load_in = solution.load(head_in);
    Node tail_in = head_in;
    while (tail_in && load_in < load_one) {
      SdSwapOneIn(problem, distance_matrix, solution, context, route_one,
                  node_one, route_in, head_in, tail_in, load_one - load_in,
                  best_move, best_delta, random);
      tail_in = solution.successor(tail_in);
      load_in += solution.load(tail_in);
    }
    head_in = solution.successor(head_in);
  }
}

bool SdSwapOneIn(const Problem &problem, const DistanceMatrix &distance_matrix,
                 Solution &solution, RouteContext &context,
                 std::set<Node> &associated_routes, Random &random,
                 OperatorCaches &operator_caches) {
  SdSwapOneInMove best_move{};
  Delta<int> best_delta;
  for (Node route_one = 0; route_one < context.num_routes; ++route_one) {
    for (Node route_in = 0; route_in < context.num_routes; ++route_in) {
      if (route_one == route_in) {
        continue;
      }
      auto &cache = operator_caches.Get(route_one, route_in);
      auto &move = *reinterpret_cast<SdSwapOneInMove *>(cache.move);
      if (!cache.TryReuse()) {
        for (Node node_one = context.heads[route_one]; node_one;
             node_one = solution.successor(node_one)) {
          SdSwapOneIn(problem, distance_matrix, solution, context, route_one,
                      node_one, route_in, move, cache.delta, random);
        }
      } else {
        move.route_one = route_one;
        move.route_in = route_in;
      }
      if (best_delta.Update(cache.delta, random)) {
        best_move = move;
      }
    }
  }
  if (best_delta.value < 0) {
    DoSdSwapOneIn(best_move, solution, context);
    associated_routes.insert(best_move.route_one);
    associated_routes.insert(best_move.route_in);
    return true;
  }
  return false;
}

} // namespace vrp::sdvrp::splitils
