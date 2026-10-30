#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  enum class FontBaseline : uint8
  {
    Alphabetic,
    Ideographic,
    Central
  };

  /// @brief The edge of a box a baseline is synthesised from, when the box has none.
  enum class BaselineSynthesisEdge : uint8
  {
    ContentBox,
    BorderBox,
    MarginBox
  };
}
