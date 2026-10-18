#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Gfx/Render2D/Gradient2D.hpp"
#include "Krystal.Gfx/Render2D/Transform2D.hpp"

namespace krys::gfx
{
  /// @brief What a fill or stroke paints with: a colour, or a gradient.
  /// @attention A gradient paint refers to its gradient rather than copying it, so the gradient must outlive
  /// the paint; a draw is done with it once the call returns.
  class Paint2D
  {
    SRGBA<uint8> _colour {};
    const Gradient2D *_gradient {nullptr};
    Transform2D _gradientSpace {};

  public:
    /// @brief A solid colour, sRGB with straight alpha.
    constexpr Paint2D(SRGBA<uint8> colour) noexcept : _colour(colour)
    {
    }

    /// @brief A gradient, its coordinates mapped into the draw's by `gradientSpace`.
    constexpr Paint2D(const Gradient2D &gradient, const Transform2D &gradientSpace = {}) noexcept
        : _gradient(&gradient), _gradientSpace(gradientSpace)
    {
    }

    KRYS_NODISCARD constexpr SRGBA<uint8> GetColour() const noexcept
    {
      return _colour;
    }

    /// @brief The gradient, or null for a solid colour.
    KRYS_NODISCARD constexpr const Gradient2D *GetGradient() const noexcept
    {
      return _gradient;
    }

    /// @brief Maps the gradient's coordinates into the coordinates of the draw it paints.
    KRYS_NODISCARD constexpr const Transform2D &GetGradientSpace() const noexcept
    {
      return _gradientSpace;
    }
  };
}
