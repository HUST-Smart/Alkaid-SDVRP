#include "sdvrp/splitils/algorithm/operators.h"

#include <vector>

#include "sdvrp/splitils/util/utils.h"

namespace vrp::sdvrp::splitils {

struct SplitReinsertionMove {
  InsertionWithCost<int> insertion;
  int residual;
  SplitReinsertionMove(const InsertionWithCost<int> &insertion, int residual)
      : insertion(insertion), residual(residual) {}
};

int SplitReinsertion(const Problem &problem,
                     const DistanceMatrix &distance_matrix, Node customer,
                     int demand, double blink_rate, Solution &solution,
                     RouteContext &context, std::set<Node> &associated_routes,
                     Random &random) {
  auto func = [&](Node predecessor, Node successor, Node customer) {
    Node pre_customer = solution.customer(predecessor);
    Node suc_customer = solution.customer(successor);
    return distance_matrix[customer][pre_customer] +
           distance_matrix[customer][suc_customer] -
           distance_matrix[pre_customer][suc_customer];
  };
  std::vector<SplitReinsertionMove> moves;
  moves.reserve(context.num_routes);
  int sum_residual = 0;
  for (Node route_index = 0; route_index < context.num_routes; ++route_index) {
    int residual =
        std::min(demand, problem.capacity - context.route_loads[route_index]);
    if (residual > 0) {
      auto insertion = CalcBestInsertion(solution, func, context, route_index,
                                         customer, random);
      moves.emplace_back(insertion, residual);
      sum_residual += residual;
    }
  }
  if (sum_residual < demand) {
    return -1;
  }
  std::stable_sort(
      moves.begin(), moves.end(),
      [](const SplitReinsertionMove &lhs, const SplitReinsertionMove &rhs) {
        return lhs.insertion.cost.value * rhs.residual <
               rhs.insertion.cost.value * lhs.residual;
      });
  int delta = 0;
  for (const auto &move : moves) {
    sum_residual -= move.residual;
    if (sum_residual >= demand && random.NextFloat() < blink_rate) {
      continue;
    }
    Load load = std::min(demand, move.residual);
    delta += move.insertion.cost.value;
    Node node_index = solution.Insert(
        customer, load, move.insertion.predecessor, move.insertion.successor);
    if (move.insertion.predecessor == 0) {
      context.heads[move.insertion.route_index] = node_index;
    }
    UpdateRouteContext(solution, move.insertion.route_index,
                       move.insertion.predecessor, context);
    associated_routes.insert(move.insertion.route_index);
    demand -= load;
    if (demand == 0) {
      break;
    }
  }
  return delta;
}

} // namespace vrp::sdvrp::splitils
