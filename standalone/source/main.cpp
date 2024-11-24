#include <alkaidsd/distance_matrix_optimizer.h>
#include <alkaidsd/solver.h>

#include <CLI/CLI.hpp>
#include <chrono>
#include <random>

std::vector<std::unique_ptr<alkaidsd::inter_operator::InterOperator>> ParseInterOperators(
    const std::vector<std::string> &args);
std::vector<std::unique_ptr<alkaidsd::intra_operator::IntraOperator>> ParseIntraOperators(
    const std::vector<std::string> &args);
std::function<std::unique_ptr<alkaidsd::acceptance_rule::AcceptanceRule>()> ParseAcceptanceRule(
    const std::string &type, const std::vector<std::string> &args);
std::unique_ptr<alkaidsd::ruin_method::RuinMethod> ParseRuinMethod(
    const std::string &type, const std::vector<std::string> &args);
alkaidsd::sorter::Sorter ParseSorter(const std::vector<std::string> &args);
enum InputFormat { COORD_LIST, DENSE_MATRIX };
alkaidsd::Instance ReadInstanceFromFile(const std::string &instance_path,
                                      InputFormat format = COORD_LIST);
std::map<std::string, double> ParseFromArgs(const std::vector<std::string> &args);

class SimpleListener : public alkaidsd::Listener {
public:
  void OnStart() override { start_time_ = std::chrono::system_clock::now(); }
  void OnUpdated([[maybe_unused]] const alkaidsd::AlkaidSolution &solution, int objective) override {
    auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::system_clock::now() - start_time_);
    std::cout << "Update at " << elapsed_time.count() << "s: " << objective << std::endl;
  }
  void OnEnd([[maybe_unused]] const alkaidsd::AlkaidSolution &solution, int objective) override {
    auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::system_clock::now() - start_time_);
    std::cout << "End at " << elapsed_time.count() << "s: " << objective << std::endl;
  }

private:
  std::chrono::system_clock::time_point start_time_;
};

int main(int argc, char **argv) {
  CLI::App app;
  std::string instance_path;
  alkaidsd::AlkaidConfig config;
  InputFormat input_format;
  app.add_option("--input", instance_path, "SDVRP problem instance file path")
      ->required()
      ->check(CLI::ExistingFile);
  std::string output;
  app.add_option("--output", output, "SDVRP solution file path")->required();
  app.set_config("--config")->check(CLI::ExistingFile);
  app.add_option("--input-format", input_format,
                 "Use coordinate list (0) or cost matrix (1) as input format")
      ->default_val(InputFormat::COORD_LIST);
  app.add_option("--random-seed", config.random_seed, "Random seed")
      ->default_val(std::random_device{}());
  app.add_option("--time-limit", config.time_limit, "Time limit")->required();
  app.add_option("--blink-rate", config.blink_rate, "Blink rate")->required();
  std::vector<std::string> inter_operators;
  app.add_option("--inter-operators", inter_operators, "Inter operators")->required();
  std::vector<std::string> intra_operators;
  app.add_option("--intra-operators", intra_operators, "Intra operators")->required();
  std::string acceptance_rule_type;
  app.add_option("--acceptance-rule-type", acceptance_rule_type, "Acceptance rule type")
      ->required();
  std::vector<std::string> acceptance_rule_args;
  app.add_option("--acceptance-rule-args", acceptance_rule_args, "Acceptance rule args");
  std::string ruin_method_type;
  app.add_option("--ruin-method-type", ruin_method_type, "Ruin method type")->required();
  std::vector<std::string> ruin_method_args;
  app.add_option("--ruin-method-args", ruin_method_args, "Ruin method args");
  std::vector<std::string> sorters;
  app.add_option("--sorters", sorters, "Sorters")->required();
  CLI11_PARSE(app, argc, argv);
  config.inter_operators = ParseInterOperators(inter_operators);
  config.intra_operators = ParseIntraOperators(intra_operators);
  config.acceptance_rule = ParseAcceptanceRule(acceptance_rule_type, acceptance_rule_args);
  config.ruin_method = ParseRuinMethod(ruin_method_type, ruin_method_args);
  config.sorter = ParseSorter(sorters);
  config.listener = std::make_unique<SimpleListener>();
  auto instance = ReadInstanceFromFile(instance_path, input_format);
  auto distance_matrix_optimizer = alkaidsd::DistanceMatrixOptimizer(instance.distance_matrix);
  alkaidsd::AlkaidSolver solver;
  auto solution = solver.Solve(config, instance);
  distance_matrix_optimizer.Restore(solution);
  std::ofstream ofs(output);
  std::string json_ext(".json");
  if (output.substr(output.size() - json_ext.size()) == json_ext) {
    solution.PrintJson(ofs);
  } else {
    ofs << solution;
  }
  return 0;
}

std::vector<std::unique_ptr<alkaidsd::inter_operator::InterOperator>> ParseInterOperators(
    const std::vector<std::string> &args) {
  std::vector<std::unique_ptr<alkaidsd::inter_operator::InterOperator>> inter_operators;
  for (const auto &arg : args) {
    if (arg == "Swap<2, 0>") {
      inter_operators.push_back(std::make_unique<alkaidsd::inter_operator::Swap<2, 0>>());
    } else if (arg == "Swap<2, 1>") {
      inter_operators.push_back(std::make_unique<alkaidsd::inter_operator::Swap<2, 1>>());
    } else if (arg == "Swap<2, 2>") {
      inter_operators.push_back(std::make_unique<alkaidsd::inter_operator::Swap<2, 2>>());
    } else if (arg == "Relocate") {
      inter_operators.push_back(std::make_unique<alkaidsd::inter_operator::Relocate>());
    } else if (arg == "SwapStar") {
      inter_operators.push_back(std::make_unique<alkaidsd::inter_operator::SwapStar>());
    } else if (arg == "Cross") {
      inter_operators.push_back(std::make_unique<alkaidsd::inter_operator::Cross>());
    } else if (arg == "SdSwapStar") {
      inter_operators.push_back(std::make_unique<alkaidsd::inter_operator::SdSwapStar>());
    } else if (arg == "SdSwapOneOne") {
      inter_operators.push_back(std::make_unique<alkaidsd::inter_operator::SdSwapOneOne>());
    } else if (arg == "SdSwapTwoOne") {
      inter_operators.push_back(std::make_unique<alkaidsd::inter_operator::SdSwapTwoOne>());
    } else {
      throw std::invalid_argument("Invalid inter operator.");
    }
  }
  return inter_operators;
}

std::vector<std::unique_ptr<alkaidsd::intra_operator::IntraOperator>> ParseIntraOperators(
    const std::vector<std::string> &args) {
  std::vector<std::unique_ptr<alkaidsd::intra_operator::IntraOperator>> intra_operators;
  for (const auto &arg : args) {
    if (arg == "Exchange") {
      intra_operators.push_back(std::make_unique<alkaidsd::intra_operator::Exchange>());
    } else if (arg == "OrOpt<1>") {
      intra_operators.push_back(std::make_unique<alkaidsd::intra_operator::OrOpt<1>>());
    } else if (arg == "OrOpt<2>") {
      intra_operators.push_back(std::make_unique<alkaidsd::intra_operator::OrOpt<2>>());
    } else if (arg == "OrOpt<3>") {
      intra_operators.push_back(std::make_unique<alkaidsd::intra_operator::OrOpt<3>>());
    } else {
      throw std::invalid_argument("Invalid intra operator.");
    }
  }
  return intra_operators;
}

std::function<std::unique_ptr<alkaidsd::acceptance_rule::AcceptanceRule>()> ParseAcceptanceRule(
    const std::string &type, const std::vector<std::string> &args) {
  auto map = ParseFromArgs(args);
  if (type == "LAHC") {
    auto length = static_cast<int>(map.at("length"));
    return [length]() {
      return std::make_unique<alkaidsd::acceptance_rule::LateAcceptanceHillClimbing>(length);
    };
  } else if (type == "SA") {
    auto initial_temperature = map.at("initial_temperature");
    auto decay = map.at("decay");
    return [initial_temperature, decay]() {
      return std::make_unique<alkaidsd::acceptance_rule::SimulatedAnnealing>(initial_temperature,
                                                                             decay);
    };
  } else if (type == "HCWE") {
    return []() { return std::make_unique<alkaidsd::acceptance_rule::HillClimbingWithEqual>(); };
  } else {
    return []() { return std::make_unique<alkaidsd::acceptance_rule::HillClimbing>(); };
  }
}

std::unique_ptr<alkaidsd::ruin_method::RuinMethod> ParseRuinMethod(
    const std::string &type, const std::vector<std::string> &args) {
  if (type == "SISRs") {
    auto map = ParseFromArgs(args);
    auto average_customers = static_cast<int>(map.at("average_customers"));
    auto max_length = static_cast<int>(map.at("max_length"));
    auto split_rate = map.at("split_rate");
    auto preserved_probability = map.at("preserved_probability");
    return std::make_unique<alkaidsd::ruin_method::SisrsRuin>(average_customers, max_length,
                                                              split_rate, preserved_probability);
  } else {
    auto num_perturb_customers = std::vector<int>{};
    for (const auto &arg : args) {
      num_perturb_customers.push_back(std::stoi(arg));
    }
    return std::make_unique<alkaidsd::ruin_method::RandomRuin>(num_perturb_customers);
  }
}

alkaidsd::sorter::Sorter ParseSorter(const std::vector<std::string> &args) {
  auto map = ParseFromArgs(args);
  alkaidsd::sorter::Sorter sorter;
  for (const auto &[key, value] : map) {
    if (key == "random") {
      sorter.AddSortFunction(std::make_unique<alkaidsd::sorter::SortByRandom>(), value);
    } else if (key == "demand") {
      sorter.AddSortFunction(std::make_unique<alkaidsd::sorter::SortByDemand>(), value);
    } else if (key == "far") {
      sorter.AddSortFunction(std::make_unique<alkaidsd::sorter::SortByFar>(), value);
    } else if (key == "close") {
      sorter.AddSortFunction(std::make_unique<alkaidsd::sorter::SortByClose>(), value);
    } else {
      throw std::invalid_argument("Invalid sort function.");
    }
  }
  return sorter;
}

std::map<std::string, double> ParseFromArgs(const std::vector<std::string> &args) {
  std::map<std::string, double> ret;
  for (const auto &arg : args) {
    auto pos = arg.find('=');
    if (pos == std::string::npos) {
      throw std::invalid_argument("Invalid argument.");
    }
    auto key = arg.substr(0, pos);
    auto value = std::stod(arg.substr(pos + 1));
    ret[key] = value;
  }
  return ret;
}

alkaidsd::Instance ReadInstanceFromFile(const std::string &instance_path, InputFormat format) {
  std::ifstream ifs(instance_path);
  if (ifs.fail()) {
    throw std::invalid_argument("Cannot open instance.");
  }
  alkaidsd::Instance instance{};
  ifs >> instance.num_customers >> instance.capacity;
  ++instance.num_customers;
  instance.demands.resize(instance.num_customers);
  for (alkaidsd::Node i = 1; i < instance.num_customers; ++i) {
    ifs >> instance.demands[i];
  }

  instance.distance_matrix.resize(instance.num_customers);
  if (format == InputFormat::DENSE_MATRIX) {
    for (alkaidsd::Node i = 0; i < instance.num_customers; ++i) {
      instance.distance_matrix[i].resize(instance.num_customers);
      for (alkaidsd::Node j = 0; j < instance.num_customers; ++j) {
        ifs >> instance.distance_matrix[i][j];
      }
    }
  } else {
    std::vector<std::pair<int, int>> customers(instance.num_customers);
    for (alkaidsd::Node i = 0; i < instance.num_customers; ++i) {
      ifs >> customers[i].first >> customers[i].second;
    }
    for (alkaidsd::Node i = 0; i < instance.num_customers; ++i) {
      instance.distance_matrix[i].resize(instance.num_customers);
      for (alkaidsd::Node j = 0; j < instance.num_customers; ++j) {
        auto [x1, y1] = customers[i];
        auto [x2, y2] = customers[j];
        instance.distance_matrix[i][j] = lround(hypot(x1 - x2, y1 - y2));
      }
    }
  }
  return instance;
}
