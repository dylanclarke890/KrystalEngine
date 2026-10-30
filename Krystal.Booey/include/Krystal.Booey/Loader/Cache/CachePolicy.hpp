#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  /// @brief How a load uses the memory cache.
  enum class CachePolicy : uint8
  {
    /// @brief Use cached resources unless they've expired.
    Verify,
    /// @brief Check cached resources are still valid (resources without validators reload).
    Revalidate,
    /// @brief Load every resource again.
    Reload,
    /// @brief Use cached resources even if they've expired (going back and forward).
    HistoryBuffer
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::CachePolicy, 4uz);
