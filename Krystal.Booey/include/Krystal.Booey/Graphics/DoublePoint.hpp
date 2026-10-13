#pragma once

#include "Krystal.Booey/Graphics/DoubleSize.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>

namespace krys::boo
{
  /// @brief A point in double precision coordinates.
  class DoublePoint
  {
  private:
    double _x {0};
    double _y {0};

  public:
    constexpr DoublePoint() noexcept = default;
    constexpr DoublePoint(double x, double y) noexcept : _x(x), _y(y)
    {
    }

    constexpr DoublePoint(const IntPoint &point) noexcept : _x(point.X()), _y(point.Y())
    {
    }

    constexpr explicit DoublePoint(const DoubleSize &size) noexcept : _x(size.Width()), _y(size.Height())
    {
    }

    KRYS_NODISCARD constexpr static DoublePoint Zero() noexcept
    {
      return DoublePoint();
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return !_x && !_y;
    }

    KRYS_NODISCARD constexpr double X() const noexcept
    {
      return _x;
    }

    KRYS_NODISCARD constexpr double Y() const noexcept
    {
      return _y;
    }

    constexpr void Move(double dx, double dy) noexcept
    {
      _x += dx;
      _y += dy;
    }

    constexpr void Move(const IntSize &a) noexcept
    {
      _x += a.Width();
      _y += a.Height();
    }

    constexpr void Move(const DoubleSize &a) noexcept
    {
      _x += a.Width();
      _y += a.Height();
    }

    constexpr void MoveBy(const DoublePoint &a) noexcept
    {
      _x += a.X();
      _y += a.Y();
    }

    constexpr void Scale(double scale) noexcept
    {
      _x *= scale;
      _y *= scale;
    }

    KRYS_NODISCARD constexpr DoublePoint Scaled(double scale) const noexcept
    {
      return {_x * scale, _y * scale};
    }

    KRYS_NODISCARD friend constexpr bool operator==(const DoublePoint &,
                                                    const DoublePoint &) noexcept = default;
  };

  KRYS_NODISCARD constexpr DoublePoint operator+(const DoublePoint &a, const DoubleSize &b) noexcept
  {
    return DoublePoint(a.X() + b.Width(), a.Y() + b.Height());
  }

  KRYS_NODISCARD constexpr DoublePoint operator+(const DoublePoint &a, const DoublePoint &b) noexcept
  {
    return DoublePoint(a.X() + b.X(), a.Y() + b.Y());
  }

  KRYS_NODISCARD constexpr DoubleSize operator-(const DoublePoint &a, const DoublePoint &b) noexcept
  {
    return DoubleSize(a.X() - b.X(), a.Y() - b.Y());
  }

  KRYS_NODISCARD constexpr DoublePoint operator-(const DoublePoint &a, const DoubleSize &b) noexcept
  {
    return DoublePoint(a.X() - b.Width(), a.Y() - b.Height());
  }

  KRYS_NODISCARD inline IntPoint FlooredIntPoint(const DoublePoint &p) noexcept
  {
    return IntPoint(ClampToInteger(std::floor(p.X())), ClampToInteger(std::floor(p.Y())));
  }

  KRYS_NODISCARD inline IntPoint RoundedIntPoint(const DoublePoint &p) noexcept
  {
    return IntPoint(ClampToInteger(std::round(p.X())), ClampToInteger(std::round(p.Y())));
  }

  KRYS_NODISCARD constexpr DoubleSize ToDoubleSize(const DoublePoint &a) noexcept
  {
    return DoubleSize(a.X(), a.Y());
  }
}
