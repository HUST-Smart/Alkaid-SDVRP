#ifndef ALKAID_SD_SRC_UTIL_RANDOM_H_
#define ALKAID_SD_SRC_UTIL_RANDOM_H_

#include <cstdint>

namespace alkaid_sd {

class Random {
public:
  explicit Random(uint32_t seed) : s() {
    s[0] = seed;
    for (int i = 1; i < 4; ++i) {
      s[i] = static_cast<uint32_t>(ScrambleWell(s[i - 1], i));
    }
  }

  inline int NextInt(int a, int b) {
    return static_cast<int>(NextInt(b - a + 1)) + a;
  }

  inline bool NextBoolean() { return NextInt() & 1u; }

  inline float NextFloat() { return (NextInt() >> 8u) * kFloatMultiplier; }

  inline Random Jump() {
    Random random = *this;
    uint32_t s0 = 0;
    uint32_t s1 = 0;
    uint32_t s2 = 0;
    uint32_t s3 = 0;
    for (unsigned int i : kJump)
      for (uint32_t b = 0; b < 32; b++) {
        if (i & 1u << b) {
          s0 ^= s[0];
          s1 ^= s[1];
          s2 ^= s[2];
          s3 ^= s[3];
        }
        NextInt();
      }
    s[0] = s0;
    s[1] = s1;
    s[2] = s2;
    s[3] = s3;
    return random;
  }

  template <class RandomIt> void Shuffle(RandomIt first, RandomIt last) {
    typename std::iterator_traits<RandomIt>::difference_type i, n;
    n = last - first;
    for (i = n - 1; i > 0; --i) {
      std::swap(first[i], first[NextInt(0, static_cast<int>(i))]);
    }
  }

private:
  static constexpr uint32_t kJump[] = {0x8764000b, 0xf542d2d3, 0x6fa035c3,
                                       0x77f2db5b};
  static constexpr uint64_t kPow32 = uint64_t(1) << 32u;
  static constexpr float kFloatMultiplier = 0x1.0p-24f;

  uint32_t s[4];

  static inline uint32_t RotateLeft(const uint32_t x, uint32_t k) {
    return (x << k) | (x >> (32 - k));
  }

  static inline uint64_t Scramble(uint64_t n, uint64_t multiple, uint32_t shift,
                                  uint32_t add) {
    return multiple * (n ^ (n >> shift)) + add;
  }

  static inline uint64_t ScrambleWell(uint64_t n, int add) {
    return Scramble(n, 1812433253, 30, add);
  }

  inline uint32_t NextInt() {
    const uint32_t result = RotateLeft(s[0] + s[3], 7) + s[0];
    const uint32_t t = s[1] << 9u;
    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = RotateLeft(s[3], 11);
    return result;
  }

  inline uint32_t NextInt(uint32_t n) {
    uint64_t m = static_cast<uint64_t>(NextInt()) * n;
    uint64_t l = m & 0xffffffff;
    if (l < n) {
      uint64_t t = kPow32 % n;
      while (l < t) {
        m = (NextInt() & 0xffffffff) * n;
        l = m & 0xffffffff;
      }
    }
    return m >> 32u;
  }
};

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_UTIL_RANDOM_H_
