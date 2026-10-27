#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include <cmath>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief A point (or vector) in 3D floating point coordinates.
  class FloatPoint3D
  {
  private:
    float _x {0};
    float _y {0};
    float _z {0};

  public:
    constexpr FloatPoint3D() noexcept = default;

    constexpr FloatPoint3D(float x, float y, float z) noexcept : _x(x), _y(y), _z(z)
    {
    }

    constexpr FloatPoint3D(const FloatPoint &p) noexcept : _x(p.X()), _y(p.Y())
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

    KRYS_NODISCARD constexpr FloatPoint XY() const noexcept
    {
      return {_x, _y};
    }

    constexpr void SetXY(FloatPoint p) noexcept
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

    KRYS_NODISCARD constexpr float Dot(const FloatPoint3D &a) const noexcept
    {
      return _x * a.X() + _y * a.Y() + _z * a.Z();
    }

    // Sets this FloatPoint3D to the cross product of the passed two.
    // It is safe for "this" to be the same as either or both of the
    // arguments.
    constexpr void Cross(const FloatPoint3D &a, const FloatPoint3D &b) noexcept
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
    KRYS_NODISCARD constexpr FloatPoint3D Cross(const FloatPoint3D &point) const noexcept
    {
      FloatPoint3D result;
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

    KRYS_NODISCARD float DistanceTo(const FloatPoint3D &a) const noexcept;

    KRYS_NODISCARD friend constexpr bool operator==(const FloatPoint3D &,
                                                    const FloatPoint3D &) noexcept = default;
  };

  constexpr FloatPoint3D &operator+=(FloatPoint3D &a, const FloatPoint3D &b) noexcept
  {
    a.Move(b.X(), b.Y(), b.Z());
    return a;
  }

  constexpr FloatPoint3D &operator+=(FloatPoint3D &a, const FloatPoint &b) noexcept
  {
    a.Move(b.X(), b.Y());
    return a;
  }

  constexpr FloatPoint3D &operator-=(FloatPoint3D &a, const FloatPoint3D &b) noexcept
  {
    a.Move(-b.X(), -b.Y(), -b.Z());
    return a;
  }

  constexpr FloatPoint3D &operator-=(FloatPoint3D &a, const FloatPoint &b) noexcept
  {
    a.Move(-b.X(), -b.Y());
    return a;
  }

  KRYS_NODISCARD constexpr FloatPoint3D operator+(const FloatPoint3D &a, const FloatPoint3D &b) noexcept
  {
    return FloatPoint3D(a.X() + b.X(), a.Y() + b.Y(), a.Z() + b.Z());
  }

  KRYS_NODISCARD constexpr FloatPoint3D operator+(const FloatPoint3D &a, const FloatPoint &b) noexcept
  {
    return FloatPoint3D(a.X() + b.X(), a.Y() + b.Y(), a.Z());
  }

  KRYS_NODISCARD constexpr FloatPoint3D operator-(const FloatPoint3D &a, const FloatPoint3D &b) noexcept
  {
    return FloatPoint3D(a.X() - b.X(), a.Y() - b.Y(), a.Z() - b.Z());
  }

  KRYS_NODISCARD constexpr FloatPoint3D operator-(const FloatPoint3D &a, const FloatPoint &b) noexcept
  {
    return FloatPoint3D(a.X() - b.X(), a.Y() - b.Y(), a.Z());
  }

  // Dot product.
  KRYS_NODISCARD constexpr float operator*(const FloatPoint3D &a, const FloatPoint3D &b) noexcept
  {
    return a.Dot(b);
  }

  KRYS_NODISCARD constexpr FloatPoint3D operator*(float k, const FloatPoint3D &v) noexcept
  {
    return FloatPoint3D(k * v.X(), k * v.Y(), k * v.Z());
  }

  KRYS_NODISCARD constexpr FloatPoint3D operator*(const FloatPoint3D &v, float k) noexcept
  {
    return FloatPoint3D(k * v.X(), k * v.Y(), k * v.Z());
  }

  inline float FloatPoint3D::DistanceTo(const FloatPoint3D &a) const noexcept
  {
    return (*this - a).Length();
  }

  TextStream &operator<<(TextStream &ts, const FloatPoint3D &point) noexcept;
}
