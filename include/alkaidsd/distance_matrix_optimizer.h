#pragma once

#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

#include <vector>

namespace alkaidsd {
  /**
   * @brief Optimizes the distance matrix by Floyd-Warshall algorithm.
   *
   * This class takes a distance matrix as input and provides methods to restore the optimized
   * solution.
   *
   * The optimization is done by Floyd-Warshall algorithm.
   */
  class DistanceMatrixOptimizer {
  public:
    /**
     * @brief Constructs a DistanceMatrixOptimizer object with a given distance matrix.
     *
     * @param distance_matrix The distance matrix to be optimized.
     */
    explicit DistanceMatrixOptimizer(std::vector<std::vector<int>>& distance_matrix);

    /**
     * @brief Restores the optimized solution to the original solution.
     *
     * @param solution The solution to be restored.
     */
    void Restore(AlkaidSolution& solution) const;

  private:
    void Restore(AlkaidSolution& solution, Node i, Node j) const;

    std::vector<std::vector<int>> original_;
    std::vector<std::vector<Node>> previous_node_indices_;
  };
}  // namespace alkaidsd
