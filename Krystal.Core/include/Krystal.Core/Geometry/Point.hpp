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
/*
 * Copyright (C) 2004-2016 Apple Inc. All rights reserved.
 * Copyright (C) 2005 Nokia. All rights reserved.
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
/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Geometry/GeometryForward.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include <algorithm>
#include <cmath>

namespace krys
{
  class TextStream;

  /// @brief A point.
  template <GeometryCoordinate T>
  class Point
  {
  private:
    using ScaleType = detail::GeometryScale<T>;

    T _x {0};
    T _y {0};

  public:
    constexpr Point() noexcept = default;

    constexpr Point(T x, T y) noexcept : _x(x), _y(y)
    {
    }

    constexpr explicit Point(const Size<T> &size) noexcept : _x(size.Width()), _y(size.Height())
    {
    }

    /// @brief Converts from another coordinate type: implicitly when no value is lost, explicitly (clamping
    /// to the integer range) otherwise.
    template <GeometryCoordinate U>
    requires(!SameType<T, U>)
    constexpr explicit(!detail::IsWideningGeometryConversion<U, T>) Point(const Point<U> &point) noexcept
        : _x(detail::ConvertGeometryValue<T>(point.X())), _y(detail::ConvertGeometryValue<T>(point.Y()))
    {
    }

    KRYS_NODISCARD constexpr static Point Zero() noexcept
    {
      return Point();
    }

    KRYS_NODISCARD constexpr static Point NarrowPrecision(double x, double y) noexcept
    requires SameType<T, float>
    {
      return Point(static_cast<float>(x), static_cast<float>(y));
    }

    KRYS_NODISCARD constexpr static Point NanPoint() noexcept
    requires FloatingPoint<T>
    {
      return {Limits<T>::quiet_NaN(), Limits<T>::quiet_NaN()};
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return !_x && !_y;
    }

    KRYS_NODISCARD constexpr bool IsNaN() const noexcept
    requires FloatingPoint<T>
    {
      return krys::IsNaN(_x);
    }

    KRYS_NODISCARD constexpr T X() const noexcept
    {
      return _x;
    }

    KRYS_NODISCARD constexpr T Y() const noexcept
    {
      return _y;
    }

    constexpr void SetX(T x) noexcept
    {
      _x = x;
    }

    constexpr void SetY(T y) noexcept
    {
      _y = y;
    }

    constexpr void Set(T x, T y) noexcept
    {
      _x = x;
      _y = y;
    }

    constexpr void Move(T dx, T dy) noexcept
    {
      _x += dx;
      _y += dy;
    }

    constexpr void Move(const Size<T> &offset) noexcept
    {
      Move(offset.Width(), offset.Height());
    }

    constexpr void MoveBy(const Point &offset) noexcept
    {
      Move(offset._x, offset._y);
    }

    /// @brief Scales each coordinate; integer coordinates are rounded.
    constexpr void Scale(ScaleType scaleX, ScaleType scaleY) noexcept
    {
      *this = Scaled(scaleX, scaleY);
    }

    constexpr void Scale(ScaleType scale) noexcept
    {
      Scale(scale, scale);
    }

    /// @brief This scaled on each axis; integer coordinates are rounded.
    KRYS_NODISCARD constexpr Point Scaled(ScaleType scaleX, ScaleType scaleY) const noexcept
    {
      if constexpr (Integral<T>)
      {
        return {static_cast<T>(std::lround(static_cast<ScaleType>(_x) * scaleX)),
                static_cast<T>(std::lround(static_cast<ScaleType>(_y) * scaleY))};
      }
      else
      {
        return {static_cast<T>(_x * scaleX), static_cast<T>(_y * scaleY)};
      }
    }

    KRYS_NODISCARD constexpr Point Scaled(ScaleType scale) const noexcept
    {
      return Scaled(scale, scale);
    }

    KRYS_NODISCARD constexpr Point ScaledBy(ScaleType scale) const noexcept
    {
      return Scaled(scale, scale);
    }

    void Rotate(double angleInRadians, const Point &aboutPoint) noexcept
    requires FloatingPoint<T>
    {
      auto sinAngle = std::sin(angleInRadians);
      auto cosAngle = std::cos(angleInRadians);
      _x -= aboutPoint._x;
      _y -= aboutPoint._y;
      auto newX = _x * cosAngle - _y * sinAngle + aboutPoint._x;
      _y = static_cast<T>(_x * sinAngle + _y * cosAngle + aboutPoint._y);
      _x = static_cast<T>(newX);
    }

    // An overload, not a default argument: a default argument can't use the member initialisers in here.
    void Rotate(double angleInRadians) noexcept
    requires FloatingPoint<T>
    {
      Rotate(angleInRadians, Point());
    }

    void Normalise() noexcept
    requires FloatingPoint<T>
    {
      T length = Length();
      if (length)
      {
        _x /= length;
        _y /= length;
      }
    }

    KRYS_NODISCARD constexpr T Dot(const Point &other) const noexcept
    {
      return _x * other._x + _y * other._y;
    }

    KRYS_NODISCARD T SlopeAngleRadians() const noexcept
    requires FloatingPoint<T>
    {
      return std::atan2(_y, _x);
    }

    KRYS_NODISCARD T Length() const noexcept
    requires FloatingPoint<T>
    {
      return std::hypot(_x, _y);
    }

    KRYS_NODISCARD constexpr T LengthSquared() const noexcept
    {
      return _x * _x + _y * _y;
    }

    KRYS_NODISCARD constexpr T DistanceSquaredToPoint(const Point &point) const noexcept
    {
      return (*this - point).DiagonalLengthSquared();
    }

    KRYS_NODISCARD constexpr Point ExpandedTo(const Point &other) const noexcept
    {
      return {std::max(_x, other._x), std::max(_y, other._y)};
    }

    KRYS_NODISCARD constexpr Point ShrunkTo(const Point &other) const noexcept
    {
      return {std::min(_x, other._x), std::min(_y, other._y)};
    }

    KRYS_NODISCARD constexpr Point ConstrainedBetween(const Point &min, const Point &max) const noexcept
    {
      return {std::max(min._x, std::min(max._x, _x)), std::max(min._y, std::min(max._y, _y))};
    }

    /// @brief This moved into @p rect (from its minimum to its maximum corner, inclusive).
    /// @note Defined in `Rect.hpp`.
    KRYS_NODISCARD constexpr Point ConstrainedWithin(const Rect<T> &rect) const noexcept;

    constexpr void ClampNegativeToZero() noexcept
    {
      *this = ExpandedTo(Zero());
    }

    KRYS_NODISCARD constexpr Point TransposedPoint() const noexcept
    {
      return {_y, _x};
    }

    KRYS_NODISCARD friend constexpr bool operator==(const Point &, const Point &) noexcept = default;

    friend constexpr Point &operator+=(Point &a, const Size<T> &b) noexcept
    {
      a.Move(b.Width(), b.Height());
      return a;
    }

    friend constexpr Point &operator-=(Point &a, const Size<T> &b) noexcept
    {
      a.Move(-b.Width(), -b.Height());
      return a;
    }

    friend constexpr Point &operator+=(Point &a, const Point &b) noexcept
    {
      a.Move(b._x, b._y);
      return a;
    }

    KRYS_NODISCARD friend constexpr Point operator+(const Point &a, const Size<T> &b) noexcept
    {
      return {a._x + b.Width(), a._y + b.Height()};
    }

    KRYS_NODISCARD friend constexpr Point operator+(const Point &a, const Point &b) noexcept
    {
      return {a._x + b._x, a._y + b._y};
    }

    KRYS_NODISCARD friend constexpr Size<T> operator-(const Point &a, const Point &b) noexcept
    {
      return {a._x - b._x, a._y - b._y};
    }

    KRYS_NODISCARD friend constexpr Point operator-(const Point &a, const Size<T> &b) noexcept
    {
      return {a._x - b.Width(), a._y - b.Height()};
    }

    KRYS_NODISCARD friend constexpr Point operator-(const Point &point) noexcept
    {
      return {-point._x, -point._y};
    }
  };

#pragma region Integer points

  KRYS_NODISCARD constexpr SizeI ToSizeI(const PointI &point) noexcept
  {
    return {point.X(), point.Y()};
  }

#pragma endregion

#pragma region Floating point points

  /// @brief The dot product of @p a and @p b.
  KRYS_NODISCARD constexpr float operator*(const PointF &a, const PointF &b) noexcept
  {
    return a.Dot(b);
  }

  KRYS_NODISCARD inline SizeI FlooredSizeI(const PointF &point) noexcept
  {
    return {ClampToInteger(std::floor(point.X())), ClampToInteger(std::floor(point.Y()))};
  }

  KRYS_NODISCARD inline PointI RoundedPointI(const PointF &point) noexcept
  {
    return {ClampToInteger(std::round(point.X())), ClampToInteger(std::round(point.Y()))};
  }

  KRYS_NODISCARD inline PointI FlooredPointI(const PointF &point) noexcept
  {
    return {ClampToInteger(std::floor(point.X())), ClampToInteger(std::floor(point.Y()))};
  }

  KRYS_NODISCARD inline PointI FlooredPointI(const SizeF &size) noexcept
  {
    return {ClampToInteger(std::floor(size.Width())), ClampToInteger(std::floor(size.Height()))};
  }

  KRYS_NODISCARD inline PointI CeiledPointI(const PointF &point) noexcept
  {
    return {ClampToInteger(std::ceil(point.X())), ClampToInteger(std::ceil(point.Y()))};
  }

  KRYS_NODISCARD inline PointF FloorPointToDevicePixels(const PointF &point, float deviceScaleFactor) noexcept
  {
    return {std::floor(point.X() * deviceScaleFactor) / deviceScaleFactor,
            std::floor(point.Y() * deviceScaleFactor) / deviceScaleFactor};
  }

  KRYS_NODISCARD inline PointF CeilPointToDevicePixels(const PointF &point, float deviceScaleFactor) noexcept
  {
    return {std::ceil(point.X() * deviceScaleFactor) / deviceScaleFactor,
            std::ceil(point.Y() * deviceScaleFactor) / deviceScaleFactor};
  }

  KRYS_NODISCARD constexpr SizeF ToSizeF(const PointF &point) noexcept
  {
    return {point.X(), point.Y()};
  }

  KRYS_NODISCARD constexpr PointF ToPointF(const SizeF &size) noexcept
  {
    return {size.Width(), size.Height()};
  }

  KRYS_NODISCARD constexpr bool AreEssentiallyEqual(const PointF &a, const PointF &b) noexcept
  {
    return AreEssentiallyEqual(a.X(), b.X()) && AreEssentiallyEqual(a.Y(), b.Y());
  }

  KRYS_NODISCARD inline PointI FlooredPointI(const PointD &point) noexcept
  {
    return {ClampToInteger(std::floor(point.X())), ClampToInteger(std::floor(point.Y()))};
  }

  KRYS_NODISCARD inline PointI RoundedPointI(const PointD &point) noexcept
  {
    return {ClampToInteger(std::round(point.X())), ClampToInteger(std::round(point.Y()))};
  }

  KRYS_NODISCARD constexpr SizeD ToSizeD(const PointD &point) noexcept
  {
    return {point.X(), point.Y()};
  }

#pragma endregion

  TextStream &operator<<(TextStream &ts, const PointI &value) noexcept;

  TextStream &operator<<(TextStream &ts, const PointF &value) noexcept;

  template <>
  struct DefaultHash<PointF>
  {
    KRYS_NODISCARD static size_t Hash(const PointF &point) noexcept
    {
      return krys::Hash::Combine(point.X(), point.Y());
    }

    KRYS_NODISCARD static bool Equal(const PointF &a, const PointF &b) noexcept
    {
      return a == b;
    }
  };

  template <>
  struct MarkableTraits<PointF>
  {
    KRYS_NODISCARD constexpr static bool IsEmptyValue(const PointF &point) noexcept
    {
      return point.IsNaN();
    }

    KRYS_NODISCARD constexpr static PointF EmptyValue() noexcept
    {
      return PointF::NanPoint();
    }
  };
}
