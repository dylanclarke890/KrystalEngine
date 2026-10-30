#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'hyphenate-character'> = auto | <string>
  // https://drafts.csswg.org/css-text-4/#propdef-hyphenate-character
  struct HyphenateCharacter : css::ValueOrKeyword<css::CSSOMStringAtom, css::keywords::Auto>
  {
    using Base::Base;

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsString() const noexcept
    {
      return IsValue();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::HyphenateCharacter)
