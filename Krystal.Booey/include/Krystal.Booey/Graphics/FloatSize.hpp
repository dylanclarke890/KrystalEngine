#pragma once

#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  /// @brief A width and height in floating point.
  class FloatSize
  {
  private:
    float _width {0};
    float _height {0};

  public:
    constexpr FloatSize() noexcept = default;
    constexpr FloatSize(float width, float height) noexcept : _width(width), _height(height)
    {
    }

    KRYS_NODISCARD static FloatSize NarrowPrecision(double width, double height) noexcept;

    KRYS_NODISCARD constexpr float Width() const noexcept
    {
      return _width;
    }

    KRYS_NODISCARD constexpr float Height() const noexcept
    {
      return _height;
    }

    KRYS_NODISCARD constexpr float MinDimension() const noexcept
    {
      return std::min(_width, _height);
    }

    KRYS_NODISCARD constexpr float MaxDimension() const noexcept
    {
      return std::max(_width, _height);
    }

    constexpr void SetWidth(float width) noexcept
    {
      _width = width;
    }

    constexpr void SetHeight(float height) noexcept
    {
      _height = height;
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return _width <= 0 || _height <= 0;
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return (_width < 0 ? -_width : _width) < Limits<float>::epsilon()
             && (_height < 0 ? -_height : _height) < Limits<float>::epsilon();
    }

    KRYS_NODISCARD constexpr float AspectRatio() const noexcept
    {
      return _width / _height;
    }

    KRYS_NODISCARD constexpr double AspectRatioDouble() const noexcept
    {
      return _width / static_cast<double>(_height);
    }

    constexpr void Expand(float width, float height) noexcept
    {
      _width += width;
      _height += height;
    }

    constexpr void Scale(float s) noexcept
    {
      _width *= s;
      _height *= s;
    }

    constexpr void Scale(float scaleX, float scaleY) noexcept
    {
      _width *= scaleX;
      _height *= scaleY;
    }

    KRYS_NODISCARD constexpr FloatSize Scaled(float s) const noexcept
    {
      return {_width * s, _height * s};
    }

    KRYS_NODISCARD constexpr FloatSize Scaled(float scaleX, float scaleY) const noexcept
    {
      return {_width * scaleX, _height * scaleY};
    }

    KRYS_NODISCARD constexpr FloatSize ConstrainedBetween(const FloatSize &min,
                                                          const FloatSize &max) const noexcept
    {
      return {std::max(min._width, std::min(max._width, _width)),
              std::max(min._height, std::min(max._height, _height))};
    }

    KRYS_NODISCARD constexpr FloatSize ExpandedTo(const FloatSize &other) const noexcept
    {
      return FloatSize(_width > other._width ? _width : other._width,
                       _height > other._height ? _height : other._height);
    }

    KRYS_NODISCARD constexpr FloatSize ShrunkTo(const FloatSize &other) const noexcept
    {
      return FloatSize(_width < other._width ? _width : other._width,
                       _height < other._height ? _height : other._height);
    }

    KRYS_NODISCARD float DiagonalLength() const noexcept
    {
      return std::hypot(_width, _height);
    }

    KRYS_NODISCARD constexpr float DiagonalLengthSquared() const noexcept
    {
      return _width * _width + _height * _height;
    }

    KRYS_NODISCARD constexpr float Area() const noexcept
    {
      return _width * _height;
    }

    KRYS_NODISCARD constexpr FloatSize TransposedSize() const noexcept
    {
      return FloatSize(_height, _width);
    }

    KRYS_NODISCARD constexpr FloatSize ScaledBy(float scale) const noexcept
    {
      return ScaledBy(scale, scale);
    }

    KRYS_NODISCARD constexpr FloatSize ScaledBy(float scaleX, float scaleY) const noexcept
    {
      return FloatSize(_width * scaleX, _height * scaleY);
    }

    KRYS_NODISCARD constexpr static FloatSize NanSize() noexcept
    {
      return {Limits<float>::quiet_NaN(), Limits<float>::quiet_NaN()};
    }

    KRYS_NODISCARD constexpr bool IsNaN() const noexcept
    {
      // NaN is the only value that isn't equal to itself.
      return _width != _width;
    }

    KRYS_NODISCARD friend constexpr bool operator==(const FloatSize &, const FloatSize &) noexcept = default;
  };

  inline FloatSize FloatSize::NarrowPrecision(double width, double height) noexcept
  {
    return FloatSize(static_cast<float>(width), static_cast<float>(height));
  }

  constexpr FloatSize &operator+=(FloatSize &a, const FloatSize &b) noexcept
  {
    a.SetWidth(a.Width() + b.Width());
    a.SetHeight(a.Height() + b.Height());
    return a;
  }

  constexpr FloatSize &operator-=(FloatSize &a, const FloatSize &b) noexcept
  {
    a.SetWidth(a.Width() - b.Width());
    a.SetHeight(a.Height() - b.Height());
    return a;
  }

  KRYS_NODISCARD constexpr FloatSize operator+(const FloatSize &a, const FloatSize &b) noexcept
  {
    return FloatSize(a.Width() + b.Width(), a.Height() + b.Height());
  }

  KRYS_NODISCARD constexpr FloatSize operator-(const FloatSize &a, const FloatSize &b) noexcept
  {
    return FloatSize(a.Width() - b.Width(), a.Height() - b.Height());
  }

  KRYS_NODISCARD constexpr FloatSize operator-(const FloatSize &size) noexcept
  {
    return FloatSize(-size.Width(), -size.Height());
  }

  KRYS_NODISCARD constexpr FloatSize operator*(const FloatSize &a, float b) noexcept
  {
    return FloatSize(a.Width() * b, a.Height() * b);
  }

  KRYS_NODISCARD constexpr FloatSize operator*(float a, const FloatSize &b) noexcept
  {
    return FloatSize(a * b.Width(), a * b.Height());
  }

  KRYS_NODISCARD constexpr FloatSize operator*(const FloatSize &a, const FloatSize &b) noexcept
  {
    return FloatSize(a.Width() * b.Width(), a.Height() * b.Height());
  }

  KRYS_NODISCARD constexpr FloatSize operator/(const FloatSize &a, const FloatSize &b) noexcept
  {
    return FloatSize(a.Width() / b.Width(), a.Height() / b.Height());
  }

  KRYS_NODISCARD constexpr FloatSize operator/(const FloatSize &a, float b) noexcept
  {
    return FloatSize(a.Width() / b, a.Height() / b);
  }

  KRYS_NODISCARD constexpr FloatSize operator/(float a, const FloatSize &b) noexcept
  {
    return FloatSize(a / b.Width(), a / b.Height());
  }

  KRYS_NODISCARD inline bool AreEssentiallyEqual(const FloatSize &a, const FloatSize &b) noexcept
  {
    return AreEssentiallyEqual(a.Width(), b.Width()) && AreEssentiallyEqual(a.Height(), b.Height());
  }
}
