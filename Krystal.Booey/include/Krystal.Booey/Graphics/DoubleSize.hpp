#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class FloatSize;

  /// @brief A width and height in double precision.
  class DoubleSize
  {
  private:
    double _width {0};
    double _height {0};

  public:
    constexpr DoubleSize() noexcept = default;
    constexpr DoubleSize(double width, double height) noexcept : _width(width), _height(height)
    {
    }

    explicit DoubleSize(const FloatSize &size) noexcept;

    KRYS_NODISCARD constexpr double Width() const noexcept
    {
      return _width;
    }

    KRYS_NODISCARD constexpr double Height() const noexcept
    {
      return _height;
    }

    KRYS_NODISCARD constexpr DoubleSize ScaledBy(double scaleX, double scaleY) const noexcept
    {
      return DoubleSize(_width * scaleX, _height * scaleY);
    }

    KRYS_NODISCARD constexpr DoubleSize ScaledBy(double scale) const noexcept
    {
      return ScaledBy(scale, scale);
    }

    KRYS_NODISCARD friend constexpr bool operator==(const DoubleSize &,
                                                    const DoubleSize &) noexcept = default;
  };
}
