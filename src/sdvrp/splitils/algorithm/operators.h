#ifndef DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_OPERATORS_H_
#define DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_OPERATORS_H_

#include <map>
#include <set>
#include <string>

#include <nlohmann/json.hpp>

#include "sdvrp/distance_matrix.h"
#include "sdvrp/splitils/model/operator_cache.h"
#include "sdvrp/splitils/model/route_context.h"
#include "sdvrp/splitils/model/solution.h"
#include "util/random.h"

namespace vrp::sdvrp::splitils {

using InterOperatorFunction = bool(const Problem &, const DistanceMatrix &,
                                   Solution &, RouteContext &, std::set<Node> &,
                                   Random &, OperatorCaches &);

template <int num_x, int num_y> InterOperatorFunction Swap;
InterOperatorFunction Relocate;
InterOperatorFunction SwapStar;
InterOperatorFunction Cross;
InterOperatorFunction SdSwapStar;
InterOperatorFunction SdSwapOneIn;
InterOperatorFunction SdSwapOneOne;
InterOperatorFunction SdSwapOneOut;
InterOperatorFunction SdSwapTwoOne;
InterOperatorFunction RouteAddition;
InterOperatorFunction KSplit;

static const std::map<std::string, InterOperatorFunction *>
    kInterOperatorFunctionMap = {{"Swap<1, 0>", Swap<1, 0>},
                                 {"Swap<2, 0>", Swap<2, 0>},
                                 {"Swap<1, 1>", Swap<1, 1>},
                                 {"Swap<2, 1>", Swap<2, 1>},
                                 {"Swap<2, 2>", Swap<2, 2>},
                                 {"Relocate", Relocate},
                                 {"SwapStar", SwapStar},
                                 {"Cross", Cross},
                                 {"SdSwapStar", SdSwapStar},
                                 {"SdSwapOneIn", SdSwapOneIn},
                                 {"SdSwapOneOne", SdSwapOneOne},
                                 {"SdSwapOneOut", SdSwapOneOut},
                                 {"SdSwapTwoOne", SdSwapTwoOne},
                                 {"RouteAddition", RouteAddition},
                                 {"KSplit", KSplit}};

static const std::map<InterOperatorFunction *, bool> kInterOperatorCacheMap = {
    {Swap<1, 0>, true},   {Swap<2, 0>, true},     {Swap<1, 1>, true},
    {Swap<2, 1>, true},   {Swap<2, 2>, true},     {Relocate, true},
    {SwapStar, true},     {Cross, true},          {SdSwapStar, true},
    {SdSwapOneIn, true},  {SdSwapOneOne, true},   {SdSwapOneOut, true},
    {SdSwapTwoOne, true}, {RouteAddition, false}, {KSplit, false}};

using IntraOperatorFunction = bool(const Problem &, const DistanceMatrix &,
                                   Node, Solution &, RouteContext &, Random &);

IntraOperatorFunction TwoOpt;
IntraOperatorFunction Exchange;
template <int num> IntraOperatorFunction OrOpt;

static const std::map<std::string, IntraOperatorFunction *>
    kIntraOperatorFunctionMap = {{"TwoOpt", TwoOpt},
                                 {"Exchange", Exchange},
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

} // namespace vrp::sdvrp::splitils

namespace nlohmann {
using vrp::sdvrp::splitils::InterOperatorFunction;
template <> struct adl_serializer<std::vector<InterOperatorFunction *>> {
  static void from_json(const json &j,
                        std::vector<InterOperatorFunction *> &functions) {
    for (auto &&name : j) {
      functions.emplace_back(
          vrp::sdvrp::splitils::kInterOperatorFunctionMap.at(name));
    }
  }
  static void to_json(json &, const std::vector<InterOperatorFunction *> &) {}
};
using vrp::sdvrp::splitils::IntraOperatorFunction;
template <> struct adl_serializer<std::vector<IntraOperatorFunction *>> {
  static void from_json(const json &j,
                        std::vector<IntraOperatorFunction *> &functions) {
    for (auto &&name : j) {
      functions.emplace_back(
          vrp::sdvrp::splitils::kIntraOperatorFunctionMap.at(name));
    }
  }
  static void to_json(json &, const std::vector<IntraOperatorFunction *> &) {}
};
}; // namespace nlohmann

#endif // DIMACS_12_SRC_SDVRP_SPLITILS_ALGORITHM_OPERATORS_H_
