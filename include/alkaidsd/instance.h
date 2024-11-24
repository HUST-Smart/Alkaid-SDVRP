#pragma once

#include <vector>

namespace alkaidsd {
  /**
   * @brief Represents a node in the instance.
   */
  using Node = short;

  /**
   * @brief Struct that defines the problem instance.
   */
  struct Instance {
    Node num_customers;       /**< The number of customers, including the depot. */
    int capacity;             /**< The capacity of the vehicles. */
    std::vector<int> demands; /**< The demands of each customer, including the depot. */
    std::vector<std::vector<int>>
        distance_matrix; /**< The distance matrix between customers, including the depot. */
  };
}  // namespace alkaidsd
