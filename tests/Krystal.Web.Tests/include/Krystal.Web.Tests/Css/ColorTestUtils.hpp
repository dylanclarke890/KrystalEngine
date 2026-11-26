#pragma once

#include "Krystal.Web/Css/Values/Color/Color.hpp"

namespace krys::web::css::tests
{
  /// @brief The colour named by the keyword @p id.
  KRYS_NODISCARD inline Color ColorKeyword(ValueId id)
  {
    return Color {KeywordColor {id}};
  }
}
