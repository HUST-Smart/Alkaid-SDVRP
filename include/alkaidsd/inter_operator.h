#pragma once

#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

#include <vector>

namespace alkaidsd {
  class CacheMap;
  class Random;
  class RouteContext;
}  // namespace alkaidsd

namespace alkaidsd::inter_operator {
  /**
   * @class InterOperator
   * @brief Base class for inter-route operators.
   */
  class InterOperator {
  public:
    /**
     * @brief Virtual destructor for InterOperator.
     */
    virtual ~InterOperator() = default;

    /**
     * @brief Executes the inter-operator on the given solution.
     * @param instance The problem instance.
     * @param solution The solution instance.
     * @param context The route context.
     * @param random The random generator.
     * @param cache_map The cache map.
     * @return A vector of route indices representing the modified routes. Empty if the operator
     * fails to optimize the solution.
     */
    virtual std::vector<Node> operator()(const Instance &instance, AlkaidSolution &solution,
                                         RouteContext &context, Random &random,
                                         CacheMap &cache_map) const = 0;
  };

  /**
   * @class Swap
   * @brief Inter-operator that performs a Swap(num_x, num_y) operation.
   * @tparam num_x The number of nodes to swap in one route.
   * @tparam num_y The number of nodes to swap in the other route.
   */
  template <int num_x, int num_y> class Swap : public InterOperator {
  public:
    std::vector<Node> operator()(const Instance &instance, AlkaidSolution &solution, RouteContext &context,
                                 Random &random, CacheMap &cache_map) const override;
  };

  /**
   * @class Relocate
   * @brief Inter-operator that performs a Relocate operation.
   */
  class Relocate : public InterOperator {
  public:
    std::vector<Node> operator()(const Instance &instance, AlkaidSolution &solution, RouteContext &context,
                                 Random &random, CacheMap &cache_map) const override;
  };

  /**
   * @class SwapStar
   * @brief Inter-operator that performs a Swap* operation.
   */
  class SwapStar : public InterOperator {
  public:
    std::vector<Node> operator()(const Instance &instance, AlkaidSolution &solution, RouteContext &context,
                                 Random &random, CacheMap &cache_map) const override;
  };

  /**
   * @class Cross
   * @brief Inter-operator that performs a Cross operation.
   */
  class Cross : public InterOperator {
  public:
    std::vector<Node> operator()(const Instance &instance, AlkaidSolution &solution, RouteContext &context,
                                 Random &random, CacheMap &cache_map) const override;
  };

  /**
   * @class SdSwapStar
   * @brief Inter-operator that performs a SD-Swap* operation.
   */
  class SdSwapStar : public InterOperator {
  public:
    std::vector<Node> operator()(const Instance &instance, AlkaidSolution &solution, RouteContext &context,
                                 Random &random, CacheMap &cache_map) const override;
  };

  /**
   * @class SdSwapOneOne
   * @brief Inter-operator that performs a SD-Swap(1, 1) operation.
   * constraints.
   */
  class SdSwapOneOne : public InterOperator {
  public:
    std::vector<Node> operator()(const Instance &instance, AlkaidSolution &solution, RouteContext &context,
                                 Random &random, CacheMap &cache_map) const override;
  };

  /**
   * @class SdSwapTwoOne
   * @brief Inter-operator that performs a SD-Swap(2, 1) operation.
   * constraints.
   */
  class SdSwapTwoOne : public InterOperator {
  public:
    std::vector<Node> operator()(const Instance &instance, AlkaidSolution &solution, RouteContext &context,
                                 Random &random, CacheMap &cache_map) const override;
  };
}  // namespace alkaidsd::inter_operator
