#include "sdvrp/distance_matrix.h"

#include <cmath>

namespace vrp::sdvrp {

std::unique_ptr<DistanceMatrix> CalcDistanceMatrix(const Problem &problem) {
  auto distance_matrix = std::make_unique<DistanceMatrix>();
  for (Node i = 0; i < problem.num_customers; ++i) {
    for (Node j = 0; j < problem.num_customers; ++j) {
      const Customer &task_i = problem.customers[i];
      const Customer &task_j = problem.customers[j];
      (*distance_matrix)[i][j] =
          lround(hypot(task_i.x - task_j.x, task_i.y - task_j.y));
      distance_matrix->original[i][j] = (*distance_matrix)[i][j];
      distance_matrix->previous_node_indices[i][j] = -1;
    }
  }
  for (Node k = 1; k < problem.num_customers; ++k) {
    for (Node i = 0; i < problem.num_customers; ++i) {
      for (Node j = 0; j < problem.num_customers; ++j) {
        int distance = (*distance_matrix)[i][k] + (*distance_matrix)[k][j];
        if ((*distance_matrix)[i][j] > distance) {
          (*distance_matrix)[i][j] = distance;
          distance_matrix->previous_node_indices[i][j] = k;
        }
      }
    }
  }
  return distance_matrix;
}

} // namespace vrp::sdvrp
