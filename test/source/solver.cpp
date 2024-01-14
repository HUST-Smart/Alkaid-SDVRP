#include <alkaidsd/solver.h>
#include <alkaidsd/version.h>
#include <doctest/doctest.h>

#include <string>

TEST_CASE("Large demand") {
  using namespace alkaidsd;

  Config config;
  config.time_limit = 1;
  config.blink_rate = 0.01;
  config.inter_operators.push_back(std::make_unique<inter_operator::SwapStar>());
  config.inter_operators.push_back(std::make_unique<inter_operator::SdSwapStar>());
  config.intra_operators.push_back(std::make_unique<intra_operator::Exchange>());
  config.acceptance_rule = []() { return std::make_unique<acceptance_rule::HillClimbing>(); };
  config.ruin_method = std::make_unique<ruin_method::RandomRuin>(std::vector{1});
  config.sorter.AddSortFunction(std::make_unique<sorter::SortByRandom>(), 1);

  Problem problem;
  problem.num_customers = 2;
  problem.capacity = 1;
  problem.demands = std::vector{0, 100};
  problem.distance_matrix = {{0, 1}, {1, 0}};

  auto solution = Solve(config, problem);
  CHECK(solution.NodeIndices().size() == 100);
  CHECK(solution.CalcObjective(problem) == 200);
}

TEST_CASE("AlkaidSD version") {
  static_assert(std::string_view(ALKAIDSD_VERSION) == std::string_view("1.0"));
  CHECK(std::string(ALKAIDSD_VERSION) == std::string("1.0"));
}
