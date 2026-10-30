#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::css
{
  struct ColorLayersResolver
  {
    boo::BlendMode BlendMode;
    List<boo::Color> Colors;
  };

  /// @brief Composites the layers, the first on top.
  /// @attention `colorLayers` must have at least one colour.
  KRYS_NODISCARD boo::Color BlendSourceOver(const ColorLayersResolver &colorLayers) noexcept;
}
