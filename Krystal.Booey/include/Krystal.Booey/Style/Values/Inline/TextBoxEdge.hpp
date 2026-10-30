#pragma once

#include "Krystal.Booey/Style/Values/Inline/TextEdge.hpp"

namespace krys::boo::style
{
  // <'text-box-edge'> = auto | <text-edge>
  // https://drafts.csswg.org/css-inline-3/#propdef-text-box-edge
  struct TextBoxEdge : TextEdge<css::keywords::Auto>
  {
    using Base::Base;

    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextBoxEdge)
