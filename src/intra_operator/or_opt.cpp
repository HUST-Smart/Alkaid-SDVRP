#include <alkaidsd/intra_operator.h>

#include "../delta.h"
#include "../route_context.h"

namespace alkaidsd::intra_operator {
  struct OrOptMove {
    bool reversed;
    Node head;
    Node tail;
    Node predecessor;
    Node successor;
  };

  void DoOrOpt(const OrOptMove &move, Node route_index, AlkaidSolution &solution, RouteContext &context) {
    Node predecessor_head = solution.Predecessor(move.head);
    Node successor_tail = solution.Successor(move.tail);
    solution.SetSuccessor(0, context.Head(route_index));
    solution.Link(predecessor_head, successor_tail);
    if (!move.reversed) {
      solution.Link(move.predecessor, move.head);
      solution.Link(move.tail, move.successor);
    } else {
      solution.ReversedLink(move.head, move.tail, move.predecessor, move.successor);
    }
    context.SetHead(route_index, solution.Successor(0));
  }

  template <int num> void OrOptInner(const Instance &instance, const AlkaidSolution &solution, Node head,
                                     Node tail, Node predecessor, Node successor,
                                     OrOptMove &best_move, Delta<int> &best_delta, Random &random) {
    Node predecessor_head = solution.Predecessor(head);
    Node successor_tail = solution.Successor(tail);
    int delta
        = instance.distance_matrix[solution.Customer(predecessor_head)]
                                 [solution.Customer(successor_tail)]
          - instance.distance_matrix[solution.Customer(predecessor_head)][solution.Customer(head)]
          - instance.distance_matrix[solution.Customer(tail)][solution.Customer(successor_tail)]
          - instance.distance_matrix[solution.Customer(predecessor)][solution.Customer(successor)];
    bool reversed = false;
    int insertion_delta
        = instance.distance_matrix[solution.Customer(predecessor)][solution.Customer(head)]
          + instance.distance_matrix[solution.Customer(successor)][solution.Customer(tail)];
    if (num > 1) {
      int reversed_delta
          = instance.distance_matrix[solution.Customer(predecessor)][solution.Customer(tail)]
            + instance.distance_matrix[solution.Customer(successor)][solution.Customer(head)];
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
  bool intra_operator::OrOpt<num>::operator()(const Instance &instance, Node route_index,
                                              AlkaidSolution &solution, RouteContext &context,
                                              Random &random) const {
    OrOptMove best_move{};
    Delta<int> best_delta{};
    Node head = context.Head(route_index);
    Node tail = head;
    for (Node i = 0; tail && i < num - 1; ++i) {
      tail = solution.Successor(tail);
    }
    while (tail) {
      Node predecessor, successor;
      predecessor = solution.Successor(tail);
      while (predecessor) {
        successor = solution.Successor(predecessor);
        OrOptInner<num>(instance, solution, head, tail, predecessor, successor, best_move,
                        best_delta, random);
        predecessor = successor;
      }
      successor = solution.Predecessor(head);
      while (successor) {
        predecessor = solution.Predecessor(successor);
        OrOptInner<num>(instance, solution, head, tail, predecessor, successor, best_move,
                        best_delta, random);
        successor = predecessor;
      }
      head = solution.Successor(head);
      tail = solution.Successor(tail);
    }
    if (best_delta.value < 0) {
      DoOrOpt(best_move, route_index, solution, context);
      context.UpdateRouteContext(solution, route_index, 0);
      return true;
    }
    return false;
  }

  template class OrOpt<1>;
  template class OrOpt<2>;
  template class OrOpt<3>;
}  // namespace alkaidsd::intra_operator
