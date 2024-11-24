#pragma once

#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

#include <vector>

namespace alkaidsd {
  class Random;
  class RouteContext;
}  // namespace alkaidsd

namespace alkaidsd::intra_operator {
  /**
   * @brief Base class for intra-route operators.
   *
   * This class defines the interface for intra-route operators, which are used to modify a solution
   * by making changes within a single route.
   */
  class IntraOperator {
  public:
    virtual ~IntraOperator() = default;

    /**
     * @brief Executes the intra-operator on a specific route within a solution.
     * @param instance The problem instance.
     * @param route_index The index of the route to apply the operator to.
     * @param solution The solution to modify.
     * @param context The route context.
     * @param random The random number generator.
     * @return True if the operator successfully modifies the solution, false otherwise.
     */
    virtual bool operator()(const Instance &instance, Node route_index, AlkaidSolution &solution,
                            RouteContext &context, Random &random) const = 0;
  };

  /**
   * @brief Intra-route operator that performs an exchange between two nodes in a route.
   *
   * This operator swaps the positions of two nodes within a single route.
   */
  class Exchange : public IntraOperator {
  public:
    bool operator()(const Instance &instance, Node route_index, AlkaidSolution &solution,
                    RouteContext &context, Random &random) const override;
  };

  /**
   * @brief Intra-route operator that performs the Or-Opt move on a route.
   *
   * This operator moves consecutive `num` nodes from one position in a route to another.
   */
  template <int num> class OrOpt : public IntraOperator {
  public:
    bool operator()(const Instance &instance, Node route_index, AlkaidSolution &solution,
                    RouteContext &context, Random &random) const override;
  };
}  // namespace alkaidsd::intra_operator
