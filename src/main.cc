#include <chrono>
#include <optional>
#include <random>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <nlohmann/json.hpp>

#include "algorithm/solver.h"
#include "logo.h"
#include "problem.h"
#include "util/solution_utils.h"

int main(int argc, char **argv) {
  auto start_time = std::chrono::system_clock::now();
  CLI::App app{
      std::string{alkaid_sd::kLogo} +
      "A program used to participate in the sdvrp track of DIMACS-12 "
      "competition\n(http://dimacs.rutgers.edu/programs/challenge/vrp/vrpsd/)"};
  std::string config_path;
  app.add_option("config", config_path, "Config path")->required();
  std::string instance_name;
  app.add_option("instance", instance_name, "Instance name")->required();
  std::string output;
  app.add_option("output", output, "Output")->required();
  std::optional<int> random_seed;
  app.add_option("-s", random_seed, "Random Seed");
  CLI11_PARSE(app, argc, argv);
  nlohmann::json default_value = {{"random_seed", std::random_device{}()}};
  nlohmann::json overwrite_value;
  if (random_seed) {
    overwrite_value["random_seed"] = random_seed.value();
  }
  auto config = alkaid_sd::ReadConfigFromFile(config_path, default_value,
                                              overwrite_value);
  auto problem_path =
      alkaid_sd::GetProblemPath(config.problem_dir, instance_name);
  auto problem = alkaid_sd::ReadProblemFromFile(problem_path);
  auto distance_matrix = alkaid_sd::CalcDistanceMatrix(problem);
  auto solution = alkaid_sd::Solve(config, problem, *distance_matrix);
  auto new_solution = *solution;
  alkaid_sd::RestoreByFloyd(*distance_matrix, *solution);
  auto end_time = std::chrono::system_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::duration<double>>(
                  end_time - start_time)
                  .count();
  std::ofstream ofs(output);
  WriteSolutionToStream(*distance_matrix, *solution, config.processor, time,
                        ofs);
  return 0;
}
