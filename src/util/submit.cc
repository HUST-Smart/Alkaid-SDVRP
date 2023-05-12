#include "util/submit.h"

#include <iostream>

namespace alkaid_sd {

void Submit(const BaseTimer &baseTimer, int objective) {
  std::cout << baseTimer.ElapsedTime() << ',' << objective << std::endl;
}

} // namespace alkaid_sd
