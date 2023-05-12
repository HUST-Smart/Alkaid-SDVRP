#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_SOLUTION_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_SOLUTION_H_

#include <algorithm>
#include <numeric>
#include <utility>

#include "sdvrp/limit.h"
#include "sdvrp/splitils/limit.h"

namespace vrp::sdvrp::splitils {

class Solution {
public:
  Solution();
  [[nodiscard]] Node predecessor(Node node_index) const;
  [[nodiscard]] Node successor(Node node_index) const;
  [[nodiscard]] Node customer(Node node_index) const;
  [[nodiscard]] Load load(Node node_index) const;
  [[nodiscard]] Node num_nodes() const;
  [[nodiscard]] const Node *node_pool() const;
  void set_predecessor(Node node_index, Node predecessor);
  void set_successor(Node node_index, Node successor);
  void set_customer(Node node_index, Node customer);
  void set_load(Node node_index, Load load);
  void Remove(Node node_index);
  Node Insert(Node customer, Load load, Node predecessor, Node successor);
  Node NewNode(Node customer, Load load);
  void Link(Node predecessor, Node successor);

private:
  struct NodeImpl {
    Node successor;
    Node predecessor;
    Node customer;
    Load load;
  };
  NodeImpl node_impls_[kMaxNumNodes]{};
  //  Node predecessors_[kMaxNumNodes]{};
  //  Node successors_[kMaxNumNodes]{};
  //  Node customers_[kMaxNumNodes]{};
  //  Load loads_[kMaxNumNodes]{};
  Node node_pool_[kMaxNumNodes]{};
  Node pool_indices_[kMaxNumNodes]{};
  Node num_nodes_;
};

inline Solution::Solution() {
  set_load(0, 0);
  set_customer(0, 0);
  num_nodes_ = 0;
  std::iota(node_pool_, node_pool_ + kMaxNumNodes, 1);
}

inline Node Solution::predecessor(Node node_index) const {
  //  return predecessors_[node_index];
  return node_impls_[node_index].predecessor;
}

inline Node Solution::successor(Node node_index) const {
  //  return successors_[node_index];
  return node_impls_[node_index].successor;
}

inline Node Solution::customer(Node node_index) const {
  //  return customers_[node_index];
  return node_impls_[node_index].customer;
}

inline Load Solution::load(Node node_index) const {
  //  return loads_[node_index];
  return node_impls_[node_index].load;
}

inline Node Solution::num_nodes() const { return num_nodes_; }

inline const Node *Solution::node_pool() const { return node_pool_; }

inline void Solution::set_predecessor(Node node_index, Node predecessor) {
  //  predecessors_[node_index] = predecessor;
  node_impls_[node_index].predecessor = predecessor;
}

inline void Solution::set_successor(Node node_index, Node successor) {
  //  successors_[node_index] = successor;
  node_impls_[node_index].successor = successor;
}

inline void Solution::set_customer(Node node_index, Node customer) {
  //  customers_[node_index] = customer;
  node_impls_[node_index].customer = customer;
}

inline void Solution::set_load(Node node_index, Load load) {
  //  loads_[node_index] = load;
  node_impls_[node_index].load = load;
}

inline void Solution::Remove(Node node_index) {
  Node predecessor = this->predecessor(node_index);
  Node successor = this->successor(node_index);
  Link(predecessor, successor);
  Node pool_index = pool_indices_[node_index];
  pool_indices_[node_pool_[num_nodes_ - 1]] = pool_index;
  std::swap(node_pool_[pool_index], node_pool_[num_nodes_ - 1]);
  --num_nodes_;
}

inline Node Solution::Insert(Node customer, Load load, Node predecessor,
                             Node successor) {
  Node node_index = NewNode(customer, load);
  Link(predecessor, node_index);
  Link(node_index, successor);
  return node_index;
}

inline Node Solution::NewNode(Node customer, Load load) {
  Node node_index = node_pool_[num_nodes_];
  pool_indices_[node_index] = num_nodes_++;
  set_customer(node_index, customer);
  set_load(node_index, load);
  return node_index;
}

inline void Solution::Link(Node predecessor, Node successor) {
  set_predecessor(successor, predecessor);
  set_successor(predecessor, successor);
}

} // namespace vrp::sdvrp::splitils

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_MODEL_SOLUTION_H_
