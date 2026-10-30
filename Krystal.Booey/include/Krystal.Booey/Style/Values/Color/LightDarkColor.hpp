#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  struct LightDarkColor;
}

namespace krys::boo::style
{
  struct Color;
  struct ColorResolutionState;

  // NOTE: There is no style::LightDarkColor type. Style conversion of css::LightDarkColor yields a
  // style::ResolvedColor.
  KRYS_NODISCARD Color ToStyleColor(const css::LightDarkColor &unresolved,
                                    ColorResolutionState &state) noexcept;
}
