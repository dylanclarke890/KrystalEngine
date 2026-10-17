#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include <algorithm>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class FloatRect;
  class LayoutRect;

  /// @brief An axis-aligned rectangle in integer coordinates.
  class IntRect
  {
  private:
    IntPoint _location;
    IntSize _size;

  public:
    constexpr IntRect() noexcept = default;
    constexpr IntRect(const IntPoint &location, const IntSize &size) noexcept
        : _location(location), _size(size)
    {
    }

    constexpr IntRect(int32 x, int32 y, int32 width, int32 height) noexcept
        : _location(IntPoint(x, y)), _size(IntSize(width, height))
    {
    }

    // Lossy, so not implicit.
    explicit IntRect(const FloatRect &rect) noexcept;
    explicit IntRect(const LayoutRect &rect) noexcept;

    KRYS_NODISCARD constexpr IntPoint Location() const noexcept
    {
      return _location;
    }

    KRYS_NODISCARD constexpr IntSize Size() const noexcept
    {
      return _size;
    }

    constexpr void SetLocation(const IntPoint &location) noexcept
    {
      _location = location;
    }

    constexpr void SetSize(const IntSize &size) noexcept
    {
      _size = size;
    }

    KRYS_NODISCARD constexpr int32 X() const noexcept
    {
      return _location.X();
    }

    KRYS_NODISCARD constexpr int32 Y() const noexcept
    {
      return _location.Y();
    }

    KRYS_NODISCARD constexpr int32 MaxX() const noexcept
    {
      return X() + Width();
    }

    KRYS_NODISCARD constexpr int32 MaxY() const noexcept
    {
      return Y() + Height();
    }

    KRYS_NODISCARD constexpr int32 Width() const noexcept
    {
      return _size.Width();
    }

    KRYS_NODISCARD constexpr int32 Height() const noexcept
    {
      return _size.Height();
    }

    constexpr void SetX(int32 x) noexcept
    {
      _location.SetX(x);
    }

    constexpr void SetY(int32 y) noexcept
    {
      _location.SetY(y);
    }

    constexpr void SetWidth(int32 width) noexcept
    {
      _size.SetWidth(width);
    }

    constexpr void SetHeight(int32 height) noexcept
    {
      _size.SetHeight(height);
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return _size.IsEmpty();
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return _size.IsZero();
    }

    // NOTE: The result is rounded to integer values, and thus may be not the exact
    // center point.
    KRYS_NODISCARD constexpr IntPoint Center() const noexcept
    {
      return IntPoint(X() + Width() / 2, Y() + Height() / 2);
    }

    constexpr void Move(const IntSize &size) noexcept
    {
      _location += size;
    }

    constexpr void MoveBy(const IntPoint &offset) noexcept
    {
      _location.Move(offset.X(), offset.Y());
    }

    constexpr void Move(int32 dx, int32 dy) noexcept
    {
      _location.Move(dx, dy);
    }

    constexpr void Expand(const IntSize &size) noexcept
    {
      _size += size;
    }

    constexpr void Expand(int32 dw, int32 dh) noexcept
    {
      _size.Expand(dw, dh);
    }

    constexpr void Contract(const IntSize &size) noexcept
    {
      _size -= size;
    }

    constexpr void Contract(int32 dw, int32 dh) noexcept
    {
      _size.Expand(-dw, -dh);
    }

    constexpr void ShiftXEdgeTo(int32 edge) noexcept
    {
      int32 delta = edge - X();
      SetX(edge);
      SetWidth(std::max(0, Width() - delta));
    }

    constexpr void ShiftMaxXEdgeTo(int32 edge) noexcept
    {
      int32 delta = edge - MaxX();
      SetWidth(std::max(0, Width() + delta));
    }

    constexpr void ShiftYEdgeTo(int32 edge) noexcept
    {
      int32 delta = edge - Y();
      SetY(edge);
      SetHeight(std::max(0, Height() - delta));
    }

    constexpr void ShiftMaxYEdgeTo(int32 edge) noexcept
    {
      int32 delta = edge - MaxY();
      SetHeight(std::max(0, Height() + delta));
    }

    constexpr void ShiftXEdgeBy(int32 delta) noexcept
    {
      Move(delta, 0);
      SetWidth(std::max(0, Width() - delta));
    }

    constexpr void ShiftMaxXEdgeBy(int32 delta) noexcept
    {
      SetWidth(std::max(0, Width() + delta));
    }

    constexpr void ShiftYEdgeBy(int32 delta) noexcept
    {
      Move(0, delta);
      SetHeight(std::max(0, Height() - delta));
    }

    constexpr void ShiftMaxYEdgeBy(int32 delta) noexcept
    {
      SetHeight(std::max(0, Height() + delta));
    }

    // Typically top left.
    KRYS_NODISCARD constexpr IntPoint MinXMinYCorner() const noexcept
    {
      return _location;
    }

    // Typically top right.
    KRYS_NODISCARD constexpr IntPoint MaxXMinYCorner() const noexcept
    {
      return IntPoint(_location.X() + _size.Width(), _location.Y());
    }

    // Typically bottom left.
    KRYS_NODISCARD constexpr IntPoint MinXMaxYCorner() const noexcept
    {
      return IntPoint(_location.X(), _location.Y() + _size.Height());
    }

    // Typically bottom right.
    KRYS_NODISCARD constexpr IntPoint MaxXMaxYCorner() const noexcept
    {
      return IntPoint(_location.X() + _size.Width(), _location.Y() + _size.Height());
    }

    KRYS_NODISCARD bool Intersects(const IntRect &other) const noexcept;
    KRYS_NODISCARD bool Contains(const IntRect &other) const noexcept;

    // This checks to see if the rect contains x,y in the traditional sense.
    // Equivalent to checking if the rect contains a 1x1 rect below and to the right of (px,py).
    KRYS_NODISCARD constexpr bool Contains(int32 px, int32 py) const noexcept
    {
      return px >= X() && px < MaxX() && py >= Y() && py < MaxY();
    }

    KRYS_NODISCARD constexpr bool Contains(const IntPoint &point) const noexcept
    {
      return Contains(point.X(), point.Y());
    }

    void Intersect(const IntRect &other) noexcept;
    void Unite(const IntRect &other) noexcept;
    void UniteIfNonZero(const IntRect &other) noexcept;

    constexpr void InflateX(int32 dx) noexcept
    {
      _location.SetX(_location.X() - dx);
      _size.SetWidth(_size.Width() + dx + dx);
    }

    constexpr void InflateY(int32 dy) noexcept
    {
      _location.SetY(_location.Y() - dy);
      _size.SetHeight(_size.Height() + dy + dy);
    }

    constexpr void Inflate(int32 d) noexcept
    {
      InflateX(d);
      InflateY(d);
    }

    constexpr void Inflate(IntSize size) noexcept
    {
      InflateX(size.Width());
      InflateY(size.Height());
    }

    void Scale(float s) noexcept;

    KRYS_NODISCARD IntSize DifferenceToPoint(const IntPoint &point) const noexcept;

    KRYS_NODISCARD int32 DistanceSquaredToPoint(const IntPoint &p) const noexcept
    {
      return DifferenceToPoint(p).DiagonalLengthSquared();
    }

    KRYS_NODISCARD constexpr IntRect TransposedRect() const noexcept
    {
      return IntRect(_location.TransposedPoint(), _size.TransposedSize());
    }

    /// @brief Whether `X() + Width()` and `Y() + Height()` fit in an `int32`.
    KRYS_NODISCARD bool IsValid() const noexcept;

    KRYS_NODISCARD IntRect ToRectWithExtentsClippedToNumericLimits() const noexcept;

    KRYS_NODISCARD friend constexpr bool operator==(const IntRect &, const IntRect &) noexcept = default;
  };

  KRYS_NODISCARD inline IntRect Intersection(const IntRect &a, const IntRect &b) noexcept
  {
    IntRect c = a;
    c.Intersect(b);
    return c;
  }

  KRYS_NODISCARD inline IntRect UnionRect(const IntRect &a, const IntRect &b) noexcept
  {
    IntRect c = a;
    c.Unite(b);
    return c;
  }

  constexpr IntRect &operator-=(IntRect &r, const IntPoint &offset) noexcept
  {
    r.Move(-offset.X(), -offset.Y());
    return r;
  }

  KRYS_NODISCARD constexpr IntRect operator-(const IntRect &r, const IntPoint &offset) noexcept
  {
    IntRect t = r;
    return t -= offset;
  }

  TextStream &operator<<(TextStream &ts, const IntRect &value) noexcept;
}
