#ifndef ALKAID_SD_SRC_ALGORITHM_RUIN_METHOD_H_
#define ALKAID_SD_SRC_ALGORITHM_RUIN_METHOD_H_

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "distance_matrix.h"
#include "model/route_context.h"
#include "model/solution.h"
#include "problem.h"
#include "util/random.h"


namespace alkaid_sd {

class RuinMethod {
public:
  virtual ~RuinMethod() = default;
  virtual std::vector<Node> Ruin(const Problem &problem,
                                 const DistanceMatrix &distance_matrix,
                                 Solution &solution, RouteContext &context,
                                 Random &random) = 0;
};

class RandomRuin : public RuinMethod {
public:
  explicit RandomRuin(std::vector<int> num_perturb_customers);
  std::vector<Node> Ruin(const Problem &problem,
                         const DistanceMatrix &distance_matrix,
                         Solution &solution, RouteContext &context,
                         Random &random) override;

private:
  std::vector<int> num_perturb_customers_;
};

class SisrsRuin : public RuinMethod {
public:
  SisrsRuin(int average_customers, int max_length, double split_rate,
            double preserved_probability);
  std::vector<Node> Ruin(const Problem &problem,
                         const DistanceMatrix &distance_matrix,
                         Solution &solution, RouteContext &context,
                         Random &random) override;

private:
  static Node GetRouteHead(Solution &solution, Node node_index, int &position);
  static void GetRoute(const Solution &solution, Node head,
                       std::vector<Node> &route);
  int average_customers_;
  int max_length_;
  double split_rate_;
  double preserved_probability_;
};

} // namespace alkaid_sd

namespace nlohmann {
template <> struct adl_serializer<std::unique_ptr<alkaid_sd::RuinMethod>> {
  static void from_json(const json &j,
                        std::unique_ptr<alkaid_sd::RuinMethod> &ruin_method) {
    if (j["type"].get<std::string>() == "SISRs") {
      auto average_customers = j["average_customers"].get<int>();
      auto max_length = j["max_length"].get<int>();
      auto split_rate = j["split_rate"].get<double>();
      auto preserved_probability = j["preserved_probability"].get<double>();
      ruin_method = std::make_unique<alkaid_sd::SisrsRuin>(
          average_customers, max_length, split_rate, preserved_probability);
    } else {
      auto num_perturb_customers =
          j["num_perturb_customers"].get<std::vector<int>>();
      ruin_method =
          std::make_unique<alkaid_sd::RandomRuin>(num_perturb_customers);
    }
  }

  static void to_json(json &, const std::unique_ptr<alkaid_sd::RuinMethod> &) {}
};

} // namespace nlohmann

#endif // ALKAID_SD_SRC_ALGORITHM_RUIN_METHOD_H_
