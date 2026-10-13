#include "Krystal.Booey/CSS/Values/Color/ColorLayersResolver.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"

namespace krys::boo::css
{
  boo::Color BlendSourceOver(const ColorLayersResolver &colorLayers) noexcept
  {
    krys_debug_assert(!colorLayers.Colors.empty());

    Maybe<boo::Color> resultingColor;
    for (const auto &color : colorLayers.Colors)
    {
      // FIXME: Apply blend-mode.
      resultingColor = !resultingColor ? color : boo::BlendSourceOver(color, *resultingColor);
      if (color.IsOpaque() && colorLayers.BlendMode == BlendMode::Normal)
      {
        return *resultingColor;
      }
    }
    return *resultingColor;
  }
}
