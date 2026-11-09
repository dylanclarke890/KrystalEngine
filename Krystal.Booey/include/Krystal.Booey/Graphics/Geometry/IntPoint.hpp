/*
 * Copyright (C) 2004-2016 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <algorithm>
#include <cmath>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class FloatPoint;
  class IntRect;

  /// @brief A point in integer coordinates.
  class IntPoint
  {
  private:
    int32 _x {0};
    int32 _y {0};

  public:
    constexpr IntPoint() noexcept = default;
    constexpr IntPoint(int32 x, int32 y) noexcept : _x(x), _y(y)
    {
    }

    constexpr explicit IntPoint(const IntSize &size) noexcept : _x(size.Width()), _y(size.Height())
    {
    }

    // Don't do this implicitly since it's lossy.
    explicit IntPoint(const FloatPoint &point) noexcept;

    KRYS_NODISCARD constexpr static IntPoint Zero() noexcept
    {
      return IntPoint();
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return !_x && !_y;
    }

    KRYS_NODISCARD constexpr int32 X() const noexcept
    {
      return _x;
    }

    KRYS_NODISCARD constexpr int32 Y() const noexcept
    {
      return _y;
    }

    constexpr void SetX(int32 x) noexcept
    {
      _x = x;
    }

    constexpr void SetY(int32 y) noexcept
    {
      _y = y;
    }

    constexpr void Move(const IntSize &s) noexcept
    {
      Move(s.Width(), s.Height());
    }

    constexpr void MoveBy(const IntPoint &offset) noexcept
    {
      Move(offset.X(), offset.Y());
    }

    constexpr void Move(int32 dx, int32 dy) noexcept
    {
      _x += dx;
      _y += dy;
    }

    void Scale(float sx, float sy) noexcept
    {
      _x = static_cast<int32>(std::lround(static_cast<float>(_x) * sx));
      _y = static_cast<int32>(std::lround(static_cast<float>(_y) * sy));
    }

    void Scale(float scale) noexcept
    {
      Scale(scale, scale);
    }

    KRYS_NODISCARD IntPoint Scaled(float scale) const noexcept
    {
      return {static_cast<int32>(std::lround(static_cast<float>(_x) * scale)),
              static_cast<int32>(std::lround(static_cast<float>(_y) * scale))};
    }

    KRYS_NODISCARD IntPoint Scaled(float scaleX, float scaleY) const noexcept
    {
      return {static_cast<int32>(std::lround(static_cast<float>(_x) * scaleX)),
              static_cast<int32>(std::lround(static_cast<float>(_y) * scaleY))};
    }

    KRYS_NODISCARD constexpr IntPoint ExpandedTo(const IntPoint &other) const noexcept
    {
      return {_x > other._x ? _x : other._x, _y > other._y ? _y : other._y};
    }

    KRYS_NODISCARD constexpr IntPoint ShrunkTo(const IntPoint &other) const noexcept
    {
      return {_x < other._x ? _x : other._x, _y < other._y ? _y : other._y};
    }

    KRYS_NODISCARD constexpr IntPoint ConstrainedBetween(const IntPoint &min,
                                                         const IntPoint &max) const noexcept
    {
      return {std::max(min.X(), std::min(max.X(), _x)), std::max(min.Y(), std::min(max.Y(), _y))};
    }

    KRYS_NODISCARD IntPoint ConstrainedWithin(const IntRect &rect) const noexcept;

    KRYS_NODISCARD constexpr int32 DistanceSquaredToPoint(const IntPoint &point) const noexcept;

    constexpr void ClampNegativeToZero() noexcept
    {
      *this = ExpandedTo(Zero());
    }

    KRYS_NODISCARD constexpr IntPoint TransposedPoint() const noexcept
    {
      return IntPoint(_y, _x);
    }

    KRYS_NODISCARD friend constexpr bool operator==(const IntPoint &, const IntPoint &) noexcept = default;
  };

  constexpr IntPoint &operator+=(IntPoint &a, const IntSize &b) noexcept
  {
    a.Move(b.Width(), b.Height());
    return a;
  }

  constexpr IntPoint &operator-=(IntPoint &a, const IntSize &b) noexcept
  {
    a.Move(-b.Width(), -b.Height());
    return a;
  }

  KRYS_NODISCARD constexpr IntPoint operator+(const IntPoint &a, const IntSize &b) noexcept
  {
    return IntPoint(a.X() + b.Width(), a.Y() + b.Height());
  }

  KRYS_NODISCARD constexpr IntPoint operator+(const IntPoint &a, const IntPoint &b) noexcept
  {
    return IntPoint(a.X() + b.X(), a.Y() + b.Y());
  }

  KRYS_NODISCARD constexpr IntSize operator-(const IntPoint &a, const IntPoint &b) noexcept
  {
    return IntSize(a.X() - b.X(), a.Y() - b.Y());
  }

  KRYS_NODISCARD constexpr IntPoint operator-(const IntPoint &a, const IntSize &b) noexcept
  {
    return IntPoint(a.X() - b.Width(), a.Y() - b.Height());
  }

  KRYS_NODISCARD constexpr IntPoint operator-(const IntPoint &point) noexcept
  {
    return IntPoint(-point.X(), -point.Y());
  }

  KRYS_NODISCARD constexpr IntSize ToIntSize(const IntPoint &a) noexcept
  {
    return IntSize(a.X(), a.Y());
  }

  constexpr int32 IntPoint::DistanceSquaredToPoint(const IntPoint &point) const noexcept
  {
    return ((*this) - point).DiagonalLengthSquared();
  }

  TextStream &operator<<(TextStream &ts, const IntPoint &value) noexcept;
}
