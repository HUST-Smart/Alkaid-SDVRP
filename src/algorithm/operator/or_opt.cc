#include "algorithm/operators.h"

#include "util/solution_utils.h"

namespace alkaid_sd {

struct OrOptMove {
  bool reversed;
  Node head;
  Node tail;
  Node predecessor;
  Node successor;
};

void DoOrOpt(const OrOptMove &move, Node route_index, Solution &solution,
             RouteContext &context) {
  Node predecessor_head = solution.predecessor(move.head);
  Node successor_tail = solution.successor(move.tail);
  solution.set_successor(0, context.heads[route_index]);
  solution.Link(predecessor_head, successor_tail);
  if (!move.reversed) {
    solution.Link(move.predecessor, move.head);
    solution.Link(move.tail, move.successor);
  } else {
    ReversedLink(solution, move.head, move.tail, move.predecessor,
                 move.successor);
  }
  context.heads[route_index] = solution.successor(0);
}

template <int num>
void OrOpt(const DistanceMatrix &distance_matrix, const Solution &solution,
           Node head, Node tail, Node predecessor, Node successor,
           OrOptMove &best_move, Delta<int> &best_delta, Random &random) {
  Node predecessor_head = solution.predecessor(head);
  Node successor_tail = solution.successor(tail);
  int delta = distance_matrix[solution.customer(predecessor_head)]
                             [solution.customer(successor_tail)] -
              distance_matrix[solution.customer(predecessor_head)]
                             [solution.customer(head)] -
              distance_matrix[solution.customer(tail)]
                             [solution.customer(successor_tail)] -
              distance_matrix[solution.customer(predecessor)]
                             [solution.customer(successor)];
  bool reversed = false;
  int insertion_delta =
      distance_matrix[solution.customer(predecessor)][solution.customer(head)] +
      distance_matrix[solution.customer(successor)][solution.customer(tail)];
  if (num > 1) {
    int reversed_delta =
        distance_matrix[solution.customer(predecessor)]
                       [solution.customer(tail)] +
        distance_matrix[solution.customer(successor)][solution.customer(head)];
    if (reversed_delta < insertion_delta) {
      insertion_delta = reversed_delta;
      reversed = true;
    }
  }
  delta += insertion_delta;
  if (best_delta.Update(delta, random)) {
    best_move = {reversed, head, tail, predecessor, successor};
  }
}

template <int num>
bool OrOpt(const Problem &problem, const DistanceMatrix &distance_matrix,
           Node route_index, Solution &solution, RouteContext &context,
           Random &random) {
  OrOptMove best_move{};
  Delta<int> best_delta;
  Node head = context.heads[route_index];
  Node tail = head;
  for (Node i = 0; tail && i < num - 1; ++i) {
    tail = solution.successor(tail);
  }
  while (tail) {
    Node predecessor, successor;
    predecessor = solution.successor(tail);
    while (predecessor) {
      successor = solution.successor(predecessor);
      OrOpt<num>(distance_matrix, solution, head, tail, predecessor, successor,
                 best_move, best_delta, random);
      predecessor = successor;
    }
    successor = solution.predecessor(head);
    while (successor) {
      predecessor = solution.predecessor(successor);
      OrOpt<num>(distance_matrix, solution, head, tail, predecessor, successor,
                 best_move, best_delta, random);
      successor = predecessor;
    }
    head = solution.successor(head);
    tail = solution.successor(tail);
  }
  if (best_delta.value < 0) {
    DoOrOpt(best_move, route_index, solution, context);
    UpdateRouteContext(solution, route_index, 0, context);
    return true;
  }
  return false;
}

template bool OrOpt<1>(const Problem &problem,
                       const DistanceMatrix &distance_matrix, Node route_index,
                       Solution &solution, RouteContext &context,
                       Random &random);

template bool OrOpt<2>(const Problem &problem,
                       const DistanceMatrix &distance_matrix, Node route_index,
                       Solution &solution, RouteContext &context,
                       Random &random);

template bool OrOpt<3>(const Problem &problem,
                       const DistanceMatrix &distance_matrix, Node route_index,
                       Solution &solution, RouteContext &context,
                       Random &random);

} // namespace alkaid_sd
