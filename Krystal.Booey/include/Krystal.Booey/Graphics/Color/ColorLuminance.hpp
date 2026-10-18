#pragma once

#include "Krystal.Core/Colour/ColourLuminance.hpp"

namespace krys::boo
{
  class Color;

  using krys::ContrastRatio;
  using krys::RelativeLuminance;

  KRYS_NODISCARD double RelativeLuminance(const Color &) noexcept;
  KRYS_NODISCARD double ContrastRatio(const Color &, const Color &) noexcept;
}
