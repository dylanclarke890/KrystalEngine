#pragma once

#include "Krystal.Booey/Style/Values/Inline/TextEdge.hpp"

namespace krys::boo::style
{
  // <'line-fit-edge'> = leading | <text-edge>
  // https://drafts.csswg.org/css-inline-3/#propdef-line-fit-edge
  struct LineFitEdge : TextEdge<css::keywords::Leading>
  {
    using Base::Base;

    KRYS_NODISCARD constexpr bool IsLeading() const noexcept
    {
      return IsKeyword();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::LineFitEdge)
