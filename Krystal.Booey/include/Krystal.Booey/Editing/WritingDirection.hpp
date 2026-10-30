#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief The direction of text an edit applies: the content's own, or left to right or right to left.
  enum class WritingDirection : uint8
  {
    Natural,
    LeftToRight,
    RightToLeft
  };
}
