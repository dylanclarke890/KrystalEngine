#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <algorithm>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class FloatSize;

  /// @brief A width and height in integers.
  class IntSize
  {
  private:
    int32 _width {0};
    int32 _height {0};

  public:
    constexpr IntSize() noexcept = default;
    constexpr IntSize(int32 width, int32 height) noexcept : _width(width), _height(height)
    {
    }

    // Don't do this implicitly since it's lossy.
    explicit IntSize(const FloatSize &size) noexcept;

    KRYS_NODISCARD constexpr int32 Width() const noexcept
    {
      return _width;
    }

    KRYS_NODISCARD constexpr int32 Height() const noexcept
    {
      return _height;
    }

    KRYS_NODISCARD constexpr int32 MinDimension() const noexcept
    {
      return std::min(_width, _height);
    }

    KRYS_NODISCARD constexpr int32 MaxDimension() const noexcept
    {
      return std::max(_width, _height);
    }

    constexpr void SetWidth(int32 width) noexcept
    {
      _width = width;
    }

    constexpr void SetHeight(int32 height) noexcept
    {
      _height = height;
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return _width <= 0 || _height <= 0;
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return !_width && !_height;
    }

    KRYS_NODISCARD constexpr float AspectRatio() const noexcept
    {
      return static_cast<float>(_width) / static_cast<float>(_height);
    }

    constexpr void Expand(int32 width, int32 height) noexcept
    {
      _width += width;
      _height += height;
    }

    constexpr void Contract(int32 width, int32 height) noexcept
    {
      _width -= width;
      _height -= height;
    }

    constexpr void Scale(float widthScale, float heightScale) noexcept
    {
      _width = static_cast<int32>(static_cast<float>(_width) * widthScale);
      _height = static_cast<int32>(static_cast<float>(_height) * heightScale);
    }

    constexpr void Scale(float scale) noexcept
    {
      Scale(scale, scale);
    }

    KRYS_NODISCARD constexpr IntSize Scaled(float widthScale, float heightScale) const noexcept
    {
      return {static_cast<int32>(static_cast<float>(_width) * widthScale),
              static_cast<int32>(static_cast<float>(_height) * heightScale)};
    }

    KRYS_NODISCARD constexpr IntSize Scaled(float s) const noexcept
    {
      return Scaled(s, s);
    }

    KRYS_NODISCARD constexpr IntSize ExpandedTo(const IntSize &other) const noexcept
    {
      return IntSize(std::max(_width, other._width), std::max(_height, other._height));
    }

    KRYS_NODISCARD constexpr IntSize ShrunkTo(const IntSize &other) const noexcept
    {
      return IntSize(std::min(_width, other._width), std::min(_height, other._height));
    }

    constexpr void ClampNegativeToZero() noexcept
    {
      *this = ExpandedTo(IntSize());
    }

    constexpr void ClampToMinimumSize(const IntSize &minimumSize) noexcept
    {
      _width = std::max(_width, minimumSize.Width());
      _height = std::max(_height, minimumSize.Height());
    }

    constexpr void ClampToMaximumSize(const IntSize &maximumSize) noexcept
    {
      _width = std::min(_width, maximumSize.Width());
      _height = std::min(_height, maximumSize.Height());
    }

    KRYS_NODISCARD constexpr IntSize ConstrainedBetween(const IntSize &min, const IntSize &max) const noexcept
    {
      return {std::max(min.Width(), std::min(max.Width(), _width)),
              std::max(min.Height(), std::min(max.Height(), _height))};
    }

    KRYS_NODISCARD constexpr uint64 UnclampedArea() const noexcept
    {
      return static_cast<uint64>(_width < 0 ? -static_cast<int64>(_width) : _width)
             * static_cast<uint64>(_height < 0 ? -static_cast<int64>(_height) : _height);
    }

    KRYS_NODISCARD constexpr int32 DiagonalLengthSquared() const noexcept
    {
      return _width * _width + _height * _height;
    }

    KRYS_NODISCARD constexpr IntSize TransposedSize() const noexcept
    {
      return IntSize(_height, _width);
    }

    KRYS_NODISCARD friend constexpr bool operator==(const IntSize &, const IntSize &) noexcept = default;
  };

  constexpr IntSize &operator+=(IntSize &a, const IntSize &b) noexcept
  {
    a.SetWidth(a.Width() + b.Width());
    a.SetHeight(a.Height() + b.Height());
    return a;
  }

  constexpr IntSize &operator-=(IntSize &a, const IntSize &b) noexcept
  {
    a.SetWidth(a.Width() - b.Width());
    a.SetHeight(a.Height() - b.Height());
    return a;
  }

  KRYS_NODISCARD constexpr IntSize operator+(const IntSize &a, const IntSize &b) noexcept
  {
    return IntSize(a.Width() + b.Width(), a.Height() + b.Height());
  }

  KRYS_NODISCARD constexpr IntSize operator-(const IntSize &a, const IntSize &b) noexcept
  {
    return IntSize(a.Width() - b.Width(), a.Height() - b.Height());
  }

  KRYS_NODISCARD constexpr IntSize operator-(const IntSize &size) noexcept
  {
    return IntSize(-size.Width(), -size.Height());
  }

  TextStream &operator<<(TextStream &ts, const IntSize &value) noexcept;
}
