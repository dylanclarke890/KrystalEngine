#pragma once

#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"

namespace krys::boo::style
{
  // <'orphans'> = <integer [1,∞]>
  // https://drafts.csswg.org/css-break/#propdef-orphans
  // FIXME: `orphans` should not support `auto`. It doesn't allow parsing `auto`.
  struct Orphans : css::ValueOrKeyword<Integer<css::Range {1, css::Range::Inf}, uint16>, css::keywords::Auto>
  {
    using Base::Base;
    using Integer = typename Base::Value;

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsInteger() const noexcept
    {
      return IsValue();
    }
    KRYS_NODISCARD Maybe<Integer> TryInteger() const noexcept
    {
      return TryValue();
    }
  };

  // MARK: - Blending

  template <>
  struct Blending<Orphans>
  {
    KRYS_NODISCARD Orphans Blend(const Orphans &, const Orphans &, const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Orphans)
