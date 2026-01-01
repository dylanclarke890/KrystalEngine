#pragma once

#include "Krystal.Lib/Core/Config.hpp"
#if KRYS_CONFIG(STD_LIBRARY_RANGES)
  #include <ranges>
#endif

namespace Krys::Ranges
{
#if KRYS_CONFIG(STD_LIBRARY_RANGES)
  /// @brief A sentinel that cannot compare equal to any other iterator and thus results in infinitely long
  /// ranges.
  using DefaultSentinel = std::default_sentinel_t;
#else
  /// @brief A sentinel that cannot compare equal to any other iterator and thus results in infinitely long
  /// ranges.
  struct DefaultSentinel
  {
  };
#endif

  /// @brief An available and usable DefaultSentinel for ease of use.
  inline constexpr DefaultSentinel default_sentinel = {};
}
