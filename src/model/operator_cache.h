#ifndef ALKAID_SD_SRC_MODEL_OPERATOR_CACHE_H_
#define ALKAID_SD_SRC_MODEL_OPERATOR_CACHE_H_

#include <array>
#include <functional>
#include <memory>
#include <vector>

#include "model/route_context.h"
#include "util/delta.h"

namespace alkaid_sd {

struct BaseCache {
  bool invalidated = true;
  Delta<int> delta;
  unsigned char move[24];

  inline bool TryReuse() {
    //    if (!invalidated) {
    //      return true;
    //    }
    invalidated = false;
    delta = Delta<int>();
    return false;
  }
};

class OperatorCaches
    : std::array<std::array<BaseCache, kMaxNumRoutes>, kMaxNumRoutes> {
public:
  void Init(const RouteContext &context, bool need) {
    need_ = need;
    if (!need) {
      return;
    }
    max_index_ = context.num_routes;
    route_pool_.clear();
    unused_indices_.clear();
    for (Node i = 0; i < context.num_routes; ++i) {
      route_index_mappings_[i] = i;
      route_pool_.emplace_back(i);
      for (Node j = 0; j < context.num_routes; ++j) {
        (*this)[i][j].invalidated = true;
      }
    }
  }

  void AddRoute(Node route_index) {
    if (!need_) {
      return;
    }
    Node index;
    if (unused_indices_.empty()) {
      index = max_index_++;
    } else {
      index = unused_indices_.back();
      unused_indices_.pop_back();
    }
    route_index_mappings_[route_index] = index;
    route_pool_.emplace_back(index);
    for (Node other : route_pool_) {
      (*this)[index][other].invalidated = true;
      (*this)[other][index].invalidated = true;
    }
  }

  void RemoveRoute(Node route_index) {
    if (!need_) {
      return;
    }
    Node index = route_index_mappings_[route_index];
    route_pool_.erase(std::find(route_pool_.begin(), route_pool_.end(), index));
    unused_indices_.emplace_back(index);
  }

  void MoveRoute(Node dest_route_index, Node src_route_index) {
    if (!need_) {
      return;
    }
    route_index_mappings_[dest_route_index] =
        route_index_mappings_[src_route_index];
  }

  BaseCache &Get(Node route_a, Node route_b) {
    return (
        *this)[route_index_mappings_[route_a]][route_index_mappings_[route_b]];
  }

private:
  bool need_;
  Node route_index_mappings_[kMaxNumRoutes];
  std::vector<Node> route_pool_;
  std::vector<Node> unused_indices_;
  Node max_index_;
};

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_MODEL_OPERATOR_CACHE_H_
