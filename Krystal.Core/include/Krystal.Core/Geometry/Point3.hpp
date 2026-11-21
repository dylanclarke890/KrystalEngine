/*
    Copyright (C) 2004, 2005, 2006 Nikolas Zimmermann <wildfox@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>
                  2005 Eric Seidel <eric@webkit.org>
                  2010 Zoltan Herczeg <zherczeg@webkit.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include <cmath>

namespace krys
{
  class TextStream;

  /// @brief A point (or vector) in 3D floating point coordinates.
  class Point3F
  {
  private:
    float _x {0};
    float _y {0};
    float _z {0};

  public:
    constexpr Point3F() noexcept = default;

    constexpr Point3F(float x, float y, float z) noexcept : _x(x), _y(y), _z(z)
    {
    }

    constexpr Point3F(const PointF &p) noexcept : _x(p.X()), _y(p.Y())
    {
    }

    KRYS_NODISCARD constexpr float X() const noexcept
    {
      return _x;
    }

    constexpr void SetX(float x) noexcept
    {
      _x = x;
    }

    KRYS_NODISCARD constexpr float Y() const noexcept
    {
      return _y;
    }

    constexpr void SetY(float y) noexcept
    {
      _y = y;
    }

    KRYS_NODISCARD constexpr PointF XY() const noexcept
    {
      return {_x, _y};
    }

    constexpr void SetXY(PointF p) noexcept
    {
      _x = p.X();
      _y = p.Y();
    }

    KRYS_NODISCARD constexpr float Z() const noexcept
    {
      return _z;
    }

    constexpr void SetZ(float z) noexcept
    {
      _z = z;
    }

    constexpr void Set(float x, float y, float z) noexcept
    {
      _x = x;
      _y = y;
      _z = z;
    }

    constexpr void Move(float dx, float dy, float dz = 0) noexcept
    {
      _x += dx;
      _y += dy;
      _z += dz;
    }

    constexpr void Scale(float sx, float sy, float sz) noexcept
    {
      _x *= sx;
      _y *= sy;
      _z *= sz;
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return !_x && !_y && !_z;
    }

    void Normalise() noexcept;

    KRYS_NODISCARD constexpr float Dot(const Point3F &a) const noexcept
    {
      return _x * a.X() + _y * a.Y() + _z * a.Z();
    }

    // Sets this Point3F to the cross product of the passed two.
    // It is safe for "this" to be the same as either or both of the
    // arguments.
    constexpr void Cross(const Point3F &a, const Point3F &b) noexcept
    {
      float x = a.Y() * b.Z() - a.Z() * b.Y();
      float y = a.Z() * b.X() - a.X() * b.Z();
      float z = a.X() * b.Y() - a.Y() * b.X();
      _x = x;
      _y = y;
      _z = z;
    }

    // Convenience function returning "this cross point" as a
    // stack-allocated result.
    KRYS_NODISCARD constexpr Point3F Cross(const Point3F &point) const noexcept
    {
      Point3F result;
      result.Cross(*this, point);
      return result;
    }

    KRYS_NODISCARD constexpr float LengthSquared() const noexcept
    {
      return Dot(*this);
    }

    KRYS_NODISCARD float Length() const noexcept
    {
      return std::hypot(_x, _y, _z);
    }

    KRYS_NODISCARD float DistanceTo(const Point3F &a) const noexcept;

    KRYS_NODISCARD friend constexpr bool operator==(const Point3F &, const Point3F &) noexcept = default;
  };

  constexpr Point3F &operator+=(Point3F &a, const Point3F &b) noexcept
  {
    a.Move(b.X(), b.Y(), b.Z());
    return a;
  }

  constexpr Point3F &operator+=(Point3F &a, const PointF &b) noexcept
  {
    a.Move(b.X(), b.Y());
    return a;
  }

  constexpr Point3F &operator-=(Point3F &a, const Point3F &b) noexcept
  {
    a.Move(-b.X(), -b.Y(), -b.Z());
    return a;
  }

  constexpr Point3F &operator-=(Point3F &a, const PointF &b) noexcept
  {
    a.Move(-b.X(), -b.Y());
    return a;
  }

  KRYS_NODISCARD constexpr Point3F operator+(const Point3F &a, const Point3F &b) noexcept
  {
    return Point3F(a.X() + b.X(), a.Y() + b.Y(), a.Z() + b.Z());
  }

  KRYS_NODISCARD constexpr Point3F operator+(const Point3F &a, const PointF &b) noexcept
  {
    return Point3F(a.X() + b.X(), a.Y() + b.Y(), a.Z());
  }

  KRYS_NODISCARD constexpr Point3F operator-(const Point3F &a, const Point3F &b) noexcept
  {
    return Point3F(a.X() - b.X(), a.Y() - b.Y(), a.Z() - b.Z());
  }

  KRYS_NODISCARD constexpr Point3F operator-(const Point3F &a, const PointF &b) noexcept
  {
    return Point3F(a.X() - b.X(), a.Y() - b.Y(), a.Z());
  }

  // Dot product.
  KRYS_NODISCARD constexpr float operator*(const Point3F &a, const Point3F &b) noexcept
  {
    return a.Dot(b);
  }

  KRYS_NODISCARD constexpr Point3F operator*(float k, const Point3F &v) noexcept
  {
    return Point3F(k * v.X(), k * v.Y(), k * v.Z());
  }

  KRYS_NODISCARD constexpr Point3F operator*(const Point3F &v, float k) noexcept
  {
    return Point3F(k * v.X(), k * v.Y(), k * v.Z());
  }

  inline float Point3F::DistanceTo(const Point3F &a) const noexcept
  {
    return (*this - a).Length();
  }

  TextStream &operator<<(TextStream &ts, const Point3F &point) noexcept;
}
