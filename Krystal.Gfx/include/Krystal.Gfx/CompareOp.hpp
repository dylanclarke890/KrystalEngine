#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::gfx
{
  /// @brief How a new value is compared with an existing one, in depth and stencil tests and comparison
  /// samplers. The test passes when `new <op> existing` holds.
  enum class CompareOp : uint8
  {
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always,
  };
}
