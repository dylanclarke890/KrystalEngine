#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  /// @brief Where a document is in entering the back/forward cache.
  enum class BackForwardCacheState : uint8
  {
    NotInBackForwardCache,
    /// @brief Its page is leaving it: `pagehide` fires before it's cached.
    AboutToEnterBackForwardCache,
    InBackForwardCache
  };
}
