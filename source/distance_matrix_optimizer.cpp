#include <alkaidsd/distance_matrix_optimizer.h>

#include <cmath>

namespace alkaidsd {
  DistanceMatrixOptimizer::DistanceMatrixOptimizer(std::vector<std::vector<int>> &distance_matrix)
      : original_(distance_matrix),
        previous_node_indices_(distance_matrix.size(), std::vector<Node>(distance_matrix.size())) {
    Node num_customers = static_cast<Node>(distance_matrix.size());
    for (Node k = 1; k < num_customers; ++k) {
      for (Node i = 0; i < num_customers; ++i) {
        for (Node j = 0; j < num_customers; ++j) {
          int distance = distance_matrix[i][k] + distance_matrix[k][j];
          if (distance_matrix[i][j] > distance) {
            distance_matrix[i][j] = distance;
            previous_node_indices_[i][j] = k;
          }
        }
      }
    }
  }

  void DistanceMatrixOptimizer::Restore(AlkaidSolution &solution, Node i, Node j) const {
    Node customer = previous_node_indices_[solution.Customer(i)][solution.Customer(j)];
    if (customer != 0) {
      Node k = solution.Insert(customer, 0, i, j);
      Restore(solution, i, k);
      Restore(solution, k, j);
    }
  }

  void DistanceMatrixOptimizer::Restore(AlkaidSolution &solution) const {
    std::vector<Node> heads;
    for (Node node_index : solution.NodeIndices()) {
      if (!solution.Predecessor(node_index)) {
        heads.push_back(node_index);
      }
    }
    for (Node node_index : heads) {
      Node predecessor = 0;
      while (node_index) {
        Restore(solution, predecessor, node_index);
        predecessor = node_index;
        node_index = solution.Successor(node_index);
      }
      Restore(solution, predecessor, 0);
    }
  }
}  // namespace alkaidsd
