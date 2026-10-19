#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  // Legacy shadow blur radius is used for canvas, and -webkit-box-shadow.
  // It has different treatment of radii > 8px.
  enum class ShadowRadiusMode : bool
  {
    Default,
    Legacy
  };

  /// @brief A shadow cast by what's drawn: offset, blurred and coloured.
  struct GraphicsDropShadow
  {
    FloatSize Offset;
    float Radius;
    boo::Color Color;
    ShadowRadiusMode RadiusMode {ShadowRadiusMode::Default};
    float Opacity {1};

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return Color.IsVisible();
    }
    KRYS_NODISCARD bool IsBlurred() const noexcept
    {
      return IsVisible() && Radius;
    }
    KRYS_NODISCARD bool HasOutsets() const noexcept
    {
      return IsBlurred() || (IsVisible() && !Offset.IsZero());
    }

    KRYS_NODISCARD friend bool operator==(const GraphicsDropShadow &a, const GraphicsDropShadow &b) noexcept
    {
      return a.Offset == b.Offset && a.Radius == b.Radius && a.Color == b.Color;
    }
  };

  /// @brief A Gaussian blur applied to what's drawn.
  struct GraphicsGaussianBlur
  {
    FloatSize Radius;

    KRYS_NODISCARD friend bool operator==(const GraphicsGaussianBlur &,
                                          const GraphicsGaussianBlur &) = default;
  };

  /// @brief A colour matrix applied to what's drawn.
  struct GraphicsColorMatrix
  {
    Array<float, 20> Values; // Really a ColourMatrix<5, 4>.

    KRYS_NODISCARD friend bool operator==(const GraphicsColorMatrix &, const GraphicsColorMatrix &) = default;
  };

  using GraphicsStyle = Variant<GraphicsDropShadow, GraphicsGaussianBlur, GraphicsColorMatrix>;

  TextStream &operator<<(TextStream &ts, const GraphicsDropShadow &dropShadow) noexcept;
  TextStream &operator<<(TextStream &ts, const GraphicsGaussianBlur &gaussianBlur) noexcept;
  TextStream &operator<<(TextStream &ts, const GraphicsColorMatrix &colorMatrix) noexcept;
  TextStream &operator<<(TextStream &ts, const GraphicsStyle &style) noexcept;
}
