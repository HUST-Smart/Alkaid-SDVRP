#include "sdvrp/visualization.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include "sdvrp/splitils/util/solution_utils.h"

namespace vrp::sdvrp {

std::string HslToRgb(double hue, double saturation, double lightness) {
  double chroma = (1 - std::abs(2 * lightness - 1)) * saturation;
  hue /= 60;
  double second_component = chroma * (1 - std::abs(std::fmod(hue, 2) - 1));
  double red = 0, green = 0, blue = 0;
  if (hue < 1) {
    red = chroma;
    green = second_component;
  } else if (hue < 2) {
    green = chroma;
    red = second_component;
  } else if (hue < 3) {
    green = chroma;
    blue = second_component;
  } else if (hue < 4) {
    blue = chroma;
    green = second_component;
  } else if (hue < 5) {
    blue = chroma;
    red = second_component;
  } else {
    red = chroma;
    blue = second_component;
  }
  double match_lightness = lightness - chroma / 2;
  red += match_lightness;
  green += match_lightness;
  blue += match_lightness;
  std::stringstream ss;
  ss << "#";
  ss << std::hex << std::setfill('0') << std::setw(6)
     << (static_cast<int>(red * 255) << 16 |
         static_cast<int>(green * 255) << 8 | static_cast<int>(blue * 255));
  return ss.str();
}

std::string SolutionToGraph(const Problem &problem,
                            const DistanceMatrix &distance_matrix,
                            const splitils::Solution &solution, float scale,
                            bool load, bool open) {
  std::stringstream ss;
  ss << "digraph{\n";
  ss << "layout=neato\n";
  ss << "splines=curved\n";
  ss << "node[margin=0shape=circle]\n";
  ss << "scale=" << scale << '\n';
  std::vector<Node> route_heads;
  for (Node i = 0; i < solution.num_nodes(); ++i) {
    Node node_index = solution.node_pool()[i];
    if (!solution.predecessor(node_index)) {
      route_heads.emplace_back(node_index);
    }
  }
  ss << "label=\"num_route=" << route_heads.size()
     << " total_distance=" << CalcObjective(solution, distance_matrix)
     << "\"\n";
  std::vector<std::vector<std::pair<int, std::string>>> color_labels(
      problem.num_customers);
  double hue_base = 360.0 / route_heads.size();
  for (Node i = 0; i < static_cast<Node>(route_heads.size()); ++i) {
    Node predecessor = 0;
    Node successor = route_heads[i];
    std::string color = "color=\"" + HslToRgb(i * hue_base, 1, 0.4) + "\"";
    while (true) {
      if (!open || (predecessor && successor)) {
        ss << solution.customer(predecessor) << "->"
           << solution.customer(successor) << "[" << color << "]\n";
      }
      if (!successor) {
        break;
      }
      color_labels[solution.customer(successor)].emplace_back(
          solution.load(successor), color);
      predecessor = successor;
      successor = solution.successor(successor);
    }
  }
  for (Node i = 0; i < problem.num_customers; ++i) {
    auto customer = problem.customers[i];
    ss << i << "[pos=\"" << customer.x << "," << customer.y << "!\"";
    if (load && !color_labels[i].empty()) {
      if (color_labels[i].size() == 1) {
        ss << "font" << color_labels[i][0].second
           << "xlabel=" << color_labels[i][0].first;
      } else {
        ss << "xlabel=<";
        bool first = true;
        for (auto &&kv : color_labels[i]) {
          if (!first) {
            ss << "<br/>";
          }
          first = false;
          ss << "<font " << kv.second << ">" << kv.first << "</font>";
        }
        ss << '>';
      }
    }
    ss << "]\n";
  }
  ss << '}';
  return ss.str();
}

std::string UrlHashEncode(const std::string &value) {
  std::stringstream escaped;
  escaped.fill('0');
  escaped << std::hex;
  for (char c : value) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' ||
        c == ',' || c == '=' || c == '/' || c == '!') {
      escaped << c;
      continue;
    }
    escaped << std::uppercase;
    escaped << '%' << std::setw(2) << int((unsigned char)c);
    escaped << std::nouppercase;
  }
  return escaped.str();
}

void ShowGraphUrl(const std::string &graph) {
  std::cerr << "https://edotor.net/?engine=dot#" + UrlHashEncode(graph)
            << std::endl;
}

} // namespace vrp::sdvrp
