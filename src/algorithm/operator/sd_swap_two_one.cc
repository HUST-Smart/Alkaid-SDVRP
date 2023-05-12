#include "algorithm/operators.h"

#include "model/route_head_guard.h"
#include "util/solution_utils.h"

namespace alkaid_sd {

struct SdSwapTwoOneMove {
  bool type;
  Node route_ij, route_k;
  Node predecessor_ij, successor_ij;
  Node node_i, node_j, node_k;
  int split_load;
  bool direction_ij, direction_ijk;
};

void DoSdSwapTwoOne(const SdSwapTwoOneMove &move, Solution &solution,
                    RouteContext &context) {
  Node predecessor_k = solution.predecessor(move.node_k);
  Node successor_k = solution.successor(move.node_k);
  {
    RouteHeadGuard guard(solution, context, move.route_ij);
    solution.Link(move.predecessor_ij, move.successor_ij);
  }
  {
    RouteHeadGuard guard(solution, context, move.route_k);
    solution.Link(predecessor_k, successor_k);
  }
  if (move.type == 0) {
    int new_node_j =
        solution.NewNode(solution.customer(move.node_j), move.split_load);
    solution.set_load(move.node_j,
                      solution.load(move.node_j) - move.split_load);
    {
      RouteHeadGuard guard(solution, context, move.route_ij);
      if (move.direction_ijk) {
        solution.Link(move.predecessor_ij, new_node_j);
        solution.Link(new_node_j, move.node_k);
        solution.Link(move.node_k, move.successor_ij);
      } else {
        solution.Link(move.predecessor_ij, move.node_k);
        solution.Link(move.node_k, new_node_j);
        solution.Link(new_node_j, move.successor_ij);
      }
    }
    {
      RouteHeadGuard guard(solution, context, move.route_k);
      if (move.direction_ij) {
        solution.Link(predecessor_k, move.node_i);
        solution.Link(move.node_i, move.node_j);
        solution.Link(move.node_j, successor_k);
      } else {
        solution.Link(predecessor_k, move.node_j);
        solution.Link(move.node_j, move.node_i);
        solution.Link(move.node_i, successor_k);
      }
    }
  } else {
    int new_node_k =
        solution.NewNode(solution.customer(move.node_k), move.split_load);
    solution.set_load(move.node_k,
                      solution.load(move.node_k) - move.split_load);
    {
      RouteHeadGuard guard(solution, context, move.route_ij);
      solution.Link(move.predecessor_ij, move.node_k);
      solution.Link(move.node_k, move.successor_ij);
    }
    {
      RouteHeadGuard guard(solution, context, move.route_k);
      int before_ij;
      int after_ij;
      if (move.direction_ij) {
        solution.Link(predecessor_k, move.node_i);
        solution.Link(move.node_i, move.node_j);
        solution.Link(move.node_j, successor_k);
        before_ij = move.node_i;
        after_ij = move.node_j;
      } else {
        solution.Link(predecessor_k, move.node_j);
        solution.Link(move.node_j, move.node_i);
        solution.Link(move.node_i, successor_k);
        before_ij = move.node_j;
        after_ij = move.node_i;
      }
      if (move.direction_ijk) {
        solution.Link(after_ij, new_node_k);
        solution.Link(new_node_k, successor_k);
      } else {
        solution.Link(predecessor_k, new_node_k);
        solution.Link(new_node_k, before_ij);
      }
    }
  }
}

inline void SdSwapTwoOne0(const Problem &problem,
                          const DistanceMatrix &distance_matrix,
                          const Solution &solution, const RouteContext &context,
                          Node route_ij, Node route_k, Node node_i, Node node_j,
                          Node node_k, Node predecessor_ij, Node successor_ij,
                          int split_load, int base_delta,
                          SdSwapTwoOneMove &best_move, Delta<int> &best_delta,
                          Random &random) {
  Node predecessor_k = solution.predecessor(node_k);
  Node successor_k = solution.successor(node_k);
  int delta_ij = distance_matrix[solution.customer(predecessor_k)]
                                [solution.customer(node_i)] +
                 distance_matrix[solution.customer(node_j)]
                                [solution.customer(successor_k)];
  int delta_ji = distance_matrix[solution.customer(predecessor_k)]
                                [solution.customer(node_j)] +
                 distance_matrix[solution.customer(node_i)]
                                [solution.customer(successor_k)];
  int delta_jk = distance_matrix[solution.customer(predecessor_ij)]
                                [solution.customer(node_j)] +
                 distance_matrix[solution.customer(node_k)]
                                [solution.customer(successor_ij)];
  int delta_kj = distance_matrix[solution.customer(predecessor_ij)]
                                [solution.customer(node_k)] +
                 distance_matrix[solution.customer(node_j)]
                                [solution.customer(successor_ij)];
  bool direction_ij = true;
  if (delta_ij > delta_ji) {
    delta_ij = delta_ji;
    direction_ij = false;
  }
  bool direction_jk = true;
  if (delta_jk > delta_kj) {
    delta_jk = delta_kj;
    direction_jk = false;
  }
  int delta =
      base_delta +
      distance_matrix[solution.customer(node_j)][solution.customer(node_k)] +
      delta_ij + delta_jk;
  if (best_delta.Update(delta, random)) {
    best_move = {0,           route_ij, route_k, predecessor_ij, successor_ij,
                 node_i,      node_j,   node_k,  split_load,     direction_ij,
                 direction_jk};
  }
}

inline void SdSwapTwoOne1(const Problem &problem,
                          const DistanceMatrix &distance_matrix,
                          const Solution &solution, const RouteContext &context,
                          Node route_ij, Node route_k, Node node_i, Node node_j,
                          Node node_k, Node predecessor_ij, Node successor_ij,
                          int split_load, int base_delta,
                          SdSwapTwoOneMove &best_move, Delta<int> &best_delta,
                          Random &random) {
  Node predecessor_k = solution.predecessor(node_k);
  Node successor_k = solution.successor(node_k);
  base_delta += distance_matrix[solution.customer(predecessor_ij)]
                               [solution.customer(node_k)] +
                distance_matrix[solution.customer(node_k)]
                               [solution.customer(successor_ij)];
  for (bool direction_ij : {true, false}) {
    int before_ij = node_i;
    int after_ij = node_j;
    if (!direction_ij) {
      std::swap(before_ij, after_ij);
    }
    for (bool direction_ijk : {true, false}) {
      int delta_ijk;
      if (direction_ijk) {
        delta_ijk = distance_matrix[solution.customer(predecessor_k)]
                                   [solution.customer(before_ij)] +
                    distance_matrix[solution.customer(after_ij)]
                                   [solution.customer(node_k)] +
                    distance_matrix[solution.customer(node_k)]
                                   [solution.customer(successor_k)];
      } else {
        delta_ijk = distance_matrix[solution.customer(predecessor_k)]
                                   [solution.customer(node_k)] +
                    distance_matrix[solution.customer(node_k)]
                                   [solution.customer(before_ij)] +
                    distance_matrix[solution.customer(after_ij)]
                                   [solution.customer(successor_k)];
      }
      int delta = base_delta + delta_ijk;
      if (best_delta.Update(delta, random)) {
        best_move = {
            1,      route_ij, route_k,    predecessor_ij, successor_ij, node_i,
            node_j, node_k,   split_load, direction_ij,   direction_ijk};
      }
    }
  }
}

void SdSwapTwoOne(const Problem &problem, const DistanceMatrix &distance_matrix,
                  const Solution &solution, const RouteContext &context,
                  Node route_ij, Node route_k, SdSwapTwoOneMove &best_move,
                  Delta<int> &best_delta, Random &random) {
  Node node_i = context.heads[route_ij];
  Node node_j = solution.successor(node_i);
  while (node_j) {
    Load load_i = solution.load(node_i);
    Load load_j = solution.load(node_j);
    for (Node node_k = context.heads[route_k]; node_k;
         node_k = solution.successor(node_k)) {
      Load load_k = solution.load(node_k);
      int predecessor_ij = solution.predecessor(node_i);
      int successor_ij = solution.successor(node_j);
      int base_delta =
          -distance_matrix[solution.customer(predecessor_ij)]
                          [solution.customer(node_i)] -
          distance_matrix[solution.customer(node_j)]
                         [solution.customer(successor_ij)] -
          distance_matrix[solution.customer(solution.predecessor(node_k))]
                         [solution.customer(node_k)] -
          distance_matrix[solution.customer(node_k)]
                         [solution.customer(solution.successor(node_k))];
      if (load_i + load_j > load_k) {
        if (load_i < load_k) {
          SdSwapTwoOne0(problem, distance_matrix, solution, context, route_ij,
                        route_k, node_i, node_j, node_k, predecessor_ij,
                        successor_ij, load_i + load_j - load_k, base_delta,
                        best_move, best_delta, random);
        }
        if (load_j < load_k) {
          SdSwapTwoOne0(problem, distance_matrix, solution, context, route_ij,
                        route_k, node_j, node_i, node_k, predecessor_ij,
                        successor_ij, load_i + load_j - load_k, base_delta,
                        best_move, best_delta, random);
        }
      } else if (load_k > load_i + load_j) {
        SdSwapTwoOne1(problem, distance_matrix, solution, context, route_ij,
                      route_k, node_i, node_j, node_k, predecessor_ij,
                      successor_ij, load_k - load_i - load_j, base_delta,
                      best_move, best_delta, random);
      }
    }
    node_i = node_j;
    node_j = solution.successor(node_j);
  }
}

bool SdSwapTwoOne(const Problem &problem, const DistanceMatrix &distance_matrix,
                  Solution &solution, RouteContext &context,
                  std::set<Node> &associated_routes, Random &random,
                  OperatorCaches &operator_caches) {
  SdSwapTwoOneMove best_move{};
  Delta<int> best_delta;
  for (Node route_ij = 0; route_ij < context.num_routes; ++route_ij) {
    for (Node route_k = 0; route_k < context.num_routes; ++route_k) {
      if (route_ij == route_k) {
        continue;
      }
      auto &cache = operator_caches.Get(route_ij, route_k);
      auto &move = *reinterpret_cast<SdSwapTwoOneMove *>(cache.move);
      if (!cache.TryReuse()) {
        SdSwapTwoOne(problem, distance_matrix, solution, context, route_ij,
                     route_k, move, cache.delta, random);
      } else {
        move.route_ij = route_ij;
        move.route_k = route_k;
      }
      if (best_delta.Update(cache.delta, random)) {
        best_move = move;
      }
    }
  }
  if (best_delta.value < 0) {
    DoSdSwapTwoOne(best_move, solution, context);
    associated_routes.insert(best_move.route_ij);
    associated_routes.insert(best_move.route_k);
    return true;
  }
  return false;
}

} // namespace alkaid_sd
