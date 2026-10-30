#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <-webkit-line-box-contain> = none | [ block || inline || font || glyphs || replaced || inline-box ||
  // initial-letter ] NOTE: There is no standard associated with this property.

  enum class WebkitLineBoxContainValue : uint8
  {
    Block,
    Inline,
    Font,
    Glyphs,
    Replaced,
    InlineBox,
    InitialLetter,
  };

  using WebkitLineBoxContainValueEnumSet = css::SpaceSeparatedEnumSet<WebkitLineBoxContainValue>;

  struct WebkitLineBoxContain
      : css::EnumSetOrKeywordBase<WebkitLineBoxContain, WebkitLineBoxContainValueEnumSet, css::keywords::None>
  {
    using Base::Base;
    KRYS_NODISCARD constexpr bool IsNone() const noexcept
    {
      return IsKeyword();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::WebkitLineBoxContain)
