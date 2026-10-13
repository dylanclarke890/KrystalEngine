#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"

namespace krys::boo::style
{
  /// @brief Resolves a system colour keyword (e.g. `Canvas`, `ButtonText`, the deprecated system colours) to
  /// the default theme's colour.
  /// @return The colour, or the invalid colour if `keyword` isn't a system colour.
  KRYS_NODISCARD boo::Color SystemColor(css::ValueId keyword, ColorOptions options) noexcept;
}
