#include "util/solution_utils.h"

#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace alkaid_sd {

int RemoveNode(const DistanceMatrix &distance_matrix, Node node_index,
               Node route_index, Solution &solution, RouteContext &context) {
  Node customer = solution.customer(node_index);
  Node predecessor = solution.predecessor(node_index);
  Node successor = solution.successor(node_index);
  int delta = distance_matrix[solution.customer(predecessor)]
                             [solution.customer(successor)];
  delta -= distance_matrix[customer][solution.customer(predecessor)] +
           distance_matrix[customer][solution.customer(successor)];
  solution.Remove(node_index);
  if (predecessor == 0) {
    context.heads[route_index] = successor;
  }
  UpdateRouteContext(solution, route_index, predecessor, context);
  return delta;
}

int CalcObjective(const Solution &solution,
                  const DistanceMatrix &distance_matrix) {
  int objective = 0;
  const Node *node_pool = solution.node_pool();
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node node_index = node_pool[i];
    Node predecessor = solution.predecessor(node_index);
    Node successor = solution.successor(node_index);
    objective += distance_matrix[solution.customer(node_index)]
                                [solution.customer(predecessor)];
    if (successor == 0) {
      objective += distance_matrix[solution.customer(node_index)][0];
    }
  }
  return objective;
}

void CheckFeasible(const Problem &problem, const Solution &solution) {
  std::set<Node> unused_node_indices(
      solution.node_pool(), solution.node_pool() + solution.num_nodes());
  std::vector<Node> customer_load_sums(problem.num_customers);
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node node_index = solution.node_pool()[i];
    if (!solution.predecessor(node_index)) {
      Node predecessor = 0;
      Node route_load = 0;
      while (node_index) {
        if (solution.predecessor(node_index) != predecessor) {
          throw std::runtime_error(
              "Predecessor and successor are not consistent.");
        }
        if (!unused_node_indices.erase(node_index)) {
          throw std::runtime_error("Some node is visited twice.");
        }
        Load load = solution.load(node_index);
        Node customer = solution.customer(node_index);
        if (customer <= 0 || customer >= problem.num_customers) {
          throw std::runtime_error("Invalid customer index.");
        }
        customer_load_sums[customer] += load;
        route_load += load;
        predecessor = node_index;
        node_index = solution.successor(node_index);
      }
      if (route_load > problem.capacity) {
        throw std::runtime_error("Route load exceeds the capacity.");
      }
    }
  }
  if (!unused_node_indices.empty()) {
    throw std::runtime_error("Some node leaks.");
  }
  for (Node i = 1; i < problem.num_customers; ++i) {
    if (customer_load_sums[i] < problem.customers[i].demand) {
      throw std::runtime_error("Some customer is not served fully.");
    }
    if (customer_load_sums[i] > problem.customers[i].demand) {
      throw std::runtime_error("Some customer is served exceedingly.");
    }
  }
}

void CheckRouteContextHeads(const Solution &solution,
                            const RouteContext &context) {
  std::set<Node> heads;
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node node_index = solution.node_pool()[i];
    if (!solution.predecessor(node_index)) {
      heads.insert(node_index);
    }
  }
  for (Node i = 0; i < context.num_routes; ++i) {
    Node head = context.heads[i];
    if (head == 0) {
      continue;
    }
    if (!heads.erase(head)) {
      throw std::runtime_error("Some head is not truly head.");
    }
  }
  if (!heads.empty()) {
    throw std::runtime_error("Some head is not recorded.");
  }
}

void ReversedLink(Solution &solution, Node left, Node right, Node predecessor,
                  Node successor) {
  while (true) {
    Node original_predecessor = solution.predecessor(right);
    solution.Link(predecessor, right);
    if (right == left) {
      break;
    }
    predecessor = right;
    right = original_predecessor;
  }
  solution.Link(left, successor);
}

void IgnoreUntilChar(char c, std::istream &stream) {
  stream.ignore(std::numeric_limits<std::streamsize>::max(), stream.widen(c));
}

Solution ReadSolutionToStream(std::istream &stream) {
  Solution solution;
  std::string prefix;
  while (stream >> prefix && prefix == "Route") {
    std::string line;
    std::getline(stream, line);
    std::istringstream is(line);
    IgnoreUntilChar('-', is);
    Node predecessor = 0;
    Node customer;
    while (is >> customer && customer) {
      IgnoreUntilChar('(', is);
      Load load;
      is >> load;
      predecessor = solution.Insert(customer, load, predecessor, 0);
      IgnoreUntilChar('-', is);
    }
  }
  return solution;
}

void WriteSolutionToStream(const DistanceMatrix &distance_matrix,
                           const Solution &solution,
                           const std::string &processor, double time,
                           std::ostream &stream) {
  Node num_routes = 0;
  int total_cost = 0;
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node node_index = solution.node_pool()[i];
    if (!solution.predecessor(node_index)) {
      stream << "Route " << ++num_routes << ": 0";
      Node predecessor = 0;
      while (node_index) {
        Node customer = solution.customer(node_index);
        stream << " - " << customer << " ( " << solution.load(node_index)
               << " )";
        total_cost += distance_matrix.original[predecessor][customer];
        predecessor = customer;
        node_index = solution.successor(node_index);
      }
      stream << " - 0\n";
      total_cost += distance_matrix.original[predecessor][0];
    }
  }
  stream << total_cost << '\n';
  stream << processor << std::endl;
  stream << time << std::endl;
}

} // namespace alkaid_sd
