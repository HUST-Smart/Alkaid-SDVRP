#pragma once

#include <algorithm>
#include <vector>

#include "../cache.h"
#include "../delta.h"
#include "../route_context.h"

namespace alkaidsd::inter_operator {
  template <class T> struct BaseCache {
    bool invalidated = true;
    Delta<int> delta;
    T move;

    bool TryReuse() {
      if (!invalidated) {
        return true;
      }
      invalidated = false;
      delta = Delta<int>();
      return false;
    }
  };

  template <class T> class InterRouteCache : public Cache {
  public:
    void Reset([[maybe_unused]] const AlkaidSolution &solution, const RouteContext &context) override {
      max_index_ = context.NumRoutes();
      matrix_.resize(context.NumRoutes());
      route_index_mappings_.resize(context.NumRoutes());
      route_pool_.clear();
      unused_indices_.clear();
      for (Node i = 0; i < context.NumRoutes(); ++i) {
        matrix_[i].resize(context.NumRoutes());
        route_index_mappings_[i] = i;
        route_pool_.emplace_back(i);
        for (Node j = 0; j < context.NumRoutes(); ++j) {
          matrix_[i][j].invalidated = true;
        }
      }
    }

    void AddRoute(Node route_index) override {
      Node index;
      if (unused_indices_.empty()) {
        index = max_index_++;
        route_index_mappings_.resize(max_index_);
        matrix_.resize(max_index_);
        for (Node i = 0; i < max_index_; ++i) {
          matrix_[i].resize(max_index_);
        }
      } else {
        index = unused_indices_.back();
        unused_indices_.pop_back();
      }
      route_index_mappings_[route_index] = index;
      route_pool_.emplace_back(index);
      for (Node other : route_pool_) {
        matrix_[index][other].invalidated = true;
        matrix_[other][index].invalidated = true;
      }
    }

    void RemoveRoute(Node route_index) override {
      Node index = route_index_mappings_[route_index];
      route_pool_.erase(std::find(route_pool_.begin(), route_pool_.end(), index));
      unused_indices_.emplace_back(index);
    }

    void MoveRoute(Node dest_route_index, Node src_route_index) override {
      route_index_mappings_[dest_route_index] = route_index_mappings_[src_route_index];
    }

    void Save([[maybe_unused]] const AlkaidSolution &solution,
              [[maybe_unused]] const RouteContext &context) override {}

    BaseCache<T> &Get(Node route_a, Node route_b) {
      return matrix_[route_index_mappings_[route_a]][route_index_mappings_[route_b]];
    }

  private:
    std::vector<std::vector<BaseCache<T>>> matrix_;
    std::vector<Node> route_index_mappings_;
    std::vector<Node> route_pool_;
    std::vector<Node> unused_indices_;
    Node max_index_{};
  };
}  // namespace alkaidsd::inter_operator
