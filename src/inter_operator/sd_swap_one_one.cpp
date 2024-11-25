#include <alkaidsd/inter_operator.h>

#include "base_cache.h"
#include "base_star.h"
#include "route_head_guard.h"

namespace alkaidsd::inter_operator {
  struct SdSwapOneOneMove {
    bool swapped;
    Node route_x, route_y;
    Node node_x, predecessor_x, successor_x;
    Node node_y, predecessor_y, successor_y;
    int split_load;
  };

  void DoSdSwapOneOne(const SdSwapOneOneMove &move, AlkaidSolution &solution, RouteContext &context) {
    Node predecessor_y = solution.Predecessor(move.node_y);
    Node successor_y = solution.Successor(move.node_y);
    {
      RouteHeadGuard guard(solution, context, move.route_x);
      solution.SetLoad(move.node_x, move.split_load);
      solution.Link(move.predecessor_y, move.node_y);
      solution.Link(move.node_y, move.successor_y);
    }
    {
      RouteHeadGuard guard(solution, context, move.route_y);
      solution.Link(predecessor_y, successor_y);
      solution.Insert(solution.Customer(move.node_x), solution.Load(move.node_y),
                      move.predecessor_x, move.successor_x);
    }
  }

  void SdSwapOneOneInner(const Instance &instance, const AlkaidSolution &solution,
                         [[maybe_unused]] const RouteContext &context, bool swapped, Node route_x,
                         Node route_y, Node node_x, Node node_y, int split_load,
                         BaseCache<SdSwapOneOneMove> &cache, Random &random) {
    Node predecessor_x = solution.Predecessor(node_x);
    Node successor_x = solution.Successor(node_x);
    Node predecessor_y = solution.Predecessor(node_y);
    Node successor_y = solution.Successor(node_y);
    int delta = -CalcDelta(instance, solution, node_y, predecessor_y, successor_y);
    int delta_x = CalcDelta(instance, solution, node_x, predecessor_y, successor_y);
    int before = CalcDelta(instance, solution, node_y, predecessor_x, node_x);
    int after = CalcDelta(instance, solution, node_y, node_x, successor_x);
    int delta_y;
    Node predecessor;
    Node successor;
    if (before <= after) {
      predecessor = predecessor_x;
      successor = node_x;
      delta_y = before;
    } else {
      predecessor = node_x;
      successor = successor_x;
      delta_y = after;
    }
    delta += delta_x + delta_y;
    if (cache.delta.Update(delta, random)) {
      cache.move = {swapped,     route_x, route_y,     node_x,    predecessor_y,
                    successor_y, node_y,  predecessor, successor, split_load};
    }
  }

  void SdSwapOneOneInner(const Instance &instance, const AlkaidSolution &solution,
                         const RouteContext &context, Node route_x, Node route_y,
                         BaseCache<SdSwapOneOneMove> &cache, Random &random) {
    for (Node node_x = context.Head(route_x); node_x; node_x = solution.Successor(node_x)) {
      int load_x = solution.Load(node_x);
      for (Node node_y = context.Head(route_y); node_y; node_y = solution.Successor(node_y)) {
        int load_y = solution.Load(node_y);
        if (load_x > load_y) {
          SdSwapOneOneInner(instance, solution, context, false, route_x, route_y, node_x, node_y,
                            load_x - load_y, cache, random);
        } else if (load_y > load_x) {
          SdSwapOneOneInner(instance, solution, context, true, route_y, route_x, node_y, node_x,
                            load_y - load_x, cache, random);
        }
      }
    }
  }

  std::vector<Node> inter_operator::SdSwapOneOne::operator()(const Instance &instance,
                                                             AlkaidSolution &solution,
                                                             RouteContext &context, Random &random,
                                                             CacheMap &cache_map) const {
    auto &caches = cache_map.Get<InterRouteCache<SdSwapOneOneMove>>(solution, context);
    SdSwapOneOneMove best_move{};
    Delta<int> best_delta{};
    for (Node route_x = 0; route_x < context.NumRoutes(); ++route_x) {
      for (Node route_y = route_x + 1; route_y < context.NumRoutes(); ++route_y) {
        auto &cache = caches.Get(route_x, route_y);
        if (!cache.TryReuse()) {
          SdSwapOneOneInner(instance, solution, context, route_x, route_y, cache, random);
        } else {
          if (!cache.move.swapped) {
            cache.move.route_x = route_x;
            cache.move.route_y = route_y;
          } else {
            cache.move.route_x = route_y;
            cache.move.route_y = route_x;
          }
        }
        if (best_delta.Update(cache.delta, random)) {
          best_move = cache.move;
        }
      }
    }
    if (best_delta.value < 0) {
      DoSdSwapOneOne(best_move, solution, context);
      return {best_move.route_x, best_move.route_y};
    }
    return {};
  }
}  // namespace alkaidsd::inter_operator
