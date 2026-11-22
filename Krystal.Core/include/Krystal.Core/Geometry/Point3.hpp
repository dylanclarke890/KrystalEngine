#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include <cmath>

namespace krys
{
  class TextStream;

  /// @brief A point, or a vector from the origin, in 3D floating-point coordinates.
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

    /// @brief @p point on the plane z = 0.
    constexpr Point3F(const PointF &point) noexcept : _x(point.X()), _y(point.Y())
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

    KRYS_NODISCARD constexpr float Z() const noexcept
    {
      return _z;
    }

    constexpr void SetZ(float z) noexcept
    {
      _z = z;
    }

    /// @brief The point projected onto the plane z = 0.
    KRYS_NODISCARD constexpr PointF XY() const noexcept
    {
      return {_x, _y};
    }

    constexpr void SetXY(PointF point) noexcept
    {
      _x = point.X();
      _y = point.Y();
    }

    constexpr void Set(float x, float y, float z) noexcept
    {
      *this = {x, y, z};
    }

    /// @brief Offsets the point by (@p dx, @p dy, @p dz).
    constexpr void Move(float dx, float dy, float dz = 0) noexcept
    {
      *this = {_x + dx, _y + dy, _z + dz};
    }

    /// @brief Multiplies each coordinate by its factor.
    constexpr void Scale(float sx, float sy, float sz) noexcept
    {
      *this = {_x * sx, _y * sy, _z * sz};
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return _x == 0 && _y == 0 && _z == 0;
    }

    /// @brief Scales the vector to length 1; the zero vector stays as it is.
    void Normalise() noexcept;

    KRYS_NODISCARD constexpr float Dot(const Point3F &other) const noexcept
    {
      return _x * other._x + _y * other._y + _z * other._z;
    }

    /// @brief The vector perpendicular to this one and @p other, following the right-hand rule.
    KRYS_NODISCARD constexpr Point3F Cross(const Point3F &other) const noexcept
    {
      return {_y * other._z - _z * other._y, _z * other._x - _x * other._z, _x * other._y - _y * other._x};
    }

    KRYS_NODISCARD constexpr float LengthSquared() const noexcept
    {
      return Dot(*this);
    }

    KRYS_NODISCARD float Length() const noexcept
    {
      return std::hypot(_x, _y, _z);
    }

    KRYS_NODISCARD float DistanceTo(const Point3F &other) const noexcept;

    KRYS_NODISCARD friend constexpr bool operator==(const Point3F &, const Point3F &) noexcept = default;
  };

  KRYS_NODISCARD constexpr Point3F operator+(const Point3F &a, const Point3F &b) noexcept
  {
    return {a.X() + b.X(), a.Y() + b.Y(), a.Z() + b.Z()};
  }

  KRYS_NODISCARD constexpr Point3F operator-(const Point3F &a, const Point3F &b) noexcept
  {
    return {a.X() - b.X(), a.Y() - b.Y(), a.Z() - b.Z()};
  }

  /// @brief @p a offset by @p b in x and y.
  KRYS_NODISCARD constexpr Point3F operator+(const Point3F &a, const PointF &b) noexcept
  {
    return {a.X() + b.X(), a.Y() + b.Y(), a.Z()};
  }

  /// @brief @p a offset by minus @p b in x and y.
  KRYS_NODISCARD constexpr Point3F operator-(const Point3F &a, const PointF &b) noexcept
  {
    return {a.X() - b.X(), a.Y() - b.Y(), a.Z()};
  }

  constexpr Point3F &operator+=(Point3F &a, const Point3F &b) noexcept
  {
    return a = a + b;
  }

  constexpr Point3F &operator-=(Point3F &a, const Point3F &b) noexcept
  {
    return a = a - b;
  }

  constexpr Point3F &operator+=(Point3F &a, const PointF &b) noexcept
  {
    return a = a + b;
  }

  constexpr Point3F &operator-=(Point3F &a, const PointF &b) noexcept
  {
    return a = a - b;
  }

  /// @brief The dot product of @p a and @p b.
  KRYS_NODISCARD constexpr float operator*(const Point3F &a, const Point3F &b) noexcept
  {
    return a.Dot(b);
  }

  KRYS_NODISCARD constexpr Point3F operator*(float factor, const Point3F &point) noexcept
  {
    return {factor * point.X(), factor * point.Y(), factor * point.Z()};
  }

  KRYS_NODISCARD constexpr Point3F operator*(const Point3F &point, float factor) noexcept
  {
    return factor * point;
  }

  inline float Point3F::DistanceTo(const Point3F &other) const noexcept
  {
    return (*this - other).Length();
  }

  TextStream &operator<<(TextStream &ts, const Point3F &point) noexcept;
}
