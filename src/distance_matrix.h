#ifndef ALKAID_SD_SRC_DISTANCE_MATRIX_H_
#define ALKAID_SD_SRC_DISTANCE_MATRIX_H_

#include <array>
#include <memory>

#include "problem.h"

namespace alkaid_sd {

template <class T>
using Matrix = std::array<std::array<T, kMaxNumCustomers>, kMaxNumCustomers>;

struct DistanceMatrix : Matrix<int> {
  Matrix<int> original;
  Matrix<Node> previous_node_indices;
};

std::unique_ptr<DistanceMatrix> CalcDistanceMatrix(const Problem &problem);

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_DISTANCE_MATRIX_H_
