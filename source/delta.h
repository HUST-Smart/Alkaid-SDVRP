#pragma once

#include "random.h"

namespace alkaidsd {
  template <class T> struct Delta {
    T value;
    int counter;

    Delta() : value(T()), counter(-1) {}

    Delta(T value, int counter) : value(value), counter(counter) {}

    bool Update(T new_value, Random &random) {
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

    bool Update(const Delta<T> &delta, Random &random) {
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
}  // namespace alkaidsd
