#include "util/timer.h"

namespace alkaid_sd {

bool BaseTimer::IsTimeOut() const { return ElapsedTime() > time_limit_; }

void CpuTimer::Set(double time_limit) {
  time_limit_ = time_limit;
  start_ = std::clock();
}

double CpuTimer::ElapsedTime() const {
  return static_cast<double>(std::clock() - start_) / CLOCKS_PER_SEC;
}

void RealTimer::Set(double time_limit) {
  time_limit_ = time_limit;
  start_ = std::chrono::high_resolution_clock::now();
}

double RealTimer::ElapsedTime() const {
  return std::chrono::duration_cast<std::chrono::duration<double>>(
             std::chrono::high_resolution_clock::now() - start_)
      .count();
}

std::unique_ptr<BaseTimer> CreateTimer(TimerType timer_type) {
  if (timer_type == kCpu) {
    return std::make_unique<CpuTimer>();
  } else {
    return std::make_unique<RealTimer>();
  }
}

} // namespace alkaid_sd
