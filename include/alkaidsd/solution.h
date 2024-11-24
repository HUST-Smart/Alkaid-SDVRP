#pragma once

#include <alkaidsd/instance.h>

#include <ostream>
#include <vector>

namespace alkaidsd {
  /**
   * @brief The solution representation to a problem instance.
   */
  class Solution {
  public:
    /**
     * @brief Calculate the objective value of the solution.
     *
     * @param instance The problem instance.
     * @return The objective value.
     */
    virtual int CalcObjective(const Instance& instance) const = 0;
  };

  /**
   * @brief The AlkaidSolution class represents a solution to a problem instance.
   */
  class AlkaidSolution : public Solution {
  public:
    /**
     * @brief Default constructor for the AlkaidSolution class.
     *
     * Initializes the solution with only a depot node.
     */
    AlkaidSolution() { node_data_.push_back({}); }

    /**
     * @brief Get the predecessor node of a given node.
     *
     * @param node_index The index of the node.
     * @return The index of the predecessor node.
     */
    Node Predecessor(Node node_index) const { return node_data_[node_index].predecessor; }

    /**
     * @brief Get the successor node of a given node.
     *
     * @param node_index The index of the node.
     * @return The index of the successor node.
     */
    Node Successor(Node node_index) const { return node_data_[node_index].successor; }

    /**
     * @brief Get the customer of a given node.
     *
     * @param node_index The index of the node.
     * @return The index of the customer.
     */
    Node Customer(Node node_index) const { return node_data_[node_index].customer; }

    /**
     * @brief Get the load of a given node.
     *
     * @param node_index The index of the node.
     * @return The load of the node.
     */
    int Load(Node node_index) const { return node_data_[node_index].load; }

    /**
     * @brief Set the predecessor node of a given node.
     *
     * @param node_index The index of the node.
     * @param predecessor The index of the predecessor node.
     */
    void SetPredecessor(Node node_index, Node predecessor) {
      node_data_[node_index].predecessor = predecessor;
    }

    /**
     * @brief Set the successor node of a given node.
     *
     * @param node_index The index of the node.
     * @param successor The index of the successor node.
     */
    void SetSuccessor(Node node_index, Node successor) {
      node_data_[node_index].successor = successor;
    }

    /**
     * @brief Set the customer of a given node.
     *
     * @param node_index The index of the node.
     * @param customer The index of the customer.
     */
    void SetCustomer(Node node_index, Node customer) { node_data_[node_index].customer = customer; }

    /**
     * @brief Set the load of a given node.
     *
     * @param node_index The index of the node.
     * @param load The load of the node.
     */
    void SetLoad(Node node_index, int load) { node_data_[node_index].load = load; }

    /**
     * @brief Remove a node from the solution.
     *
     * @param node_index The index of the node to be removed.
     */
    void Remove(Node node_index) {
      Node predecessor = this->Predecessor(node_index);
      Node successor = this->Successor(node_index);
      Link(predecessor, successor);
      Node index_in_used_nodes = this->node_data_[node_index].index_in_used_nodes;
      Node last_node = used_nodes_.back();
      node_data_[last_node].index_in_used_nodes = index_in_used_nodes;
      used_nodes_[index_in_used_nodes] = last_node;
      used_nodes_.pop_back();
      unused_nodes_.push_back(node_index);
    }

    /**
     * @brief Insert a new node into the solution.
     *
     * @param customer The index of the customer for the new node.
     * @param load The load for the new node.
     * @param predecessor The index of the predecessor node.
     * @param successor The index of the successor node.
     * @return The index of the new node.
     */
    Node Insert(Node customer, int load, Node predecessor, Node successor) {
      Node node_index = NewNode(customer, load);
      Link(predecessor, node_index);
      Link(node_index, successor);
      return node_index;
    }

    /**
     * @brief Create a new node in the solution.
     *
     * @param customer The index of the customer for the new node.
     * @param load The load for the new node.
     * @return The index of the new node.
     */
    Node NewNode(Node customer, int load) {
      Node node_index;
      if (unused_nodes_.empty()) {
        node_index = node_data_.size();
        node_data_.push_back({});
      } else {
        node_index = unused_nodes_.back();
        unused_nodes_.pop_back();
      }
      node_data_[node_index].index_in_used_nodes = used_nodes_.size();
      used_nodes_.push_back(node_index);
      SetCustomer(node_index, customer);
      SetLoad(node_index, load);
      return node_index;
    }

    /**
     * @brief Link two nodes together.
     *
     * @param predecessor The index of the predecessor node.
     * @param successor The index of the successor node.
     */
    void Link(Node predecessor, Node successor) {
      SetPredecessor(successor, predecessor);
      SetSuccessor(predecessor, successor);
    }

    /**
     * @brief Get the indices of all used nodes in the solution.
     *
     * @return A vector containing the indices of all used nodes.
     */
    const std::vector<Node>& NodeIndices() const { return used_nodes_; }

    /**
     * @brief Get the maximum node index in the solution.
     *
     * @return The maximum node index.
     */
    Node MaxNodeIndex() const { return node_data_.size() - 1; }

    /**
     * @brief Calculate the objective value of the solution.
     *
     * @param instance The problem instance.
     * @return The objective value.
     */
    virtual int CalcObjective(const Instance& instance) const override {
      int objective = 0;
      for (Node node_index : NodeIndices()) {
        Node predecessor = Predecessor(node_index);
        Node successor = Successor(node_index);
        objective += instance.distance_matrix[Customer(node_index)][Customer(predecessor)];
        if (successor == 0) {
          objective += instance.distance_matrix[Customer(node_index)][0];
        }
      }
      return objective;
    }

    /**
     * @brief Reverse the links between two nodes.
     *
     * @param left The index of the left node.
     * @param right The index of the right node.
     * @param predecessor The index of the predecessor node.
     * @param successor The index of the successor node.
     */
    void ReversedLink(Node left, Node right, Node predecessor, Node successor) {
      while (true) {
        Node original_predecessor = Predecessor(right);
        Link(predecessor, right);
        if (right == left) {
          break;
        }
        predecessor = right;
        right = original_predecessor;
      }
      Link(left, successor);
    }

    /**
     * @brief Overload the << operator to print the solution.
     *
     * @param os The output stream.
     * @param solution The solution to be printed.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const AlkaidSolution& solution) {
      Node num_routes = 0;
      for (Node node_index : solution.NodeIndices()) {
        if (!solution.Predecessor(node_index)) {
          os << "Route " << ++num_routes << ": 0";
          while (node_index) {
            Node customer = solution.Customer(node_index);
            os << " - " << customer << " ( " << solution.Load(node_index) << " )";
            node_index = solution.Successor(node_index);
          }
          os << " - 0\n";
        }
      }
      return os;
    }

    /**
     * @brief Print the solution in json format.
     *
     * @param os The output stream.
     * @param solution The solution to be printed.
     * @return The output stream.
     */
    std::ostream& PrintJson(std::ostream& os) {
      os << "[";
      for (Node node_index : NodeIndices()) {
        if (!Predecessor(node_index)) {
          os << "[{ \"customer\": 0, \"quantity\": 0 }";
          while (node_index) {
            Node customer = Customer(node_index);
            os << ", { \"customer\": " << customer << ", \"quantity\": " << Load(node_index)
               << " }";
            node_index = Successor(node_index);
          }
          os << ",{ \"customer\": 0, \"quantity\": 0 }],\n";
        }
      }
      os.seekp(-2, std::ios::cur);
      return os << "]";
    }

  private:
    struct NodeData {
      Node successor;
      Node predecessor;
      Node customer;
      int load;
      Node index_in_used_nodes;
    };
    std::vector<NodeData> node_data_;
    std::vector<Node> used_nodes_;
    std::vector<Node> unused_nodes_;
  };
}  // namespace alkaidsd
