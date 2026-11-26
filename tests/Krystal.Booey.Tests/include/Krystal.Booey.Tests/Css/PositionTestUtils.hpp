#pragma once

#include "Krystal.Booey/Css/Values/Primitives/Position.hpp"

namespace krys::boo::css::tests
{
  /// @brief The position at @p x and @p y.
  KRYS_NODISCARD inline Position PositionAt(TwoComponentPositionHorizontal x, TwoComponentPositionVertical y)
  {
    return Position {TwoComponentPositionHorizontalVertical {krys::move(x), krys::move(y)}};
  }

  /// @brief The centre position (`center center`).
  KRYS_NODISCARD inline Position CentrePosition()
  {
    return PositionAt({keywords::Center {}}, {keywords::Center {}});
  }
}
