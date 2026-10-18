#pragma once

#include "Krystal.Core/Colour/ColourTypes.hpp"
#include <cmath>

namespace krys
{
  // MARK: - Normalization

  template <typename ComponentType>
  KRYS_NODISCARD inline ComponentType NormalizeHue(ComponentType hue) noexcept
  {
    return static_cast<ComponentType>(std::fmod(std::fmod(hue, 360.0) + 360.0, 360.0));
  }

  template <typename ColourType>
  KRYS_NODISCARD inline ColourType
    CreateColourTypeByNormalizingComponents(const ColourComponents<float, 4> &colorComponents) noexcept
  {
    return CreateFromComponentsClamping<ColourType>(colorComponents);
  }
}
