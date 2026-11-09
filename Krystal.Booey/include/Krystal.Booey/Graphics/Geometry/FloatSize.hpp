/*
 * Copyright (C) 2003-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
 * Copyright (C) 2005 Nokia. All rights reserved.
 *               2008 Eric Seidel <eric@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/DoubleSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include <algorithm>
#include <cmath>

namespace krys
{
  class TextStream;
}

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

    constexpr FloatSize(const IntSize &size) noexcept
        : _width(static_cast<float>(size.Width())), _height(static_cast<float>(size.Height()))
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

    KRYS_NODISCARD constexpr bool IsExpressibleAsIntSize() const noexcept
    {
      return IsWithinIntRange(_width) && IsWithinIntRange(_height);
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

    constexpr operator DoubleSize() const noexcept
    {
      return {_width, _height};
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

  /// @brief The determinant of the matrix whose columns are @p a and @p b (the z of their cross product).
  KRYS_NODISCARD constexpr float Determinant(const FloatSize &a, const FloatSize &b) noexcept
  {
    return a.Width() * b.Height() - a.Height() * b.Width();
  }

  // Keeps the scalar overloads visible alongside this one.
  using krys::AreEssentiallyEqual;

  KRYS_NODISCARD inline bool AreEssentiallyEqual(const FloatSize &a, const FloatSize &b) noexcept
  {
    return AreEssentiallyEqual(a.Width(), b.Width()) && AreEssentiallyEqual(a.Height(), b.Height());
  }

  KRYS_NODISCARD inline IntSize RoundedIntSize(const FloatSize &p) noexcept
  {
    return IntSize(ClampToInteger(std::round(p.Width())), ClampToInteger(std::round(p.Height())));
  }

  KRYS_NODISCARD inline IntSize FlooredIntSize(const FloatSize &p) noexcept
  {
    return IntSize(ClampToInteger(std::floor(p.Width())), ClampToInteger(std::floor(p.Height())));
  }

  KRYS_NODISCARD inline IntSize ExpandedIntSize(const FloatSize &p) noexcept
  {
    return IntSize(ClampToInteger(std::ceil(p.Width())), ClampToInteger(std::ceil(p.Height())));
  }

  KRYS_NODISCARD inline IntPoint FlooredIntPoint(const FloatSize &p) noexcept
  {
    return IntPoint(ClampToInteger(std::floor(p.Width())), ClampToInteger(std::floor(p.Height())));
  }

  TextStream &operator<<(TextStream &ts, const FloatSize &value) noexcept;
}
