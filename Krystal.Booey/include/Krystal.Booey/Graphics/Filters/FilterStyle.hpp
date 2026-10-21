#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsStyle.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief One filter primitive applied by the graphics context: the style its transparency layer is
  /// composited with, and where the primitive's result goes.
  struct FilterStyle
  {
    Maybe<GraphicsStyle> Style;
    FloatRect PrimitiveSubregion;
    FloatRect ImageRect;
  };

  using FilterStyleVector = List<FilterStyle>;
}
