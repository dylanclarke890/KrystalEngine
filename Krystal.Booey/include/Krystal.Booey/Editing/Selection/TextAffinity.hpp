#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief Which side of a line break a position at it belongs to.
  /// Upstream means before a line break, downstream means after a line break.
  enum class Affinity : bool
  {
    Upstream,
    Downstream
  };
}
