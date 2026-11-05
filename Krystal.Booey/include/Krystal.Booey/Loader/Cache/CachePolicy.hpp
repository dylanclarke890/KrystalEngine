#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  /// @brief How a load uses the memory cache.
  enum class CachePolicy : uint8
  {
    /// @brief Use cached resources.
    Use,
    /// @brief Load every resource again, as reloading a page does (so edited files load again).
    Reload
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::CachePolicy, 2uz);
