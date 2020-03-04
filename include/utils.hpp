#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <cstdint>

namespace PiBench
{
namespace utils
{
    // Starting from 10
    static constexpr uint64_t POWER_TO_G[] = {29, 29, 53, 53, 117, 125, 229, 221,
    469, 517, 589, 861, 1189, 1653, 2333, 3381, 4629, 6565, 9293, 13093, 18509,
    26253, 37117, 52317, 74101, 104581, 147973, 209173, 296029, 418341, 91733,
    836661, 1183221, 1673485, 2366509, 3346853, 4732789, 6693237, 9465541,
    13386341, 18931141, 26772693, 37862197, 53545221, 75724373, 107090317};

    /**
     * @brief Calculates the discrete logarithmic of integer k.
     *
     * This function can be used to shuffle integers in a range without
     * collisions. The implementation is derived from:
     * "Quickly Generating Billion-Record Synthetic Databases", Jim Gray et al,
     * SIGMOD 19
     *
     * @tparam POWER_OF_TWO The nterval covered by the function is [1, 2^POWER_OF_TWO].
     * @param k
     * @return uint64_t
     */
    template<uint32_t POWER_OF_TWO>
    static uint64_t discrete_log(const uint64_t k) noexcept
    {
        static constexpr uint32_t POWER = POWER_OF_TWO + 2;
        static_assert(POWER >=10 && POWER <= 55, "POWER must be in range [10,55].");
        static constexpr uint64_t G = POWER_TO_G[POWER-10];
        static constexpr uint64_t P = 1ULL << (POWER-2);
        static constexpr uint64_t P_MASK = P-1;

        //assert(k > 0);
        assert(k < (1ULL << (POWER-2)));

        uint64_t up = k;
        uint64_t x = 0;
        uint64_t radix = 1;
        uint64_t Gpow = (G-1)/4;
        for(uint64_t i=0; up; ++i)
        {
            if(up & radix)
            {
                x = x + radix;
                up = (up + Gpow + 4 * up * Gpow) & P_MASK;
            }
            radix = radix << 1;
            Gpow = (Gpow + Gpow + 4 * Gpow * Gpow) & P_MASK;
        }
        return P-x;
    }

    template<typename T>
    static T fnv1a(const void* data, size_t size)
    {
        static_assert(
            std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value,
            "fnv1a only supports 32 bits and 64 bits variants."
        );

        static constexpr T INIT = std::is_same<T, uint32_t>::value ? 2166136261u : 14695981039346656037ul;
        static constexpr T PRIME = std::is_same<T, uint32_t>::value ? 16777619 : 1099511628211ul;

        auto src = reinterpret_cast<const char*>(data);
        T sum = INIT;
        while (size--)
        {
            sum = (sum ^ *src) * PRIME;
            src++;
        }
        return sum;
    }

    /**
     * @brief Calculate multiplicative hash of integer in the same domain.
     *
     * This was adapted from:
     *
     * ""The Art of Computer Programming, Volume 3, Sorting and Searching",
     * D.E. Knuth, 6.4 p:516"
     *
     * The function should be used as a cheap way to scramble integers in the
     * domain [0,2^T] to integers in the same domain [0,2^T]. If the resulting
     * hash should be in a smaller domain [0,2^m], the result should be right
     * shifted by X bits where X = (32 | 64) - m.
     *
     * @tparam T type of input and output (uint32_t or uint64_t).
     * @param x integer to be hashed.
     * @return T hashed result.
     */
    template<typename T>
    static T multiplicative_hash(T x)
    {
        static_assert(
            std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value,
            "fnv1a only supports 32 bits and 64 bits variants."
        );

        static constexpr T A = std::is_same<T, uint32_t>::value ?
            2654435761u
            :
            11400714819323198393ul;
        return A * x;
    }

    /**
     * @brief Verify endianess during runtime.
     *
     * @return true
     * @return false
     */
    static bool is_big_endian(void)
    {
        volatile union {
            uint32_t i;
            char c[4];
        } bint = {0x01020304};

        return bint.c[0] == 1;
    }

    /**
     * @brief Read memory without optimizing out.
     *
     * This function should be called to simulate a dummy use of the variable p.
     * It tells the compiler that there might be side effects and avoids any
     * optimization-out.
     *
     * Source: https://youtu.be/nXaxk27zwlk?t=2550
     *
     * @param addr beginning of memory region.
     * @param size size of memory region.
     */
    static void dummy_use(void* addr, size_t size)
    {
        char* p = static_cast<char*>(p);
        char* end = static_cast<char*>(p) + size;

        if(p >= end)
            return;

        for(; p<end; ++p)
        {
            void* vptr = static_cast<void*>(p);
            asm volatile("" : : "g"(vptr) : "memory");
        }
    }

    static const char gDigitsLut[200] = {
        '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6',
        '0', '7', '0', '8', '0', '9', '1', '0', '1', '1', '1', '2', '1', '3',
        '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0',
        '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7',
        '2', '8', '2', '9', '3', '0', '3', '1', '3', '2', '3', '3', '3', '4',
        '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', '4', '0', '4', '1',
        '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8',
        '4', '9', '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5',
        '5', '6', '5', '7', '5', '8', '5', '9', '6', '0', '6', '1', '6', '2',
        '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
        '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6',
        '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8', '2', '8', '3',
        '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', '9', '0',
        '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7',
        '9', '8', '9', '9'};

#define BEGIN2(n)               \
  do {                          \
    int t = (n);                \
    if (t < 10)                 \
      *p++ = '0' + t;           \
    else {                      \
      t *= 2;                   \
      *p++ = gDigitsLut[t];     \
      *p++ = gDigitsLut[t + 1]; \
    }                           \
  } while (0)
#define MIDDLE2(n)            \
  do {                        \
    int t = (n)*2;            \
    *p++ = gDigitsLut[t];     \
    *p++ = gDigitsLut[t + 1]; \
  } while (0)

#define BEGIN4(n)        \
  do {                   \
    int t4 = (n);        \
    if (t4 < 100)        \
      BEGIN2(t4);        \
    else {               \
      BEGIN2(t4 / 100);  \
      MIDDLE2(t4 % 100); \
    }                    \
  } while (0)
#define MIDDLE4(n)     \
  do {                 \
    int t4 = (n);      \
    MIDDLE2(t4 / 100); \
    MIDDLE2(t4 % 100); \
  } while (0)

#define BEGIN8(n)          \
  do {                     \
    uint32_t t8 = (n);     \
    if (t8 < 10000)        \
      BEGIN4(t8);          \
    else {                 \
      BEGIN4(t8 / 10000);  \
      MIDDLE4(t8 % 10000); \
    }                      \
  } while (0)
#define MIDDLE8(n)       \
  do {                   \
    uint32_t t8 = (n);   \
    MIDDLE4(t8 / 10000); \
    MIDDLE4(t8 % 10000); \
  } while (0)
#define MIDDLE16(n)           \
  do {                        \
    uint64_t t16 = (n);       \
    MIDDLE8(t16 / 100000000); \
    MIDDLE8(t16 % 100000000); \
  } while (0)


    static void u32toa_lut(uint32_t x, char* p) {
      if (x < 100000000)
        BEGIN8(x);
      else {
        BEGIN2(x / 100000000);
        MIDDLE8(x % 100000000);
      }
      *p = 0;
    }

    }  // namespace utils
} // namespace PiBench
#endif