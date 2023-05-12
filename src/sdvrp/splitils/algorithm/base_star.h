#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_BASE_STAR_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_BASE_STAR_H_

#include "sdvrp/splitils/algorithm/operators.h"

namespace vrp::sdvrp::splitils {

struct Insertion {
  Delta<int> delta;
  Node predecessor{}, successor{};
};

template <int num> struct BestInsertion {
  Insertion insertions[num];

  inline void Reset() {
    for (auto &insertion : insertions) {
      insertion.delta = Delta(std::numeric_limits<int>::max(), -1);
    }
  }

  inline void Add(int delta, Node predecessor, Node successor, Random &random) {
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
      } else if (delta == insertions[i].delta.value &&
                 insertions[i].delta.counter != -1) {
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

  inline void Add(const Insertion &insertion, Random &random) {
    for (int i = 0; i < num; ++i) {
      if (insertion.delta.value < insertions[i].delta.value) {
        for (int j = num - 1; j > i; --j) {
          insertions[j] = insertions[j - 1];
        }
        insertions[i] = insertion;
        return;
      } else if (insertion.delta.value == insertions[i].delta.value &&
                 insertions[i].delta.counter != -1) {
        if (random.NextInt(1, insertions[i].delta.counter +
                                  insertion.delta.counter) == 1) {
          for (int j = num - 1; j > i; --j) {
            insertions[j] = insertions[j - 1];
          }
          insertions[i].delta.counter += insertion.delta.counter;
          insertions[i].predecessor = insertion.predecessor;
          insertions[i].successor = insertion.successor;
          break;
        } else {
          insertions[i].delta.counter += insertion.delta.counter;
        }
      }
    }
  }

  [[nodiscard]] inline const Insertion *FindBest() const { return insertions; }

  [[nodiscard]] inline const Insertion *
  FindBestWithoutNode(Node node_index) const {
    for (auto &insertion : insertions) {
      if (insertion.delta.counter > 0 && insertion.predecessor != node_index &&
          insertion.successor != node_index) {
        return &insertion;
      }
    }
    return nullptr;
  }

  [[nodiscard]] inline const Insertion *
  FindBestWithoutRoute(Node route_index) const {
    for (auto &insertion : insertions) {
      if (insertion.delta.counter > 0 && insertion.route_index != route_index) {
        return &insertion;
      }
    }
    return nullptr;
  }

  template <class Predicate>
  [[nodiscard]] inline const Insertion *FindBest(Predicate &&predicate) const {
    for (auto &insertion : insertions) {
      if (insertion.delta.counter > 0 && predicate(insertion)) {
        return &insertion;
      }
    }
    return nullptr;
  }
};

inline int CalcDelta(const DistanceMatrix &distance_matrix,
                     const Solution &solution, Node node_index,
                     Node predecessor, Node successor) {
  return distance_matrix[solution.customer(node_index)]
                        [solution.customer(predecessor)] +
         distance_matrix[solution.customer(node_index)]
                        [solution.customer(successor)] -
         distance_matrix[solution.customer(predecessor)]
                        [solution.customer(successor)];
}

template <int num>
void PreprocessInsertions(const DistanceMatrix &distance_matrix,
                          const Solution &solution, const RouteContext &context,
                          Node route_x, Node route_y,
                          std::vector<BestInsertion<num>> &insertions,
                          Random &random) {
  Node node_x = context.heads[route_x];
  while (node_x) {
    auto &insertion = insertions[node_x];
    insertion.Reset();
    Node predecessor = 0;
    Node successor = context.heads[route_y];
    while (true) {
      int delta =
          CalcDelta(distance_matrix, solution, node_x, predecessor, successor);
      insertion.Add(delta, predecessor, successor, random);
      if (!successor) {
        break;
      }
      predecessor = successor;
      successor = solution.successor(successor);
    }
    node_x = solution.successor(node_x);
  }
}

inline std::vector<std::vector<BestInsertion<3>>> star_caches(kMaxNumRoutes);

inline void PreprocessStarInsertions(const Problem &problem,
                                     const DistanceMatrix &distance_matrix,
                                     const Solution &solution,
                                     const RouteContext &context, Node route,
                                     Random &random) {
  auto &&insertions = star_caches[route];
  if (!insertions.empty()) {
    return;
  }
  insertions.resize(problem.num_customers);
  for (Node customer = 1; customer < problem.num_customers; ++customer) {
    insertions[customer].Reset();
  }
  Node predecessor = 0;
  Node successor = context.heads[route];
  while (true) {
    Node predecessor_customer = solution.customer(predecessor);
    Node successor_customer = solution.customer(successor);
    auto &&predecessor_distances = distance_matrix[predecessor_customer];
    auto &&successor_distances = distance_matrix[successor_customer];
    auto distance = distance_matrix[predecessor_customer][successor_customer];
    for (Node customer = 1; customer < problem.num_customers; ++customer) {
      int delta = predecessor_distances[customer] +
                  successor_distances[customer] - distance;
      insertions[customer].Add(delta, predecessor, successor, random);
    }
    if (!successor) {
      break;
    }
    predecessor = successor;
    successor = solution.successor(successor);
  }
}

template <int num>
void PreprocessRouteInsertions(const Problem &problem,
                               const DistanceMatrix &distance_matrix,
                               const Solution &solution,
                               const RouteContext &context,
                               std::vector<BestInsertion<num>> &insertions,
                               Random &random) {
  for (Node route_x = 0; route_x < context.num_routes; ++route_x) {
    Node node_x = context.heads[route_x];
    while (node_x) {
      auto &insertion = insertions[node_x];
      insertion.Reset();
      Load load = solution.load(node_x);
      for (Node route_y = 0; route_y < context.num_routes; ++route_y) {
        if (route_x == route_y ||
            context.route_loads[route_y] + load > problem.capacity) {
          continue;
        }
        Node predecessor = 0;
        Node successor = context.heads[route_y];
        Insertion best_insertion;
        best_insertion.delta = Delta(std::numeric_limits<int>::max(), -1);
        while (true) {
          int delta = CalcDelta(distance_matrix, solution, node_x, predecessor,
                                successor);
          if (best_insertion.delta.Update(delta, random)) {
            best_insertion.predecessor = predecessor;
            best_insertion.successor = successor;
          }
          if (!successor) {
            break;
          }
          predecessor = successor;
          successor = solution.successor(successor);
        }
        if (best_insertion.delta.counter > 0) {
          insertion.Add(best_insertion, random);
        }
      }
      node_x = solution.successor(node_x);
    }
  }
}

} // namespace vrp::sdvrp::splitils

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_BASE_STAR_H_
