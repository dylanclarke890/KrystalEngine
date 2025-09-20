#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include <functional>

namespace Krys
{
  namespace Impl
  {
    template <class Head, class... Tail>
    static void HashCombine(size_t &seed, const Head &head, Tail... tail)
    {
      // this algorithm is based on boost::combine_hash and is designed to mix/smear around values
      seed ^= std::hash<Head> {}(head) + 0x9e'37'79'b9'7f'4a'7c'55 + (seed << 6) + (seed >> 2);
      (HashCombine(seed, tail), ...);
    }
  }

  struct HashUtils
  {
    STATIC_CLASS(HashUtils)

    template <class... Args>
    static size_t HashCombine(Args... args)
    {
      size_t seed = 0u;
      Impl::HashCombine(seed, args...);
      return seed;
    }

    /// @brief FNV-1a 32bit hashing algorithm.
    NO_DISCARD static constexpr uint32 fnv1a_32(char const *s, size_t count) noexcept
    {
      return ((count ? fnv1a_32(s, count - 1) : 2'166'136'261u) ^ s[count]) * 16'777'619u;
    }
  };
}