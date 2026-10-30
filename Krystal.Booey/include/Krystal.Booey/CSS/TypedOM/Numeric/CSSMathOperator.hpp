#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css
{
  /// @brief The operation a math value performs.
  /// @see https://drafts.css-houdini.org/css-typed-om/#enumdef-cssmathoperator
  enum class CSSMathOperator : uint8
  {
    Sum,
    Product,
    Negate,
    Invert,
    Min,
    Max,
    Clamp
  };
}
