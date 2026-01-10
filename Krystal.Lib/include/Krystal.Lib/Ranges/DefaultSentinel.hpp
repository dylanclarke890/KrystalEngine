#pragma once

#include <ranges>

namespace Krys::Ranges
{
  /// @brief A sentinel that cannot compare equal to any other iterator and thus results in infinitely long
  /// ranges.
  using DefaultSentinel = std::default_sentinel_t;

  /// @brief An available and usable DefaultSentinel for ease of use.
  inline constexpr DefaultSentinel default_sentinel = {};
}
