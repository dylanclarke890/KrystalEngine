#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"

namespace krys::boo::css::tests
{
  /// @brief The colour named by the keyword @p id.
  KRYS_NODISCARD inline Color ColorKeyword(ValueId id)
  {
    return Color {KeywordColor {id}};
  }
}
