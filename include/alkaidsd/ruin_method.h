#pragma once

#include <alkaidsd/instance.h>
#include <alkaidsd/solution.h>

#include <vector>

namespace alkaidsd {
  class Random;
  class RouteContext;
}  // namespace alkaidsd

namespace alkaidsd::ruin_method {
  /**
   * @class RuinMethod
   * @brief Abstract base class for ruin methods.
   *
   * This class defines the interface for ruin methods.
   */
  class RuinMethod {
  public:
    /**
     * @brief Virtual destructor for RuinMethod.
     */
    virtual ~RuinMethod() = default;

    /**
     * @brief Ruin the solution by removing nodes.
     *
     * This method is responsible for removing nodes from the solution based on the specified ruin
     * method.
     *
     * @param instance The problem instance.
     * @param solution The solution to be ruined.
     * @param context The route context.
     * @param random The random number generator.
     * @return A vector of nodes that have been removed from the solution.
     */
    virtual std::vector<Node> Ruin(const Instance &instance, AlkaidSolution &solution,
                                   RouteContext &context, Random &random)
        = 0;
  };

  /**
   * @class RandomRuin
   * @brief Ruin method that randomly removes nodes from the solution.
   *
   * This class implements the RuinMethod interface and provides a random ruin method.
   */
  class RandomRuin : public RuinMethod {
  public:
    /**
     * @brief Constructor for RandomRuin.
     *
     * @param num_perturb_customers A vector of integers representing the number of customers to
     * perturb in each route.
     */
    explicit RandomRuin(std::vector<int> num_perturb_customers);

    std::vector<Node> Ruin(const Instance &instance, AlkaidSolution &solution, RouteContext &context,
                           Random &random) override;

  private:
    std::vector<int> num_perturb_customers_;
  };

  /**
   * @class SisrsRuin
   * @brief Ruin method based on the Sequential Insertion and Removal of Subroutes (SISRs)
   * algorithm.
   *
   * This class implements the RuinMethod interface and provides a ruin method based on the SISRs
   * algorithm.
   */
  class SisrsRuin : public RuinMethod {
  public:
    /**
     * @brief Constructor for SisrsRuin.
     *
     * @param average_customers The average number of customers to remove from each route.
     * @param max_length The maximum length of the subroutes to be removed.
     * @param split_rate The split rate.
     * @param preserved_probability The probability of preserving a node.
     */
    SisrsRuin(int average_customers, int max_length, double split_rate,
              double preserved_probability);

    std::vector<Node> Ruin(const Instance &instance, AlkaidSolution &solution, RouteContext &context,
                           Random &random) override;

  private:
    static Node GetRouteHead(AlkaidSolution &solution, Node node_index, int &position);
    static void GetRoute(const AlkaidSolution &solution, Node head, std::vector<Node> &route);
    int average_customers_;
    int max_length_;
    double split_rate_;
    double preserved_probability_;
  };
}  // namespace alkaidsd::ruin_method
