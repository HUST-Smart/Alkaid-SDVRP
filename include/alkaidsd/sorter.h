#pragma once

#include <alkaidsd/instance.h>

#include <memory>
#include <vector>

namespace alkaidsd {
  class Random;
}  // namespace alkaidsd

namespace alkaidsd::sorter {
  /**
   * @brief Abstract base class for sort operators.
   */
  class SortOperator {
  public:
    /**
     * @brief Virtual destructor for SortOperator.
     */
    virtual ~SortOperator() = default;

    /**
     * @brief Sorts the given vector of customers based on a specific criterion.
     * @param instance The problem instance.
     * @param customers customers to be sorted.
     * @param random The random number generator.
     */
    virtual void operator()(const Instance &instance, std::vector<Node> &customers,
                            Random &random) const = 0;
  };

  /**
   * @brief Class for sorting customers using different sort operators.
   */
  class Sorter {
  public:
    /**
     * @brief Adds a sort operator with a specified weight.
     * @param sort_function The sort operator to be added.
     * @param weight The weight of the sort operator.
     */
    void AddSortFunction(std::unique_ptr<SortOperator> &&sort_function, double weight);

    /**
     * @brief Sorts the given vector of customers using the added sort operators.
     * @param instance The problem instance.
     * @param customers customers to be sorted.
     * @param random The random number generator.
     */
    void Sort(const Instance &instance, std::vector<Node> &customers, Random &random) const;

  private:
    double sum_weights_ = 0;
    std::vector<std::pair<std::unique_ptr<SortOperator>, double>> sort_functions_;
  };

  /**
   * @brief Sort operator that randomly shuffles customers.
   */
  class SortByRandom : public SortOperator {
  public:
    void operator()(const Instance &instance, std::vector<Node> &customers,
                    Random &random) const override;
  };

  /**
   * @brief Sort operator that sorts customers based on their demand in descending order.
   */
  class SortByDemand : public SortOperator {
  public:
    void operator()(const Instance &instance, std::vector<Node> &customers,
                    Random &random) const override;
  };

  /**
   * @brief Sort operator that sorts customers based on their distance to the depot in descending
   * order.
   */
  class SortByFar : public SortOperator {
  public:
    void operator()(const Instance &instance, std::vector<Node> &customers,
                    Random &random) const override;
  };

  /**
   * @brief Sort operator that sorts customers based on their distance to the depot in ascending
   * order.
   */
  class SortByClose : public SortOperator {
  public:
    void operator()(const Instance &instance, std::vector<Node> &customers,
                    Random &random) const override;
  };
}  // namespace alkaidsd::sorter
