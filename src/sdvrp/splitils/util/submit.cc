#include "sdvrp/splitils/util/submit.h"

#include <iostream>

namespace vrp::sdvrp::splitils {

void Submit(const BaseTimer &baseTimer, int objective) {
  std::cout << baseTimer.ElapsedTime() << ',' << objective << std::endl;
}

} // namespace vrp::sdvrp::splitils
