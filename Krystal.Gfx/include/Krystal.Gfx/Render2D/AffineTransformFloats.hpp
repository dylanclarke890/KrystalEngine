#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Geometry/AffineTransform.hpp"
#include "Krystal.Core/Maths/Vector.hpp"

namespace krys::gfx
{
  /// @brief @p point mapped by @p transform (`x' = A x + C y + E`, `y' = B x + D y + F`), in float.
  KRYS_NODISCARD constexpr Vec2f MapPoint(const AffineTransform &transform, Vec2f point) noexcept
  {
    return {static_cast<float>(transform.A() * point.x + transform.C() * point.y + transform.E()),
            static_cast<float>(transform.B() * point.x + transform.D() * point.y + transform.F())};
  }

  /// @brief @p transform's linear part (`A`, `B`, `C`, `D`), in float for the GPU.
  KRYS_NODISCARD constexpr Array<float, 4> LinearPartF(const AffineTransform &transform) noexcept
  {
    return {static_cast<float>(transform.A()), static_cast<float>(transform.B()),
            static_cast<float>(transform.C()), static_cast<float>(transform.D())};
  }

  /// @brief @p transform's translation (`E`, `F`), in float for the GPU.
  KRYS_NODISCARD constexpr Array<float, 2> TranslationF(const AffineTransform &transform) noexcept
  {
    return {static_cast<float>(transform.E()), static_cast<float>(transform.F())};
  }
}
