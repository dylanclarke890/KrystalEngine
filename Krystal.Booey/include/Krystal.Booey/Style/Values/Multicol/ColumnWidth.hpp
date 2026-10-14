#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <'column-width'> = auto | <length [0,∞]>
  // https://www.w3.org/TR/css-multicol-1/#propdef-column-width
  struct ColumnWidth : css::ValueOrKeyword<Length<css::NonNegative, float>, css::keywords::Auto>
  {
    using Base::Base;
    using Length = typename Base::Value;

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return IsValue();
    }
    KRYS_NODISCARD Maybe<Length> TryLength() const noexcept
    {
      return TryValue();
    }
  };

  // MARK: - Blending

  template <>
  struct Blending<ColumnWidth>
  {
    KRYS_NODISCARD bool CanBlend(const ColumnWidth &, const ColumnWidth &) noexcept;
    KRYS_NODISCARD ColumnWidth Blend(const ColumnWidth &, const ColumnWidth &,
                                     const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ColumnWidth)
