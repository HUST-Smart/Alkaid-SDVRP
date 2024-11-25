#include <alkaidsd/inter_operator.h>

#include "base_cache.h"
#include "route_head_guard.h"

namespace alkaidsd::inter_operator {
  struct SdSwapTwoOneMove {
    bool type;
    Node route_ij, route_k;
    Node predecessor_ij, successor_ij;
    Node node_i, node_j, node_k;
    int split_load;
    bool direction_ij, direction_ijk;
  };

  void DoSdSwapTwoOne(const SdSwapTwoOneMove &move, AlkaidSolution &solution, RouteContext &context) {
    Node predecessor_k = solution.Predecessor(move.node_k);
    Node successor_k = solution.Successor(move.node_k);
    {
      RouteHeadGuard guard(solution, context, move.route_ij);
      solution.Link(move.predecessor_ij, move.successor_ij);
    }
    {
      RouteHeadGuard guard(solution, context, move.route_k);
      solution.Link(predecessor_k, successor_k);
    }
    if (move.type == 0) {
      int new_node_j = solution.NewNode(solution.Customer(move.node_j), move.split_load);
      solution.SetLoad(move.node_j, solution.Load(move.node_j) - move.split_load);
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
      int new_node_k = solution.NewNode(solution.Customer(move.node_k), move.split_load);
      solution.SetLoad(move.node_k, solution.Load(move.node_k) - move.split_load);
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

  void SdSwapTwoOne0(const Instance &instance, const AlkaidSolution &solution,
                     [[maybe_unused]] const RouteContext &context, Node route_ij, Node route_k,
                     Node node_i, Node node_j, Node node_k, Node predecessor_ij, Node successor_ij,
                     int split_load, int base_delta, BaseCache<SdSwapTwoOneMove> &cache,
                     Random &random) {
    Node predecessor_k = solution.Predecessor(node_k);
    Node successor_k = solution.Successor(node_k);
    int delta_ij
        = instance.distance_matrix[solution.Customer(predecessor_k)][solution.Customer(node_i)]
          + instance.distance_matrix[solution.Customer(node_j)][solution.Customer(successor_k)];
    int delta_ji
        = instance.distance_matrix[solution.Customer(predecessor_k)][solution.Customer(node_j)]
          + instance.distance_matrix[solution.Customer(node_i)][solution.Customer(successor_k)];
    int delta_jk
        = instance.distance_matrix[solution.Customer(predecessor_ij)][solution.Customer(node_j)]
          + instance.distance_matrix[solution.Customer(node_k)][solution.Customer(successor_ij)];
    int delta_kj
        = instance.distance_matrix[solution.Customer(predecessor_ij)][solution.Customer(node_k)]
          + instance.distance_matrix[solution.Customer(node_j)][solution.Customer(successor_ij)];
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
    int delta = base_delta
                + instance.distance_matrix[solution.Customer(node_j)][solution.Customer(node_k)]
                + delta_ij + delta_jk;
    if (cache.delta.Update(delta, random)) {
      cache.move = {0,      route_ij, route_k,    predecessor_ij, successor_ij, node_i,
                    node_j, node_k,   split_load, direction_ij,   direction_jk};
    }
  }

  void SdSwapTwoOne1(const Instance &instance, const AlkaidSolution &solution,
                     [[maybe_unused]] const RouteContext &context, Node route_ij, Node route_k,
                     Node node_i, Node node_j, Node node_k, Node predecessor_ij, Node successor_ij,
                     int split_load, int base_delta, BaseCache<SdSwapTwoOneMove> &cache,
                     Random &random) {
    Node predecessor_k = solution.Predecessor(node_k);
    Node successor_k = solution.Successor(node_k);
    base_delta
        += instance.distance_matrix[solution.Customer(predecessor_ij)][solution.Customer(node_k)]
           + instance.distance_matrix[solution.Customer(node_k)][solution.Customer(successor_ij)];
    for (bool direction_ij : {true, false}) {
      int before_ij = node_i;
      int after_ij = node_j;
      if (!direction_ij) {
        std::swap(before_ij, after_ij);
      }
      for (bool direction_ijk : {true, false}) {
        int delta_ijk;
        if (direction_ijk) {
          delta_ijk
              = instance
                    .distance_matrix[solution.Customer(predecessor_k)][solution.Customer(before_ij)]
                + instance.distance_matrix[solution.Customer(after_ij)][solution.Customer(node_k)]
                + instance
                      .distance_matrix[solution.Customer(node_k)][solution.Customer(successor_k)];
        } else {
          delta_ijk
              = instance.distance_matrix[solution.Customer(predecessor_k)][solution.Customer(node_k)]
                + instance.distance_matrix[solution.Customer(node_k)][solution.Customer(before_ij)]
                + instance
                      .distance_matrix[solution.Customer(after_ij)][solution.Customer(successor_k)];
        }
        int delta = base_delta + delta_ijk;
        if (cache.delta.Update(delta, random)) {
          cache.move = {1,      route_ij, route_k,    predecessor_ij, successor_ij, node_i,
                        node_j, node_k,   split_load, direction_ij,   direction_ijk};
        }
      }
    }
  }

  void SdSwapTwoOneInner(const Instance &instance, const AlkaidSolution &solution,
                         const RouteContext &context, Node route_ij, Node route_k,
                         BaseCache<SdSwapTwoOneMove> &cache, Random &random) {
    Node node_i = context.Head(route_ij);
    Node node_j = solution.Successor(node_i);
    while (node_j) {
      int load_i = solution.Load(node_i);
      int load_j = solution.Load(node_j);
      for (Node node_k = context.Head(route_k); node_k; node_k = solution.Successor(node_k)) {
        int load_k = solution.Load(node_k);
        Node predecessor_ij = solution.Predecessor(node_i);
        Node successor_ij = solution.Successor(node_j);
        int base_delta
            = -instance.distance_matrix[solution.Customer(predecessor_ij)][solution.Customer(node_i)]
              - instance.distance_matrix[solution.Customer(node_j)][solution.Customer(successor_ij)]
              - instance.distance_matrix[solution.Customer(solution.Predecessor(node_k))]
                                       [solution.Customer(node_k)]
              - instance.distance_matrix[solution.Customer(node_k)]
                                       [solution.Customer(solution.Successor(node_k))];
        if (load_i + load_j > load_k) {
          if (load_i < load_k) {
            SdSwapTwoOne0(instance, solution, context, route_ij, route_k, node_i, node_j, node_k,
                          predecessor_ij, successor_ij, load_i + load_j - load_k, base_delta, cache,
                          random);
          }
          if (load_j < load_k) {
            SdSwapTwoOne0(instance, solution, context, route_ij, route_k, node_j, node_i, node_k,
                          predecessor_ij, successor_ij, load_i + load_j - load_k, base_delta, cache,
                          random);
          }
        } else if (load_k > load_i + load_j) {
          SdSwapTwoOne1(instance, solution, context, route_ij, route_k, node_i, node_j, node_k,
                        predecessor_ij, successor_ij, load_k - load_i - load_j, base_delta, cache,
                        random);
        }
      }
      node_i = node_j;
      node_j = solution.Successor(node_j);
    }
  }

  std::vector<Node> inter_operator::SdSwapTwoOne::operator()(const Instance &instance,
                                                             AlkaidSolution &solution,
                                                             RouteContext &context, Random &random,
                                                             CacheMap &cache_map) const {
    auto &caches = cache_map.Get<InterRouteCache<SdSwapTwoOneMove>>(solution, context);
    SdSwapTwoOneMove best_move{};
    Delta<int> best_delta{};
    for (Node route_ij = 0; route_ij < context.NumRoutes(); ++route_ij) {
      for (Node route_k = 0; route_k < context.NumRoutes(); ++route_k) {
        if (route_ij == route_k) {
          continue;
        }
        auto &cache = caches.Get(route_ij, route_k);
        if (!cache.TryReuse()) {
          SdSwapTwoOneInner(instance, solution, context, route_ij, route_k, cache, random);
        } else {
          cache.move.route_ij = route_ij;
          cache.move.route_k = route_k;
        }
        if (best_delta.Update(cache.delta, random)) {
          best_move = cache.move;
        }
      }
    }
    if (best_delta.value < 0) {
      DoSdSwapTwoOne(best_move, solution, context);
      return {best_move.route_ij, best_move.route_k};
    }
    return {};
  }
}  // namespace alkaidsd::inter_operator
