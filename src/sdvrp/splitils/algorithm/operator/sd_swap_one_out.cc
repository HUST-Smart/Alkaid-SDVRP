#include "sdvrp/splitils/algorithm/operators.h"

#include "sdvrp/splitils/model/direction.h"

namespace vrp::sdvrp::splitils {

struct SdSwapOneOutMove {
  bool direction_out;
  bool reversed_out;
  bool before_tail_out;
  Node route_one;
  Node node_one;
  Node route_out;
  Node head_out;
  Node tail_out;
  int tail_split_load;
};

template <class DirectionOut>
void DoSdSwapOneOut(const SdSwapOneOutMove &move, Solution &solution,
                    RouteContext &context) {
  Node tail_out = move.tail_out;
  Node predecessor_one = solution.predecessor(move.node_one);
  Node successor_one = solution.successor(move.node_one);
  Node predecessor_head_out =
      DirectionOut::Predecessor(solution, move.head_out);
  Node successor_tail_out = DirectionOut::Successor(solution, tail_out);
  Node split_index =
      solution.NewNode(solution.customer(tail_out), move.tail_split_load);
  solution.set_load(tail_out, solution.load(tail_out) - move.tail_split_load);
  Node predecessor = predecessor_one;
  if (!move.reversed_out) {
    for (Node node_index = move.head_out; node_index != successor_tail_out;) {
      Node next_node_index = DirectionOut::Successor(solution, node_index);
      solution.Link(predecessor, node_index);
      predecessor = node_index;
      node_index = next_node_index;
    }
  } else {
    for (Node node_index = move.tail_out; node_index != predecessor_head_out;) {
      Node next_node_index = DirectionOut::Predecessor(solution, node_index);
      solution.Link(predecessor, node_index);
      predecessor = node_index;
      node_index = next_node_index;
    }
  }
  solution.Link(predecessor, successor_one);
  if (!predecessor_one) {
    context.heads[move.route_one] = solution.successor(0);
  }
  if (move.before_tail_out) {
    DirectionOut::Link(predecessor_head_out, move.node_one, solution);
    DirectionOut::Link(move.node_one, split_index, solution);
    DirectionOut::Link(split_index, successor_tail_out, solution);
  } else {
    DirectionOut::Link(predecessor_head_out, split_index, solution);
    DirectionOut::Link(split_index, move.node_one, solution);
    DirectionOut::Link(move.node_one, successor_tail_out, solution);
  }
  Node predecessor_out =
      move.direction_out ? successor_tail_out : predecessor_head_out;
  if (!predecessor_out) {
    context.heads[move.route_out] = solution.successor(0);
  }
}

template <class DirectionOut>
inline void
SdSwapOneOut(const Problem &problem, const DistanceMatrix &distance_matrix,
             const Solution &solution, const RouteContext &context,
             Node route_one, Node node_one, Node route_out, Node head_out,
             Node tail_out, int tail_split_load, SdSwapOneOutMove &best_move,
             Delta<int> &best_delta, Random &random) {
  Node customer_node_one = solution.customer(node_one);
  Node customer_head_out = solution.customer(head_out);
  Node customer_tail_out = solution.customer(tail_out);
  Node predecessor_one = solution.customer(solution.predecessor(node_one));
  Node successor_one = solution.customer(solution.successor(node_one));
  Node predecessor_head_out =
      solution.customer(DirectionOut::Predecessor(solution, head_out));
  Node successor_tail_out =
      solution.customer(DirectionOut::Successor(solution, tail_out));
  int delta = -distance_matrix[customer_node_one][predecessor_one] -
              distance_matrix[customer_node_one][successor_one] -
              distance_matrix[customer_head_out][predecessor_head_out] +
              distance_matrix[customer_node_one][customer_tail_out];
  int non_reversed_delta = distance_matrix[customer_head_out][predecessor_one] +
                           distance_matrix[customer_tail_out][successor_one];
  int reversed_delta = distance_matrix[customer_tail_out][predecessor_one] +
                       distance_matrix[customer_head_out][successor_one];
  bool reversed_out;
  if (non_reversed_delta <= reversed_delta) {
    delta += non_reversed_delta;
    reversed_out = false;
  } else {
    delta += reversed_delta;
    reversed_out = true;
  }
  int before_tail_delta =
      distance_matrix[customer_node_one][predecessor_head_out];
  int after_tail_delta =
      distance_matrix[customer_tail_out][predecessor_head_out] +
      distance_matrix[customer_node_one][successor_tail_out] -
      distance_matrix[customer_tail_out][successor_tail_out];
  bool before_tail_out;
  if (before_tail_delta <= after_tail_delta) {
    delta += before_tail_delta;
    before_tail_out = true;
  } else {
    delta += after_tail_delta;
    before_tail_out = false;
  }
  if (best_delta.Update(delta, random)) {
    best_move = {DirectionOut::kDirection,
                 reversed_out,
                 before_tail_out,
                 route_one,
                 node_one,
                 route_out,
                 head_out,
                 tail_out,
                 tail_split_load};
  }
}

template <class DirectionOut>
void SdSwapOneOut(const Problem &problem, const DistanceMatrix &distance_matrix,
                  const Solution &solution, const RouteContext &context,
                  Node route_one, Node node_one, Node route_out,
                  SdSwapOneOutMove &best_move, Delta<int> &best_delta,
                  Random &random) {
  Load load_one = solution.load(node_one);
  Node head_out = DirectionOut::Head(context, route_out);
  Node tail_out = head_out;
  Load load_out = solution.load(head_out);
  while (true) {
    while (load_out < load_one) {
      tail_out = DirectionOut::Successor(solution, tail_out);
      if (!tail_out) {
        return;
      }
      load_out += solution.load(tail_out);
    }
    int tail_split_load = load_out - load_one;
    if (tail_split_load > 0) {
      SdSwapOneOut<DirectionOut>(problem, distance_matrix, solution, context,
                                 route_one, node_one, route_out, head_out,
                                 tail_out, tail_split_load, best_move,
                                 best_delta, random);
    }
    load_out -= solution.load(head_out);
    head_out = DirectionOut::Successor(solution, head_out);
  }
}

bool SdSwapOneOut(const Problem &problem, const DistanceMatrix &distance_matrix,
                  Solution &solution, RouteContext &context,
                  std::set<Node> &associated_routes, Random &random,
                  OperatorCaches &operator_caches) {
  SdSwapOneOutMove best_move{};
  Delta<int> best_delta;
  for (Node route_one = 0; route_one < context.num_routes; ++route_one) {
    for (Node route_out = 0; route_out < context.num_routes; ++route_out) {
      if (route_one == route_out) {
        continue;
      }
      auto &cache = operator_caches.Get(route_one, route_out);
      auto &move = *reinterpret_cast<SdSwapOneOutMove *>(cache.move);
      if (!cache.TryReuse()) {
        for (Node node_one = context.heads[route_one]; node_one;
             node_one = solution.successor(node_one)) {
          SdSwapOneOut<Forward>(problem, distance_matrix, solution, context,
                                route_one, node_one, route_out, move,
                                cache.delta, random);
          SdSwapOneOut<Backward>(problem, distance_matrix, solution, context,
                                 route_one, node_one, route_out, move,
                                 cache.delta, random);
        }
      } else {
        move.route_one = route_one;
        move.route_out = route_out;
      }
      if (best_delta.Update(cache.delta, random)) {
        best_move = move;
      }
    }
  }
  if (best_delta.value < 0) {
    if (!best_move.direction_out) {
      DoSdSwapOneOut<Forward>(best_move, solution, context);
    } else {
      DoSdSwapOneOut<Backward>(best_move, solution, context);
    }
    associated_routes.insert(best_move.route_one);
    associated_routes.insert(best_move.route_out);
    return true;
  }
  return false;
}

} // namespace vrp::sdvrp::splitils
