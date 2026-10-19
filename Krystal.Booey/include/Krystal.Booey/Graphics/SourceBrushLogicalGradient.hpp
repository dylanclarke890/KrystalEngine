#pragma once

#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"

namespace krys::boo
{
  /// @brief A gradient a brush paints with, and the transform from its space to the user space.
  struct SourceBrushLogicalGradient
  {
    Ref<boo::Gradient> Gradient;
    AffineTransform SpaceTransform;

    KRYS_NODISCARD friend bool operator==(const SourceBrushLogicalGradient &a,
                                          const SourceBrushLogicalGradient &b) noexcept
    {
      return a.Gradient.get() == b.Gradient.get() && a.SpaceTransform == b.SpaceTransform;
    }
  };
}
