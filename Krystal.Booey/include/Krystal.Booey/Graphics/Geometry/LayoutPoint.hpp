/*
 * Copyright (c) 2012-2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include <algorithm>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief A point in layout units.
  class LayoutPoint
  {
  private:
    LayoutUnit _x;
    LayoutUnit _y;

  public:
    constexpr LayoutPoint() noexcept = default;

    template <typename T, typename U>
    constexpr LayoutPoint(T x, U y) noexcept : _x(x), _y(y)
    {
    }

    constexpr LayoutPoint(const PointI &point) noexcept : _x(point.X()), _y(point.Y())
    {
    }

    constexpr explicit LayoutPoint(const PointF &point) noexcept : _x(point.X()), _y(point.Y())
    {
    }

    constexpr explicit LayoutPoint(const LayoutSize &size) noexcept : _x(size.Width()), _y(size.Height())
    {
    }

    constexpr explicit LayoutPoint(const PointD &point) noexcept : _x(point.X()), _y(point.Y())
    {
    }

    KRYS_NODISCARD constexpr static LayoutPoint Zero() noexcept
    {
      return LayoutPoint();
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return !_x && !_y;
    }

    KRYS_NODISCARD constexpr LayoutUnit X() const noexcept
    {
      return _x;
    }

    KRYS_NODISCARD constexpr LayoutUnit Y() const noexcept
    {
      return _y;
    }

    template <typename T>
    constexpr void SetX(T x) noexcept
    {
      _x = x;
    }

    template <typename T>
    constexpr void SetY(T y) noexcept
    {
      _y = y;
    }

    constexpr void Move(const LayoutSize &s) noexcept
    {
      Move(s.Width(), s.Height());
    }

    constexpr void MoveBy(const LayoutPoint &offset) noexcept
    {
      Move(offset.X(), offset.Y());
    }

    template <typename T, typename U>
    constexpr void Move(T dx, U dy) noexcept
    {
      _x += dx;
      _y += dy;
    }

    KRYS_NODISCARD friend constexpr bool operator==(const LayoutPoint &,
                                                    const LayoutPoint &) noexcept = default;

    constexpr void Scale(float s) noexcept
    {
      _x *= s;
      _y *= s;
    }

    constexpr void Scale(float sx, float sy) noexcept
    {
      _x *= sx;
      _y *= sy;
    }

    KRYS_NODISCARD constexpr LayoutPoint Scaled(float s) const noexcept
    {
      return {_x * s, _y * s};
    }

    KRYS_NODISCARD constexpr LayoutPoint Scaled(float sx, float sy) const noexcept
    {
      return {_x * sx, _y * sy};
    }

    KRYS_NODISCARD constexpr LayoutPoint ConstrainedBetween(const LayoutPoint &min,
                                                            const LayoutPoint &max) const noexcept
    {
      return {std::max(min.X(), std::min(max.X(), _x)), std::max(min.Y(), std::min(max.Y(), _y))};
    }

    KRYS_NODISCARD constexpr LayoutPoint ExpandedTo(const LayoutPoint &other) const noexcept
    {
      return {std::max(_x, other._x), std::max(_y, other._y)};
    }

    KRYS_NODISCARD constexpr LayoutPoint ShrunkTo(const LayoutPoint &other) const noexcept
    {
      return {std::min(_x, other._x), std::min(_y, other._y)};
    }

    constexpr void ClampNegativeToZero() noexcept
    {
      *this = ExpandedTo(Zero());
    }

    KRYS_NODISCARD constexpr LayoutPoint TransposedPoint() const noexcept
    {
      return {_y, _x};
    }

    KRYS_NODISCARD constexpr LayoutPoint Fraction() const noexcept
    {
      return {_x.Fraction(), _y.Fraction()};
    }

    constexpr operator PointF() const noexcept
    {
      return {_x, _y};
    }

    constexpr operator PointD() const noexcept
    {
      return {_x.ToDouble(), _y.ToDouble()};
    }
  };

  constexpr LayoutPoint &operator+=(LayoutPoint &a, const LayoutSize &b) noexcept
  {
    a.Move(b.Width(), b.Height());
    return a;
  }

  constexpr LayoutPoint &operator-=(LayoutPoint &a, const LayoutSize &b) noexcept
  {
    a.Move(-b.Width(), -b.Height());
    return a;
  }

  KRYS_NODISCARD constexpr LayoutPoint operator+(const LayoutPoint &a, const LayoutSize &b) noexcept
  {
    return LayoutPoint(a.X() + b.Width(), a.Y() + b.Height());
  }

  KRYS_NODISCARD constexpr LayoutPoint operator+(const LayoutPoint &a, const LayoutPoint &b) noexcept
  {
    return LayoutPoint(a.X() + b.X(), a.Y() + b.Y());
  }

  KRYS_NODISCARD constexpr LayoutSize operator-(const LayoutPoint &a, const LayoutPoint &b) noexcept
  {
    return LayoutSize(a.X() - b.X(), a.Y() - b.Y());
  }

  KRYS_NODISCARD constexpr LayoutPoint operator-(const LayoutPoint &a, const LayoutSize &b) noexcept
  {
    return LayoutPoint(a.X() - b.Width(), a.Y() - b.Height());
  }

  KRYS_NODISCARD constexpr LayoutPoint operator-(const LayoutPoint &point) noexcept
  {
    return LayoutPoint(-point.X(), -point.Y());
  }

  KRYS_NODISCARD constexpr LayoutPoint ToLayoutPoint(const LayoutSize &size) noexcept
  {
    return LayoutPoint(size.Width(), size.Height());
  }

  KRYS_NODISCARD constexpr LayoutSize ToLayoutSize(const LayoutPoint &point) noexcept
  {
    return LayoutSize(point.X(), point.Y());
  }

  KRYS_NODISCARD constexpr PointI FlooredPointI(const LayoutPoint &point) noexcept
  {
    return PointI(point.X().Floor(), point.Y().Floor());
  }

  KRYS_NODISCARD constexpr PointI RoundedPointI(const LayoutPoint &point) noexcept
  {
    return PointI(point.X().Round(), point.Y().Round());
  }

  KRYS_NODISCARD constexpr PointI RoundedPointI(const LayoutSize &size) noexcept
  {
    return RoundedPointI(ToLayoutPoint(size));
  }

  KRYS_NODISCARD constexpr PointI CeiledPointI(const LayoutPoint &point) noexcept
  {
    return PointI(point.X().Ceil(), point.Y().Ceil());
  }

  KRYS_NODISCARD inline LayoutPoint FlooredLayoutPoint(const PointF &p) noexcept
  {
    return LayoutPoint(LayoutUnit::FromFloatFloor(p.X()), LayoutUnit::FromFloatFloor(p.Y()));
  }

  KRYS_NODISCARD inline LayoutPoint CeiledLayoutPoint(const PointF &p) noexcept
  {
    return LayoutPoint(LayoutUnit::FromFloatCeil(p.X()), LayoutUnit::FromFloatCeil(p.Y()));
  }

  KRYS_NODISCARD constexpr SizeI SnappedSizeI(const LayoutSize &size, const LayoutPoint &location) noexcept
  {
    auto snap = [](LayoutUnit a, LayoutUnit b)
    {
      LayoutUnit fraction = b.Fraction();
      return RoundToInt(fraction + a) - RoundToInt(fraction);
    };
    return SizeI(snap(size.Width(), location.X()), snap(size.Height(), location.Y()));
  }

  KRYS_NODISCARD inline PointF RoundPointToDevicePixels(const LayoutPoint &point, float pixelSnappingFactor,
                                                        bool directionalRoundingToRight = true,
                                                        bool directionalRoundingToBottom = true) noexcept
  {
    return PointF(RoundToDevicePixel(point.X(), pixelSnappingFactor, !directionalRoundingToRight),
                  RoundToDevicePixel(point.Y(), pixelSnappingFactor, !directionalRoundingToBottom));
  }

  KRYS_NODISCARD inline PointF FloorPointToDevicePixels(const LayoutPoint &point,
                                                        float pixelSnappingFactor) noexcept
  {
    return PointF(FloorToDevicePixel(point.X(), pixelSnappingFactor),
                  FloorToDevicePixel(point.Y(), pixelSnappingFactor));
  }

  KRYS_NODISCARD inline PointF CeilPointToDevicePixels(const LayoutPoint &point,
                                                       float pixelSnappingFactor) noexcept
  {
    return PointF(CeilToDevicePixel(point.X(), pixelSnappingFactor),
                  CeilToDevicePixel(point.Y(), pixelSnappingFactor));
  }

  KRYS_NODISCARD inline SizeF SnapSizeToDevicePixel(const LayoutSize &size, const LayoutPoint &location,
                                                    float pixelSnappingFactor) noexcept
  {
    auto snap = [&](LayoutUnit a, LayoutUnit b)
    {
      LayoutUnit fraction = b.Fraction();
      return RoundToDevicePixel(fraction + a, pixelSnappingFactor)
             - RoundToDevicePixel(fraction, pixelSnappingFactor);
    };
    return SizeF(snap(size.Width(), location.X()), snap(size.Height(), location.Y()));
  }

  struct LayoutPointLimits
  {
    LayoutPoint Min;
    LayoutPoint Max;

    KRYS_NODISCARD constexpr LayoutPoint Clamp(const LayoutPoint &point) const noexcept
    {
      return point.ConstrainedBetween(Min, Max);
    }

    KRYS_NODISCARD constexpr bool Fits(const LayoutPoint &point) const noexcept
    {
      return point == Clamp(point);
    }

    KRYS_NODISCARD constexpr LayoutSize Distance(const LayoutPoint &point) const noexcept
    {
      return point - Clamp(point);
    }
  };

  TextStream &operator<<(TextStream &ts, const LayoutPoint &value) noexcept;
}
