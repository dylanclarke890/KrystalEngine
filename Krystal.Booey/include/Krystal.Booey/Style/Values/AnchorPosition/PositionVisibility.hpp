#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'position-visibility'> = always | [ anchors-valid || anchors-visible || no-overflow ]
  // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-visibility

  enum class PositionVisibilityValue : uint8
  {
    AnchorsValid,
    AnchorsVisible,
    NoOverflow,
  };

  using PositionVisibilityValueEnumSet = css::SpaceSeparatedEnumSet<PositionVisibilityValue>;

  struct PositionVisibility
      : css::EnumSetOrKeywordBase<PositionVisibility, PositionVisibilityValueEnumSet, css::keywords::Always>
  {
    using Base::Base;
    KRYS_NODISCARD constexpr bool IsAlways() const noexcept
    {
      return IsKeyword();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PositionVisibility)
