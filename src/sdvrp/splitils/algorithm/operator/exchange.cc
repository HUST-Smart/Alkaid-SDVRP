#include "sdvrp/splitils/algorithm/operators.h"

namespace vrp::sdvrp::splitils {

struct ExchangeMove {
  Node node_a;
  Node node_b;
};

void DoExchange(const ExchangeMove &move, Node route_index, Solution &solution,
                RouteContext &context) {
  Node predecessor_a = solution.predecessor(move.node_a);
  Node successor_a = solution.successor(move.node_a);
  Node predecessor_b = solution.predecessor(move.node_b);
  Node successor_b = solution.successor(move.node_b);
  solution.Link(predecessor_a, move.node_b);
  solution.Link(move.node_b, successor_a);
  solution.Link(predecessor_b, move.node_a);
  solution.Link(move.node_a, successor_b);
  if (!predecessor_a) {
    context.heads[route_index] = move.node_b;
  }
  UpdateRouteContext(solution, route_index, predecessor_a, context);
}

void Exchange(const DistanceMatrix &distance_matrix, const Solution &solution,
              Node node_a, Node node_b, ExchangeMove &best_move,
              Delta<int> &best_delta, Random &random) {
  Node predecessor_a = solution.predecessor(node_a);
  Node successor_a = solution.successor(node_a);
  Node predecessor_b = solution.predecessor(node_b);
  Node successor_b = solution.successor(node_b);
  int delta = distance_matrix[solution.customer(predecessor_a)]
                             [solution.customer(node_b)] +
              distance_matrix[solution.customer(node_b)]
                             [solution.customer(successor_a)] +
              distance_matrix[solution.customer(predecessor_b)]
                             [solution.customer(node_a)] +
              distance_matrix[solution.customer(node_a)]
                             [solution.customer(successor_b)] -
              distance_matrix[solution.customer(predecessor_a)]
                             [solution.customer(node_a)] -
              distance_matrix[solution.customer(node_a)]
                             [solution.customer(successor_a)] -
              distance_matrix[solution.customer(predecessor_b)]
                             [solution.customer(node_b)] -
              distance_matrix[solution.customer(node_b)]
                             [solution.customer(successor_b)];
  if (best_delta.Update(delta, random)) {
    best_move = {node_a, node_b};
  }
}

bool Exchange(const Problem &problem, const DistanceMatrix &distance_matrix,
              Node route_index, Solution &solution, RouteContext &context,
              Random &random) {
  ExchangeMove best_move{};
  Delta<int> best_delta;
  Node node_a = context.heads[route_index];
  while (node_a) {
    Node node_b = solution.successor(node_a);
    if (node_b) {
      node_b = solution.successor(node_b);
      while (node_b) {
        Exchange(distance_matrix, solution, node_a, node_b, best_move,
                 best_delta, random);
        node_b = solution.successor(node_b);
      }
    }
    node_a = solution.successor(node_a);
  }
  if (best_delta.value < 0) {
    DoExchange(best_move, route_index, solution, context);
    return true;
  }
  return false;
}

} // namespace vrp::sdvrp::splitils
