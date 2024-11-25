#pragma once

#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "route_context.h"

namespace alkaidsd {
  class Cache {
  public:
    virtual ~Cache() = default;
    virtual void Reset(const alkaidsd::AlkaidSolution &solution, const alkaidsd::RouteContext &context)
        = 0;
    virtual void AddRoute(alkaidsd::Node route_index) = 0;
    virtual void RemoveRoute(alkaidsd::Node route_index) = 0;
    virtual void MoveRoute(alkaidsd::Node dest_route_index, alkaidsd::Node src_route_index) = 0;
    virtual void Save(const alkaidsd::AlkaidSolution &solution, const alkaidsd::RouteContext &context)
        = 0;
  };

  class CacheMap : public Cache {
  public:
    template <class T>
    T &Get(const alkaidsd::AlkaidSolution &solution, const alkaidsd::RouteContext &context) {
      auto it = caches_.find(typeid(T));
      if (it == caches_.end()) {
        auto cache = std::make_unique<T>();
        cache->Reset(solution, context);
        auto &cache_ref = *cache;
        caches_.emplace(typeid(T), std::move(cache));
        return cache_ref;
      }
      return *static_cast<T *>(it->second.get());
    }
    void Reset(const alkaidsd::AlkaidSolution &solution, const alkaidsd::RouteContext &context) override {
      for (auto &[_, cache] : caches_) {
        cache->Reset(solution, context);
      }
    }
    void AddRoute(alkaidsd::Node route_index) override {
      for (auto &[_, cache] : caches_) {
        cache->AddRoute(route_index);
      }
    }
    void RemoveRoute(alkaidsd::Node route_index) override {
      for (auto &[_, cache] : caches_) {
        cache->RemoveRoute(route_index);
      }
    }
    void MoveRoute(alkaidsd::Node dest_route_index, alkaidsd::Node src_route_index) override {
      for (auto &[_, cache] : caches_) {
        cache->MoveRoute(dest_route_index, src_route_index);
      }
    }
    void Save(const alkaidsd::AlkaidSolution &solution, const alkaidsd::RouteContext &context) override {
      for (auto &[_, cache] : caches_) {
        cache->Save(solution, context);
      }
    }

  private:
    std::unordered_map<std::type_index, std::unique_ptr<Cache>> caches_;
  };
}  // namespace alkaidsd
