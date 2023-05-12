#include "sdvrp/splitils/algorithm/operators.h"

#include "sdvrp/splitils/util/solution_utils.h"

namespace vrp::sdvrp::splitils {

struct TwoOptMove {
  Node head;
  Node tail;
};

void DoTwoOpt(const TwoOptMove &move, Node route_index, Solution &solution,
              RouteContext &context) {
  Node predecessor_head = solution.predecessor(move.head);
  Node successor_tail = solution.successor(move.tail);
  ReversedLink(solution, move.head, move.tail, predecessor_head,
               successor_tail);
  if (!predecessor_head) {
    context.heads[route_index] = move.tail;
  }
  UpdateRouteContext(solution, route_index, predecessor_head, context);
}

void TwoOpt(const DistanceMatrix &distance_matrix, const Solution &solution,
            Node head, Node tail, TwoOptMove &best_move, Delta<int> &best_delta,
            Random &random) {
  Node predecessor_head = solution.predecessor(head);
  Node successor_tail = solution.successor(tail);
  int delta = distance_matrix[solution.customer(predecessor_head)]
                             [solution.customer(tail)] +
              distance_matrix[solution.customer(head)]
                             [solution.customer(successor_tail)] -
              distance_matrix[solution.customer(predecessor_head)]
                             [solution.customer(head)] -
              distance_matrix[solution.customer(tail)]
                             [solution.customer(successor_tail)];
  if (best_delta.Update(delta, random)) {
    best_move = {head, tail};
  }
}

bool TwoOpt(const Problem &problem, const DistanceMatrix &distance_matrix,
            Node route_index, Solution &solution, RouteContext &context,
            Random &random) {
  TwoOptMove best_move{};
  Delta<int> best_delta;
  Node head = context.heads[route_index];
  while (head) {
    Node tail = solution.successor(head);
    while (tail) {
      TwoOpt(distance_matrix, solution, head, tail, best_move, best_delta,
             random);
      tail = solution.successor(tail);
    }
    head = solution.successor(head);
  }
  if (best_delta.value < 0) {
    DoTwoOpt(best_move, route_index, solution, context);
    return true;
  }
  return false;
}

} // namespace vrp::sdvrp::splitils
