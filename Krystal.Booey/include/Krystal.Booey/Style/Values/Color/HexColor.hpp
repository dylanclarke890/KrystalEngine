#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  struct HexColor;
}

namespace krys::boo::style
{
  struct Color;
  struct ColorResolutionState;

  // NOTE: There is no style::HexColor type. Style conversion of css::HexColor yields a style::ResolvedColor.
  KRYS_NODISCARD Color ToStyleColor(const css::HexColor &unresolved, ColorResolutionState &state) noexcept;
}
