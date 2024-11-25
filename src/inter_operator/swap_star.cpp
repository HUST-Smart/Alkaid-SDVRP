#include <alkaidsd/inter_operator.h>

#include <vector>

#include "base_cache.h"
#include "base_star.h"
#include "route_head_guard.h"

namespace alkaidsd::inter_operator {
  struct SwapStarMove {
    Node route_x, route_y;
    Node node_x, predecessor_x, successor_x;
    Node node_y, predecessor_y, successor_y;
  };

  void DoSwapStar(SwapStarMove &move, AlkaidSolution &solution, RouteContext &context) {
    Node predecessor_x = solution.Predecessor(move.node_x);
    Node successor_x = solution.Successor(move.node_x);
    Node predecessor_y = solution.Predecessor(move.node_y);
    Node successor_y = solution.Successor(move.node_y);
    {
      RouteHeadGuard guard(solution, context, move.route_x);
      solution.Link(predecessor_x, successor_x);
      solution.Link(move.predecessor_y, move.node_y);
      solution.Link(move.node_y, move.successor_y);
    }
    {
      RouteHeadGuard guard(solution, context, move.route_y);
      solution.Link(predecessor_y, successor_y);
      solution.Link(move.predecessor_x, move.node_x);
      solution.Link(move.node_x, move.successor_x);
    }
  }

  void SwapStarInner(const Instance &instance, const AlkaidSolution &solution, const RouteContext &context,
                     Node route_x, Node route_y, BaseCache<SwapStarMove> &cache,
                     StarCaches &star_caches, Random &random) {
    star_caches.Preprocess(instance, solution, context, route_x, random);
    star_caches.Preprocess(instance, solution, context, route_y, random);
    Node node_x = context.Head(route_x);
    while (node_x) {
      auto &&insertion_x = star_caches.Get(route_y, solution.Customer(node_x));
      int load_x = solution.Load(node_x);
      int load_y_lower = -instance.capacity + context.Load(route_y) + load_x;
      int load_y_upper = instance.capacity - context.Load(route_x) + load_x;
      Node node_y = context.Head(route_y);
      while (node_y) {
        int load_y = solution.Load(node_y);
        if (load_y >= load_y_lower && load_y <= load_y_upper) {
          auto &&insertion_y = star_caches.Get(route_x, solution.Customer(node_y));
          Node predecessor_x = solution.Predecessor(node_x);
          Node successor_x = solution.Successor(node_x);
          Node predecessor_y = solution.Predecessor(node_y);
          Node successor_y = solution.Successor(node_y);
          int delta = -CalcDelta(instance, solution, node_x, predecessor_x, successor_x)
                      - CalcDelta(instance, solution, node_y, predecessor_y, successor_y);
          int delta_x = CalcDelta(instance, solution, node_x, predecessor_y, successor_y);
          int delta_y = CalcDelta(instance, solution, node_y, predecessor_x, successor_x);
          auto best_insertion_x = insertion_x.FindBestWithoutNode(node_y);
          if (best_insertion_x && best_insertion_x->delta.value < delta_x) {
            delta_x = best_insertion_x->delta.value;
            predecessor_y = best_insertion_x->predecessor;
            successor_y = best_insertion_x->successor;
          }
          auto best_insertion_y = insertion_y.FindBestWithoutNode(node_x);
          if (best_insertion_y && best_insertion_y->delta.value < delta_y) {
            delta_y = best_insertion_y->delta.value;
            predecessor_x = best_insertion_y->predecessor;
            successor_x = best_insertion_y->successor;
          }
          delta += delta_x + delta_y;
          if (cache.delta.Update(delta, random)) {
            cache.move = {route_x,     route_y, node_x,        predecessor_y,
                          successor_y, node_y,  predecessor_x, successor_x};
          }
        }
        node_y = solution.Successor(node_y);
      }
      node_x = solution.Successor(node_x);
    }
  }

  std::vector<Node> inter_operator::SwapStar::operator()(const Instance &instance, AlkaidSolution &solution,
                                                         RouteContext &context, Random &random,
                                                         CacheMap &cache_map) const {
    auto &caches = cache_map.Get<InterRouteCache<SwapStarMove>>(solution, context);
    auto &star_caches = cache_map.Get<StarCaches>(solution, context);
    SwapStarMove best_move{};
    Delta<int> best_delta{};
    for (Node route_x = 0; route_x < context.NumRoutes(); ++route_x) {
      for (Node route_y = route_x + 1; route_y < context.NumRoutes(); ++route_y) {
        auto &cache = caches.Get(route_x, route_y);
        if (!cache.TryReuse()) {
          SwapStarInner(instance, solution, context, route_x, route_y, cache, star_caches, random);
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
      DoSwapStar(best_move, solution, context);
      return {best_move.route_x, best_move.route_y};
    }
    return {};
  }
}  // namespace alkaidsd::inter_operator
