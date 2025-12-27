#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <functional>

namespace Krys
{
  struct Hash
  {
    STATIC_CLASS(Hash)

    template <class... Args>
    KRYS_NODISCARD constexpr static size_t Combine(Args... args) noexcept
    {
      size_t seed = 0u;
      Combine(seed, args...);
      return seed;
    }

    /// @brief FNV-1a 32bit hashing algorithm.
    KRYS_NODISCARD static constexpr uint32 fnv1a_32(char const *s, size_t count) noexcept
    {
      return ((count ? fnv1a_32(s, count - 1) : 2'166'136'261u) ^ s[count]) * 16'777'619u;
    }

  private:
    template <class Head, class... Tail>
    constexpr static void Combine(size_t &seed, const Head &head, Tail... tail) noexcept
    {
      // this algorithm is based on boost::combine_hash and is designed to mix/smear around values
      seed ^= std::hash<Head> {}(head) + 0x9e'37'79'b9'7f'4a'7c'55 + (seed << 6) + (seed >> 2);
      (Combine(seed, tail), ...);
    }
  };
}