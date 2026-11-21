#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <rapidhash.h>

namespace krys
{
  /// @brief A hash algorithm: what `FastHash` (and later `StableHash`) build their hashes from. A backend
  /// that misses a function fails to compile where it's named.
  template <typename T>
  concept HashAlgorithm = requires(Span<const byte> bytes, uint64 value, uint64 seed) {
    // Hashes a run of bytes (strings, buffers).
    { T::HashBytes(bytes, seed) } noexcept -> SameType<uint64>;
    // Mixes one integer (integers, enumerations, pointers) into a well-distributed hash.
    { T::MixInteger(value) } noexcept -> SameType<uint64>;
    // Folds another hash into a running one.
    { T::Combine(seed, value) } noexcept -> SameType<uint64>;
  };

  /// @brief rapidhash: a fast, high-quality hash with good distribution on the short keys hash tables see.
  /// @see https://github.com/Nicoshev/rapidhash
  struct RapidHashAlgorithm
  {
    KRYS_NODISCARD KRYS_ALWAYS_INLINE static uint64 HashBytes(Span<const byte> bytes,
                                                              uint64 seed = 0) noexcept
    {
      return rapidhash_withSeed(bytes.data(), bytes.size(), seed);
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE static uint64 MixInteger(uint64 value) noexcept
    {
      return rapid_mix(value ^ rapid_secret[0], rapid_secret[1]);
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE static uint64 Combine(uint64 seed, uint64 value) noexcept
    {
      return rapid_mix(seed ^ rapid_secret[2], value ^ rapid_secret[1]);
    }
  };

  static_assert(HashAlgorithm<RapidHashAlgorithm>);

  /// @brief The algorithm behind `FastHash`. Change it here to change every in-memory hash in the engine; its
  /// results are never stored, so nothing else needs to change with it.
  using FastHashAlgorithm = RapidHashAlgorithm;
}
