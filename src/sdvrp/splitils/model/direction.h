#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_DIRECTION_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_DIRECTION_H_

namespace vrp::sdvrp::splitils {

struct Forward {
  static constexpr bool kDirection = false;
  static Node Head(const RouteContext &context, Node node_index) {
    return context.heads[node_index];
  }
  static Node Predecessor(const Solution &solution, Node node_index) {
    return solution.predecessor(node_index);
  }
  static Node Successor(const Solution &solution, Node node_index) {
    return solution.successor(node_index);
  }
  static void Link(Node predecessor, Node successor, Solution &solution) {
    solution.Link(predecessor, successor);
  }
};

struct Backward {
  static constexpr bool kDirection = true;
  static Node Head(const RouteContext &context, Node node_index) {
    return context.tails[node_index];
  }
  static Node Predecessor(const Solution &solution, Node node_index) {
    return solution.successor(node_index);
  }
  static Node Successor(const Solution &solution, Node node_index) {
    return solution.predecessor(node_index);
  }
  static void Link(Node predecessor, Node successor, Solution &solution) {
    solution.Link(successor, predecessor);
  }
};

} // namespace vrp::sdvrp::splitils

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_DIRECTION_H_
