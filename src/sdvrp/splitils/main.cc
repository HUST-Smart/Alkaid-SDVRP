#include <chrono>
#include <optional>
#include <random>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <nlohmann/json.hpp>

#include "sdvrp/logo.h"
#include "sdvrp/problem.h"
#include "sdvrp/splitils/algorithm/splitils.h"
#include "sdvrp/splitils/util/solution_utils.h"
#include "sdvrp/visualization.h"

int main(int argc, char **argv) {
  std::cerr << vrp::sdvrp::splitils::ConvertConfigToString(
                   vrp::sdvrp::splitils::Config())
            << std::endl;
  auto start_time = std::chrono::system_clock::now();
  CLI::App app{
      std::string{vrp::sdvrp::kLogo} +
      "A program used to participate in the sdvrp track of DIMACS-12 "
      "competition\n(http://dimacs.rutgers.edu/programs/challenge/vrp/vrpsd/)"};
  std::string config_path;
  app.add_option("config", config_path, "Config path")->required();
  std::string instance_name;
  app.add_option("instance", instance_name, "Instance name")->required();
  std::string output1;
  app.add_option("output1", output1, "Output1")->required();
  std::string output2;
  app.add_option("output2", output2, "Output2")->required();
  std::optional<int> random_seed;
  app.add_option("-s", random_seed, "Random Seed");
  int lower_bound = 0;
  app.add_option("-l", lower_bound, "Lower bound");
  CLI11_PARSE(app, argc, argv);
  nlohmann::json default_value = {{"random_seed", std::random_device{}()}};
  nlohmann::json overwrite_value;
  if (random_seed) {
    overwrite_value["random_seed"] = random_seed.value();
  }
  auto config = vrp::sdvrp::splitils::ReadConfigFromFile(
      config_path, default_value, overwrite_value);
  auto problem_path =
      vrp::sdvrp::GetProblemPath(config.problem_dir, instance_name);
  auto problem = vrp::sdvrp::ReadProblemFromFile(problem_path);
  auto distance_matrix = vrp::sdvrp::CalcDistanceMatrix(problem);
  auto solution = vrp::sdvrp::splitils::SplitIlsAlgorithm(
      config, problem, *distance_matrix, lower_bound);
  //  vrp::sdvrp::splitils::RecoverFloyd(*distance_matrix, *solution);
  //  vrp::sdvrp::splitils::Reflow(problem, *solution);
  auto new_solution = *solution;
  auto end_time = std::chrono::system_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::duration<double>>(
                  end_time - start_time)
                  .count();
  vrp::sdvrp::splitils::RecoverDijkstra(problem, *distance_matrix, *solution);
  vrp::sdvrp::splitils::RecoverDijkstraForce(problem, *distance_matrix,
                                             new_solution);
  std::ofstream ofs1(output1);
  WriteSolutionToStream(*distance_matrix, *solution, time, ofs1);
  std::ofstream ofs2(output2);
  WriteSolutionToStream(*distance_matrix, new_solution, time, ofs2);
  vrp::sdvrp::ShowGraphUrl(
      vrp::sdvrp::SolutionToGraph(problem, *distance_matrix, new_solution));
  return 0;
}
