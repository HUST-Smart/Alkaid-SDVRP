#ifndef ALKAID_SD_SRC_UTIL_DELTA_H_
#define ALKAID_SD_SRC_UTIL_DELTA_H_

#include "util/random.h"

namespace alkaid_sd {

template <class T> struct Delta {
  T value;
  int counter;

  inline Delta() : value(T()), counter(-1) {}

  inline Delta(T value, int counter) : value(value), counter(counter) {}

  inline bool Update(T new_value, Random &random) {
    if (new_value < value) {
      value = new_value;
      counter = 1;
      return true;
    } else if (new_value == value && counter != -1) {
      ++counter;
      return random.NextInt(1, counter) == 1;
    }
    return false;
  }

  inline bool Update(const Delta<T> &delta, Random &random) {
    if (delta.value < value) {
      value = delta.value;
      counter = delta.counter;
      return true;
    } else if (delta.value == value && counter != -1) {
      counter += delta.counter;
      return random.NextInt(1, counter) <= delta.counter;
    }
    return false;
  }
};

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_UTIL_DELTA_H_
