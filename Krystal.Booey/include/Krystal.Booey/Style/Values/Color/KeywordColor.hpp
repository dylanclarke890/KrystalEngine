#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  struct KeywordColor;
}

namespace krys::boo::style
{
  struct Color;
  struct ColorResolutionState;

  // NOTE: There is no style::KeywordColor type. Style conversion of css::KeywordColor yields a
  // style::CurrentColor or a style::ResolvedColor.
  KRYS_NODISCARD Color ToStyleColor(const css::KeywordColor &unresolved,
                                    ColorResolutionState &state) noexcept;
}
