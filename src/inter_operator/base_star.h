#pragma once

#include <alkaidsd/inter_operator.h>

#include <limits>

#include "base_cache.h"

namespace alkaidsd::inter_operator {
  struct Insertion {
    Delta<int> delta;
    Node predecessor{}, successor{};
  };

  template <int num> struct BestInsertion {
    Insertion insertions[num];

    void Reset() {
      for (auto &insertion : insertions) {
        insertion.delta = Delta(std::numeric_limits<int>::max(), -1);
      }
    }

    void Add(int delta, Node predecessor, Node successor, Random &random) {
      for (int i = 0; i < num; ++i) {
        if (insertions[i].delta.value == std::numeric_limits<int>::max()) {
          insertions[i] = {{delta, 1}, predecessor, successor};
          return;
        } else if (delta < insertions[i].delta.value) {
          for (int j = num - 1; j > i; --j) {
            insertions[j] = insertions[j - 1];
          }
          insertions[i] = {{delta, 1}, predecessor, successor};
          return;
        } else if (delta == insertions[i].delta.value && insertions[i].delta.counter != -1) {
          if (random.NextInt(1, insertions[i].delta.counter + 1) == 1) {
            for (int j = num - 1; j > i; --j) {
              insertions[j] = insertions[j - 1];
            }
            ++insertions[i].delta.counter;
            insertions[i].predecessor = predecessor;
            insertions[i].successor = successor;
            break;
          } else {
            ++insertions[i].delta.counter;
          }
        }
      }
    }

    const Insertion *FindBest() const { return insertions; }

    const Insertion *FindBestWithoutNode(Node node_index) const {
      for (auto &insertion : insertions) {
        if (insertion.delta.counter > 0 && insertion.predecessor != node_index
            && insertion.successor != node_index) {
          return &insertion;
        }
      }
      return nullptr;
    }
  };

  class StarCaches : public Cache {
  public:
    void Reset(const AlkaidSolution &solution, const RouteContext &context) override {
      caches_.resize(context.NumRoutes());
      for (Node route_index = 0;
           static_cast<size_t>(route_index) < std::min(routes_.size(), caches_.size());
           ++route_index) {
        bool same_route = false;
        if (route_index < context.NumRoutes()) {
          same_route = true;
          Node head = context.Head(route_index);
          for (Node node : routes_[route_index]) {
            if (head != node) {
              same_route = false;
              break;
            }
            head = solution.Successor(head);
          }
          if (head != 0) {
            same_route = false;
          }
        }
        if (!same_route) {
          caches_[route_index].clear();
        }
      }
    }
    void AddRoute(Node route_index) override { caches_.resize(route_index + 1); }
    void RemoveRoute(Node route_index) override { caches_[route_index].clear(); }
    void MoveRoute(Node dest_route_index, Node src_route_index) override {
      caches_[dest_route_index].swap(caches_[src_route_index]);
    }
    void Preprocess(const Instance &problem, const AlkaidSolution &solution, const RouteContext &context,
                    Node route, Random &random) {
      auto &&insertions = caches_[route];
      if (!insertions.empty()) {
        return;
      }
      insertions.resize(problem.num_customers);
      for (Node customer = 1; customer < problem.num_customers; ++customer) {
        insertions[customer].Reset();
      }
      Node predecessor = 0;
      Node successor = context.Head(route);
      while (true) {
        Node predecessor_customer = solution.Customer(predecessor);
        Node successor_customer = solution.Customer(successor);
        auto &&predecessor_distances = problem.distance_matrix[predecessor_customer];
        auto &&successor_distances = problem.distance_matrix[successor_customer];
        auto distance = problem.distance_matrix[predecessor_customer][successor_customer];
        for (Node customer = 1; customer < problem.num_customers; ++customer) {
          int delta = predecessor_distances[customer] + successor_distances[customer] - distance;
          insertions[customer].Add(delta, predecessor, successor, random);
        }
        if (!successor) {
          break;
        }
        predecessor = successor;
        successor = solution.Successor(successor);
      }
    }
    void Save(const AlkaidSolution &solution, const RouteContext &context) override {
      routes_.resize(context.NumRoutes());
      for (Node route_index = 0; static_cast<size_t>(route_index) < routes_.size(); ++route_index) {
        auto &route = routes_[route_index];
        route.clear();
        for (Node node = context.Head(route_index); node; node = solution.Successor(node)) {
          route.push_back(node);
        }
      }
    }
    BestInsertion<3> &Get(Node route_index, Node customer) {
      return caches_[route_index][customer];
    }

  private:
    std::vector<std::vector<BestInsertion<3>>> caches_;
    std::vector<std::vector<Node>> routes_;
  };

  inline int CalcDelta(const Instance &problem, const AlkaidSolution &solution, Node node_index,
                       Node predecessor, Node successor) {
    return problem.distance_matrix[solution.Customer(node_index)][solution.Customer(predecessor)]
           + problem.distance_matrix[solution.Customer(node_index)][solution.Customer(successor)]
           - problem.distance_matrix[solution.Customer(predecessor)][solution.Customer(successor)];
  }
}  // namespace alkaidsd::inter_operator
