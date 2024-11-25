#include <alkaidsd/ruin_method.h>

#include <algorithm>
#include <numeric>
#include <set>
#include <vector>

#include "random.h"
#include "route_context.h"

namespace alkaidsd::ruin_method {
  RandomRuin::RandomRuin(std::vector<int> num_perturb_customers)
      : num_perturb_customers_(std::move(num_perturb_customers)) {}

  std::vector<Node> RandomRuin::Ruin(const Instance &instance, [[maybe_unused]] AlkaidSolution &solution,
                                     [[maybe_unused]] RouteContext &context, Random &random) {
    int num_perturb = num_perturb_customers_[random.NextInt(
        0, static_cast<int>(num_perturb_customers_.size()) - 1)];
    std::vector<Node> customers(instance.num_customers - 1);
    std::iota(customers.begin(), customers.end(), 1);
    random.Shuffle(customers.begin(), customers.end());
    customers.erase(customers.begin() + num_perturb, customers.end());
    return customers;
  }

  SisrsRuin::SisrsRuin(int average_customers, int max_length, double split_rate,
                       double preserved_probability)
      : average_customers_(average_customers),
        max_length_(max_length),
        split_rate_(split_rate),
        preserved_probability_(preserved_probability) {}

  std::vector<Node> SisrsRuin::Ruin(const Instance &instance, AlkaidSolution &solution,
                                    RouteContext &context, Random &random) {
    double average_length = static_cast<double>(instance.num_customers - 1) / context.NumRoutes();
    double max_length = std::min(static_cast<double>(max_length_), average_length);
    double max_strings = 4.0 * average_customers_ / (1 + max_length_) - 1;
    size_t num_strings = static_cast<size_t>(random.NextFloat() * max_strings) + 1;
    int customer_seed = random.NextInt(1, instance.num_customers - 1);
    std::vector<Node> node_indices(solution.NodeIndices());
    auto &&seed_distances = instance.distance_matrix[customer_seed];
    std::stable_sort(node_indices.begin(), node_indices.end(), [&](Node lhs, Node rhs) {
      return seed_distances[solution.Customer(lhs)] < seed_distances[solution.Customer(rhs)];
    });
    std::set<Node> visited_heads;
    std::vector<Node> route;
    std::vector<Node> customer_indices;
    for (Node node_index : node_indices) {
      if (visited_heads.size() >= num_strings) {
        break;
      }
      int position;
      int head = GetRouteHead(solution, node_index, position);
      if (!visited_heads.insert(head).second) {
        continue;
      }
      GetRoute(solution, head, route);
      int route_length = static_cast<int>(route.size());
      double max_ruin_length = std::min(static_cast<double>(route_length), max_length);
      int ruin_length = static_cast<int>(random.NextFloat() * max_ruin_length) + 1;
      int num_preserved = 0;
      int preserved_start_position = -1;
      if (ruin_length >= 2 && ruin_length < route_length && random.NextFloat() < split_rate_) {
        while (ruin_length < route_length) {
          if (random.NextFloat() < preserved_probability_) {
            break;
          }
          ++num_preserved;
          ++ruin_length;
        }
        preserved_start_position = random.NextInt(1, ruin_length - num_preserved - 1);
      }
      int min_start_position = std::max(0, position - ruin_length + 1);
      int max_start_position = std::min(route_length - ruin_length, position);
      int start_position = random.NextInt(min_start_position, max_start_position);
      for (int j = 0; j < ruin_length; ++j) {
        if (j < preserved_start_position || j >= preserved_start_position + num_preserved) {
          customer_indices.emplace_back(solution.Customer(route[start_position + j]));
        }
      }
    }
    std::sort(customer_indices.begin(), customer_indices.end());
    customer_indices.erase(std::unique(customer_indices.begin(), customer_indices.end()),
                           customer_indices.end());
    random.Shuffle(customer_indices.begin(), customer_indices.end());
    return customer_indices;
  }

  Node SisrsRuin::GetRouteHead(AlkaidSolution &solution, Node node_index, int &position) {
    position = 0;
    while (true) {
      Node predecessor = solution.Predecessor(node_index);
      if (!predecessor) {
        return node_index;
      }
      node_index = predecessor;
      ++position;
    }
  }

  void SisrsRuin::GetRoute(const AlkaidSolution &solution, Node head, std::vector<Node> &route) {
    route.clear();
    while (head) {
      route.emplace_back(head);
      head = solution.Successor(head);
    }
  }
}  // namespace alkaidsd::ruin_method
