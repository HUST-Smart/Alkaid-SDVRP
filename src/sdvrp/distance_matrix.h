#ifndef DIMACS_12_SRC_SDVRP_DISTANCE_MATRIX_H_
#define DIMACS_12_SRC_SDVRP_DISTANCE_MATRIX_H_

#include <array>
#include <memory>

#include "sdvrp/limit.h"
#include "sdvrp/problem.h"

namespace vrp::sdvrp {

template <class T>
using Matrix = std::array<std::array<T, kMaxNumCustomers>, kMaxNumCustomers>;

struct DistanceMatrix : Matrix<int> {
  Matrix<int> original;
  Matrix<Node> previous_node_indices;
};

std::unique_ptr<DistanceMatrix> CalcDistanceMatrix(const Problem &problem);

} // namespace vrp::sdvrp

#endif // DIMACS_12_SRC_SDVRP_DISTANCE_MATRIX_H_
