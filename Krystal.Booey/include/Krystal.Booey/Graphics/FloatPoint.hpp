#pragma once

#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  class AffineTransform;
  class FloatRect;
  class TransformationMatrix;

  /// @brief A point in floating point coordinates.
  class FloatPoint
  {
  private:
    float _x {0};
    float _y {0};

  public:
    constexpr FloatPoint() noexcept = default;
    constexpr FloatPoint(float x, float y) noexcept : _x(x), _y(y)
    {
    }

    constexpr FloatPoint(const IntPoint &p) noexcept
        : _x(static_cast<float>(p.X())), _y(static_cast<float>(p.Y()))
    {
    }

    constexpr explicit FloatPoint(const FloatSize &size) noexcept : _x(size.Width()), _y(size.Height())
    {
    }

    constexpr explicit FloatPoint(const DoublePoint &point) noexcept
        : _x(static_cast<float>(point.X())), _y(static_cast<float>(point.Y()))
    {
    }

    KRYS_NODISCARD constexpr static FloatPoint Zero() noexcept
    {
      return FloatPoint();
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return !_x && !_y;
    }

    KRYS_NODISCARD constexpr static FloatPoint NarrowPrecision(double x, double y) noexcept
    {
      return FloatPoint(static_cast<float>(x), static_cast<float>(y));
    }

    KRYS_NODISCARD constexpr float X() const noexcept
    {
      return _x;
    }

    KRYS_NODISCARD constexpr float Y() const noexcept
    {
      return _y;
    }

    constexpr void SetX(float x) noexcept
    {
      _x = x;
    }

    constexpr void SetY(float y) noexcept
    {
      _y = y;
    }

    constexpr void Set(float x, float y) noexcept
    {
      _x = x;
      _y = y;
    }

    constexpr void Move(float dx, float dy) noexcept
    {
      _x += dx;
      _y += dy;
    }

    constexpr void Move(const IntSize &a) noexcept
    {
      _x += static_cast<float>(a.Width());
      _y += static_cast<float>(a.Height());
    }

    constexpr void Move(const FloatSize &a) noexcept
    {
      _x += a.Width();
      _y += a.Height();
    }

    constexpr void MoveBy(const IntPoint &a) noexcept
    {
      _x += static_cast<float>(a.X());
      _y += static_cast<float>(a.Y());
    }

    constexpr void MoveBy(const FloatPoint &a) noexcept
    {
      _x += a.X();
      _y += a.Y();
    }

    constexpr void Scale(float scale) noexcept
    {
      _x *= scale;
      _y *= scale;
    }

    constexpr void Scale(float scaleX, float scaleY) noexcept
    {
      _x *= scaleX;
      _y *= scaleY;
    }

    KRYS_NODISCARD constexpr FloatPoint Scaled(float scale) const noexcept
    {
      return {_x * scale, _y * scale};
    }

    KRYS_NODISCARD constexpr FloatPoint Scaled(float scaleX, float scaleY) const noexcept
    {
      return {_x * scaleX, _y * scaleY};
    }

    void Rotate(double angleInRadians, const FloatPoint &aboutPoint = {}) noexcept;

    void Normalise() noexcept
    {
      float tempLength = Length();

      if (tempLength)
      {
        _x /= tempLength;
        _y /= tempLength;
      }
    }

    KRYS_NODISCARD constexpr float Dot(const FloatPoint &a) const noexcept
    {
      return _x * a.X() + _y * a.Y();
    }

    KRYS_NODISCARD float SlopeAngleRadians() const noexcept
    {
      return std::atan2(_y, _x);
    }

    KRYS_NODISCARD float Length() const noexcept
    {
      return std::hypot(_x, _y);
    }

    KRYS_NODISCARD constexpr float LengthSquared() const noexcept
    {
      return _x * _x + _y * _y;
    }

    KRYS_NODISCARD constexpr FloatPoint ConstrainedBetween(const FloatPoint &min,
                                                           const FloatPoint &max) const noexcept
    {
      return {std::max(min.X(), std::min(max.X(), _x)), std::max(min.Y(), std::min(max.Y(), _y))};
    }

    KRYS_NODISCARD FloatPoint ConstrainedWithin(const FloatRect &rect) const noexcept;

    KRYS_NODISCARD FloatPoint MatrixTransform(const TransformationMatrix &transform) const noexcept;
    KRYS_NODISCARD FloatPoint MatrixTransform(const AffineTransform &transform) const noexcept;

    KRYS_NODISCARD constexpr FloatPoint ShrunkTo(const FloatPoint &other) const noexcept
    {
      return {std::min(_x, other._x), std::min(_y, other._y)};
    }

    KRYS_NODISCARD constexpr FloatPoint ExpandedTo(const FloatPoint &other) const noexcept
    {
      return {std::max(_x, other._x), std::max(_y, other._y)};
    }

    KRYS_NODISCARD constexpr FloatPoint TransposedPoint() const noexcept
    {
      return {_y, _x};
    }

    KRYS_NODISCARD constexpr FloatPoint ScaledBy(float scale) const noexcept
    {
      return FloatPoint(_x * scale, _y * scale);
    }

    KRYS_NODISCARD constexpr static FloatPoint NanPoint() noexcept
    {
      return {Limits<float>::quiet_NaN(), Limits<float>::quiet_NaN()};
    }

    KRYS_NODISCARD constexpr bool IsNaN() const noexcept
    {
      return krys::IsNaN(_x);
    }

    constexpr operator DoublePoint() const noexcept
    {
      return {_x, _y};
    }

    KRYS_NODISCARD friend constexpr bool operator==(const FloatPoint &,
                                                    const FloatPoint &) noexcept = default;
  };

  constexpr FloatPoint &operator+=(FloatPoint &a, const FloatSize &b) noexcept
  {
    a.Move(b.Width(), b.Height());
    return a;
  }

  constexpr FloatPoint &operator+=(FloatPoint &a, const FloatPoint &b) noexcept
  {
    a.Move(b.X(), b.Y());
    return a;
  }

  constexpr FloatPoint &operator-=(FloatPoint &a, const FloatSize &b) noexcept
  {
    a.Move(-b.Width(), -b.Height());
    return a;
  }

  KRYS_NODISCARD constexpr FloatPoint operator+(const FloatPoint &a, const FloatSize &b) noexcept
  {
    return FloatPoint(a.X() + b.Width(), a.Y() + b.Height());
  }

  KRYS_NODISCARD constexpr FloatPoint operator+(const FloatPoint &a, const FloatPoint &b) noexcept
  {
    return FloatPoint(a.X() + b.X(), a.Y() + b.Y());
  }

  KRYS_NODISCARD constexpr FloatSize operator-(const FloatPoint &a, const FloatPoint &b) noexcept
  {
    return FloatSize(a.X() - b.X(), a.Y() - b.Y());
  }

  KRYS_NODISCARD constexpr FloatPoint operator-(const FloatPoint &a, const FloatSize &b) noexcept
  {
    return FloatPoint(a.X() - b.Width(), a.Y() - b.Height());
  }

  KRYS_NODISCARD constexpr FloatPoint operator-(const FloatPoint &a) noexcept
  {
    return FloatPoint(-a.X(), -a.Y());
  }

  KRYS_NODISCARD constexpr float operator*(const FloatPoint &a, const FloatPoint &b) noexcept
  {
    // dot product
    return a.Dot(b);
  }

  inline void FloatPoint::Rotate(double angleInRadians, const FloatPoint &aboutPoint) noexcept
  {
    auto sinAngle = std::sin(angleInRadians);
    auto cosAngle = std::cos(angleInRadians);
    _x -= aboutPoint.X();
    _y -= aboutPoint.Y();
    auto newX = _x * cosAngle - _y * sinAngle + aboutPoint.X();
    _y = static_cast<float>(_x * sinAngle + _y * cosAngle + aboutPoint.Y());
    _x = static_cast<float>(newX);
  }

  KRYS_NODISCARD inline IntSize FlooredIntSize(const FloatPoint &p) noexcept
  {
    return IntSize(ClampToInteger(std::floor(p.X())), ClampToInteger(std::floor(p.Y())));
  }

  KRYS_NODISCARD inline IntPoint RoundedIntPoint(const FloatPoint &p) noexcept
  {
    return IntPoint(ClampToInteger(std::round(p.X())), ClampToInteger(std::round(p.Y())));
  }

  KRYS_NODISCARD inline IntPoint FlooredIntPoint(const FloatPoint &p) noexcept
  {
    return IntPoint(ClampToInteger(std::floor(p.X())), ClampToInteger(std::floor(p.Y())));
  }

  KRYS_NODISCARD inline IntPoint CeiledIntPoint(const FloatPoint &p) noexcept
  {
    return IntPoint(ClampToInteger(std::ceil(p.X())), ClampToInteger(std::ceil(p.Y())));
  }

  KRYS_NODISCARD inline FloatPoint FloorPointToDevicePixels(const FloatPoint &p,
                                                            float deviceScaleFactor) noexcept
  {
    return FloatPoint(std::floor(p.X() * deviceScaleFactor) / deviceScaleFactor,
                      std::floor(p.Y() * deviceScaleFactor) / deviceScaleFactor);
  }

  KRYS_NODISCARD inline FloatPoint CeilPointToDevicePixels(const FloatPoint &p,
                                                           float deviceScaleFactor) noexcept
  {
    return FloatPoint(std::ceil(p.X() * deviceScaleFactor) / deviceScaleFactor,
                      std::ceil(p.Y() * deviceScaleFactor) / deviceScaleFactor);
  }

  KRYS_NODISCARD constexpr FloatSize ToFloatSize(const FloatPoint &a) noexcept
  {
    return FloatSize(a.X(), a.Y());
  }

  KRYS_NODISCARD constexpr FloatPoint ToFloatPoint(const FloatSize &a) noexcept
  {
    return FloatPoint(a.Width(), a.Height());
  }

  KRYS_NODISCARD inline bool AreEssentiallyEqual(const FloatPoint &a, const FloatPoint &b) noexcept
  {
    return AreEssentiallyEqual(a.X(), b.X()) && AreEssentiallyEqual(a.Y(), b.Y());
  }
}

namespace krys
{
  template <>
  struct DefaultHash<boo::FloatPoint>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FloatPoint &point) noexcept
    {
      return krys::Hash::Combine(point.X(), point.Y());
    }

    KRYS_NODISCARD static bool Equal(const boo::FloatPoint &a, const boo::FloatPoint &b) noexcept
    {
      return a == b;
    }
  };

  template <>
  struct MarkableTraits<boo::FloatPoint>
  {
    KRYS_NODISCARD constexpr static bool IsEmptyValue(const boo::FloatPoint &point) noexcept
    {
      return point.IsNaN();
    }

    KRYS_NODISCARD constexpr static boo::FloatPoint EmptyValue() noexcept
    {
      return boo::FloatPoint::NanPoint();
    }
  };
}
