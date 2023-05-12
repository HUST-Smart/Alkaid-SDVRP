#ifndef ALKAID_SD_SRC_ALGORITHM_OPERATORS_H_
#define ALKAID_SD_SRC_ALGORITHM_OPERATORS_H_

#include <map>
#include <set>
#include <string>

#include <nlohmann/json.hpp>

#include "distance_matrix.h"
#include "model/operator_cache.h"
#include "model/route_context.h"
#include "model/solution.h"
#include "util/random.h"

namespace alkaid_sd {

using InterOperatorFunction = bool(const Problem &, const DistanceMatrix &,
                                   Solution &, RouteContext &, std::set<Node> &,
                                   Random &, OperatorCaches &);

template <int num_x, int num_y> InterOperatorFunction Swap;
InterOperatorFunction Relocate;
InterOperatorFunction SwapStar;
InterOperatorFunction Cross;
InterOperatorFunction SdSwapStar;
InterOperatorFunction SdSwapOneOne;
InterOperatorFunction SdSwapTwoOne;

static const std::map<std::string, InterOperatorFunction *>
    kInterOperatorFunctionMap = {
        {"Swap<2, 0>", Swap<2, 0>},    {"Swap<2, 1>", Swap<2, 1>},
        {"Swap<2, 2>", Swap<2, 2>},    {"Relocate", Relocate},
        {"SwapStar", SwapStar},        {"Cross", Cross},
        {"SdSwapStar", SdSwapStar},    {"SdSwapOneOne", SdSwapOneOne},
        {"SdSwapTwoOne", SdSwapTwoOne}};

static const std::map<InterOperatorFunction *, bool> kInterOperatorCacheMap = {
    {Swap<2, 0>, true}, {Swap<2, 1>, true},   {Swap<2, 2>, true},
    {Relocate, true},   {SwapStar, true},     {Cross, true},
    {SdSwapStar, true}, {SdSwapOneOne, true}, {SdSwapTwoOne, true},
};

using IntraOperatorFunction = bool(const Problem &, const DistanceMatrix &,
                                   Node, Solution &, RouteContext &, Random &);

IntraOperatorFunction Exchange;
template <int num> IntraOperatorFunction OrOpt;

static const std::map<std::string, IntraOperatorFunction *>
    kIntraOperatorFunctionMap = {{"Exchange", Exchange},
                                 {"OrOpt<1>", OrOpt<1>},
                                 {"OrOpt<2>", OrOpt<2>},
                                 {"OrOpt<3>", OrOpt<3>}};

int SplitReinsertion(const Problem &problem,
                     const DistanceMatrix &distance_matrix, Node customer,
                     int demand, double blink_rate, Solution &solution,
                     RouteContext &context, std::set<Node> &associated_routes,
                     Random &random);

void Repair(const DistanceMatrix &distance_matrix, Node route_index,
            Solution &solution, RouteContext &context);

} // namespace alkaid_sd

namespace nlohmann {
using alkaid_sd::InterOperatorFunction;
template <> struct adl_serializer<std::vector<InterOperatorFunction *>> {
  static void from_json(const json &j,
                        std::vector<InterOperatorFunction *> &functions) {
    for (auto &&name : j) {
      functions.emplace_back(alkaid_sd::kInterOperatorFunctionMap.at(name));
    }
  }
  static void to_json(json &, const std::vector<InterOperatorFunction *> &) {}
};
using alkaid_sd::IntraOperatorFunction;
template <> struct adl_serializer<std::vector<IntraOperatorFunction *>> {
  static void from_json(const json &j,
                        std::vector<IntraOperatorFunction *> &functions) {
    for (auto &&name : j) {
      functions.emplace_back(alkaid_sd::kIntraOperatorFunctionMap.at(name));
    }
  }
  static void to_json(json &, const std::vector<IntraOperatorFunction *> &) {}
};
}; // namespace nlohmann

#endif // ALKAID_SD_SRC_ALGORITHM_OPERATORS_H_
