#include "split_reinsertion.h"

#include <algorithm>
#include <vector>

#include "utils.h"

namespace alkaidsd {
  struct SplitReinsertionMove {
    InsertionWithCost<int> insertion;
    int residual;
    SplitReinsertionMove(const InsertionWithCost<int> &insertion, int residual)
        : insertion(insertion), residual(residual) {}
  };

  void SplitReinsertion(const Instance &instance, Node customer, int demand, double blink_rate,
                        AlkaidSolution &solution, RouteContext &context, Random &random) {
    auto func = [&](Node predecessor, Node successor, Node customer) {
      Node pre_customer = solution.Customer(predecessor);
      Node suc_customer = solution.Customer(successor);
      return instance.distance_matrix[customer][pre_customer]
             + instance.distance_matrix[customer][suc_customer]
             - instance.distance_matrix[pre_customer][suc_customer];
    };
    std::vector<SplitReinsertionMove> moves;
    moves.reserve(context.NumRoutes());
    int sum_residual = 0;
    for (Node route_index = 0; route_index < context.NumRoutes(); ++route_index) {
      int residual = std::min(demand, instance.capacity - context.Load(route_index));
      if (residual > 0) {
        auto insertion = CalcBestInsertion(solution, func, context, route_index, customer, random);
        moves.emplace_back(insertion, residual);
        sum_residual += residual;
      }
    }
    if (sum_residual < demand) {
      return;
    }
    std::stable_sort(moves.begin(), moves.end(),
                     [](const SplitReinsertionMove &lhs, const SplitReinsertionMove &rhs) {
                       return lhs.insertion.cost.value * rhs.residual
                              < rhs.insertion.cost.value * lhs.residual;
                     });
    for (const auto &move : moves) {
      sum_residual -= move.residual;
      if (sum_residual >= demand && random.NextFloat() < blink_rate) {
        continue;
      }
      int load = std::min(demand, move.residual);
      Node node_index
          = solution.Insert(customer, load, move.insertion.predecessor, move.insertion.successor);
      if (move.insertion.predecessor == 0) {
        context.SetHead(move.insertion.route_index, node_index);
      }
      context.UpdateRouteContext(solution, move.insertion.route_index, move.insertion.predecessor);
      demand -= load;
      if (demand == 0) {
        break;
      }
    }
  }
}  // namespace alkaidsd
