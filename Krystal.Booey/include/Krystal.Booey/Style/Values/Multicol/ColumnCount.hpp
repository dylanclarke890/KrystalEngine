#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <'column-width'> = auto | <integer [1,∞]>
  // https://www.w3.org/TR/css-multicol-1/#propdef-column-count
  struct ColumnCount
      : css::ValueOrKeyword<Integer<css::Range {1, css::Range::Inf}, uint16>, css::keywords::Auto>
  {
    using Base::Base;

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
  };

  // MARK: - Blending

  template <>
  struct Blending<ColumnCount>
  {
    KRYS_NODISCARD bool CanBlend(const ColumnCount &, const ColumnCount &) noexcept;
    KRYS_NODISCARD ColumnCount Blend(const ColumnCount &, const ColumnCount &,
                                     const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ColumnCount)
