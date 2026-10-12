#pragma once

#include "Krystal.Booey/Graphics/Color/ColorTypes.hpp"
#include <cmath>

namespace krys::boo
{
  // MARK: - Normalization

  template <typename ComponentType>
  KRYS_NODISCARD inline ComponentType NormalizeHue(ComponentType hue) noexcept
  {
    return static_cast<ComponentType>(std::fmod(std::fmod(hue, 360.0) + 360.0, 360.0));
  }

  template <typename ColorType>
  KRYS_NODISCARD inline ColorType
    CreateColorTypeByNormalizingComponents(const ColorComponents<float, 4> &colorComponents) noexcept
  {
    return CreateFromComponentsClamping<ColorType>(colorComponents);
  }
}
