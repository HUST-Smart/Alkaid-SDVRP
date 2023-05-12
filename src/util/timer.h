#ifndef ALKAID_SD_SRC_UTIL_TIMER_H_
#define ALKAID_SD_SRC_UTIL_TIMER_H_

#include <chrono>
#include <ctime>
#include <memory>

#include <nlohmann/json.hpp>

namespace alkaid_sd {

class BaseTimer {
public:
  virtual ~BaseTimer() = default;
  virtual void Set(double time_limit) = 0;
  [[nodiscard]] bool IsTimeOut() const;
  [[nodiscard]] virtual double ElapsedTime() const = 0;

protected:
  double time_limit_{};
};

class CpuTimer : public BaseTimer {
public:
  void Set(double time_limit) override;
  [[nodiscard]] double ElapsedTime() const override;

private:
  std::clock_t start_{};
};

class RealTimer : public BaseTimer {
public:
  void Set(double time_limit) override;
  [[nodiscard]] double ElapsedTime() const override;

private:
  std::chrono::high_resolution_clock::time_point start_{};
};

enum TimerType { kCpu, kReal };

NLOHMANN_JSON_SERIALIZE_ENUM(TimerType, {{kCpu, "cpu"}, {kReal, "real"}})

std::unique_ptr<BaseTimer> CreateTimer(TimerType timer_type);

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_UTIL_TIMER_H_
