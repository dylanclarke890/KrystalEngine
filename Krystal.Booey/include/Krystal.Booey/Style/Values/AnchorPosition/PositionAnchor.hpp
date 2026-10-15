#pragma once

#include "Krystal.Booey/Style/Values/AnchorPosition/AnchorName.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'position-anchor'> = auto | <anchor-name>
  // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-anchor
  struct PositionAnchor : css::ValueOrKeyword<AnchorName, css::keywords::Auto>
  {
    using Base::Base;

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsName() const noexcept
    {
      return IsValue();
    }
    KRYS_NODISCARD Maybe<AnchorName> TryName() const noexcept
    {
      return TryValue();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PositionAnchor)
