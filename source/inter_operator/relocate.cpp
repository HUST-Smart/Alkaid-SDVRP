#include <alkaidsd/inter_operator.h>

#include "base_cache.h"
#include "base_star.h"
#include "route_head_guard.h"

namespace alkaidsd::inter_operator {
  struct RelocateMove {
    Node route_x, route_y;
    Node node_x, predecessor_x, successor_x;
  };

  void DoRelocate(RelocateMove &move, AlkaidSolution &solution, RouteContext &context) {
    Node predecessor_x = solution.Predecessor(move.node_x);
    Node successor_x = solution.Successor(move.node_x);
    {
      RouteHeadGuard guard(solution, context, move.route_x);
      solution.Link(predecessor_x, successor_x);
    }
    {
      RouteHeadGuard guard(solution, context, move.route_y);
      solution.Link(move.predecessor_x, move.node_x);
      solution.Link(move.node_x, move.successor_x);
    }
  }

  void RelocateInner(const Instance &instance, const AlkaidSolution &solution, const RouteContext &context,
                     Node route_x, Node route_y, BaseCache<RelocateMove> &cache,
                     StarCaches &star_caches, Random &random) {
    star_caches.Preprocess(instance, solution, context, route_y, random);
    Node node_x = context.Head(route_x);
    while (node_x) {
      if (context.Load(route_y) + solution.Load(node_x) <= instance.capacity) {
        auto insertion = star_caches.Get(route_y, solution.Customer(node_x)).FindBest();
        Node predecessor_x = solution.Predecessor(node_x);
        Node successor_x = solution.Successor(node_x);
        int delta = insertion->delta.value
                    - CalcDelta(instance, solution, node_x, predecessor_x, successor_x);
        if (cache.delta.Update(delta, random)) {
          cache.move = {route_x, route_y, node_x, insertion->predecessor, insertion->successor};
        }
      }
      node_x = solution.Successor(node_x);
    }
  }

  std::vector<Node> inter_operator::Relocate::operator()(const Instance &instance, AlkaidSolution &solution,
                                                         RouteContext &context, Random &random,
                                                         CacheMap &cache_map) const {
    auto &caches = cache_map.Get<InterRouteCache<RelocateMove>>(solution, context);
    auto &star_caches = cache_map.Get<StarCaches>(solution, context);
    RelocateMove best_move{};
    Delta<int> best_delta{};
    for (Node route_x = 0; route_x < context.NumRoutes(); ++route_x) {
      for (Node route_y = 0; route_y < context.NumRoutes(); ++route_y) {
        if (route_x == route_y) {
          continue;
        }
        auto &cache = caches.Get(route_x, route_y);
        if (!cache.TryReuse()) {
          RelocateInner(instance, solution, context, route_x, route_y, cache, star_caches, random);
        } else {
          cache.move.route_x = route_x;
          cache.move.route_y = route_y;
        }
        if (best_delta.Update(cache.delta, random)) {
          best_move = cache.move;
        }
      }
    }
    if (best_delta.value < 0) {
      DoRelocate(best_move, solution, context);
      return {best_move.route_x, best_move.route_y};
    }
    return {};
  }
}  // namespace alkaidsd::inter_operator
