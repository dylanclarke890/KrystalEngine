#pragma once

#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief A rotation quaternion, used to interpolate decomposed 3D transforms.
  struct Quaternion
  {
    double X;
    double Y;
    double Z;
    double W;

    KRYS_NODISCARD constexpr bool operator==(const Quaternion &other) const noexcept = default;

    KRYS_NODISCARD Quaternion Slerp(const Quaternion &other, double progress) const noexcept;
    KRYS_NODISCARD Quaternion Accumulate(const Quaternion &other) const noexcept;
    KRYS_NODISCARD Quaternion Interpolate(const Quaternion &other, double progress,
                                          CompositeOperation compositeOperation) const noexcept;
  };
}
