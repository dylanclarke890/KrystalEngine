#pragma once

#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"

namespace krys::boo::style
{
  // <'math-depth'> = auto-add | add(<integer>) | <integer>
  // https://w3c.github.io/mathml-core/#propdef-math-depth
  struct MathDepth : Integer<>
  {
    using Base = Integer<>;
    using Base::Base;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<MathDepth>
  {
    KRYS_NODISCARD MathDepth operator()(BuilderState &, const css::Value &) noexcept;
  };
}
