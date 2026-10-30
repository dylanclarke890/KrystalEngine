#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include <algorithm>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class IntRect;

  /// @brief An axis-aligned rectangle in floating point coordinates.
  class FloatRect
  {
  private:
    FloatPoint _location;
    FloatSize _size;

  public:
    enum class ContainsMode : uint8
    {
      InsideOrOnStroke,
      InsideButNotOnStroke
    };

    constexpr FloatRect() noexcept = default;
    constexpr FloatRect(const FloatPoint &location, const FloatSize &size) noexcept
        : _location(location), _size(size)
    {
    }

    constexpr FloatRect(float x, float y, float width, float height) noexcept
        : _location(FloatPoint(x, y)), _size(FloatSize(width, height))
    {
    }

    constexpr FloatRect(const FloatPoint &topLeft, const FloatPoint &bottomRight) noexcept
        : _location(topLeft), _size(FloatSize(bottomRight.X() - topLeft.X(), bottomRight.Y() - topLeft.Y()))
    {
    }

    FloatRect(const IntRect &rect) noexcept;

    KRYS_NODISCARD static FloatRect NarrowPrecision(double x, double y, double width, double height) noexcept;

    KRYS_NODISCARD constexpr FloatPoint Location() const noexcept
    {
      return _location;
    }

    KRYS_NODISCARD constexpr FloatSize Size() const noexcept
    {
      return _size;
    }

    constexpr void SetLocation(const FloatPoint &location) noexcept
    {
      _location = location;
    }

    constexpr void SetSize(const FloatSize &size) noexcept
    {
      _size = size;
    }

    KRYS_NODISCARD constexpr float X() const noexcept
    {
      return _location.X();
    }

    KRYS_NODISCARD constexpr float Y() const noexcept
    {
      return _location.Y();
    }

    KRYS_NODISCARD constexpr float MaxX() const noexcept
    {
      return X() + Width();
    }

    KRYS_NODISCARD constexpr float MaxY() const noexcept
    {
      return Y() + Height();
    }

    KRYS_NODISCARD constexpr float Width() const noexcept
    {
      return _size.Width();
    }

    KRYS_NODISCARD constexpr float Height() const noexcept
    {
      return _size.Height();
    }

    KRYS_NODISCARD constexpr float Area() const noexcept
    {
      return _size.Area();
    }

    constexpr void SetX(float x) noexcept
    {
      _location.SetX(x);
    }

    constexpr void SetY(float y) noexcept
    {
      _location.SetY(y);
    }

    constexpr void SetWidth(float width) noexcept
    {
      _size.SetWidth(width);
    }

    constexpr void SetHeight(float height) noexcept
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

    KRYS_NODISCARD bool IsExpressibleAsIntRect() const noexcept;

    KRYS_NODISCARD constexpr FloatPoint Center() const noexcept
    {
      return Location() + Size() / 2;
    }

    constexpr void Move(const FloatSize &delta) noexcept
    {
      _location += delta;
    }

    constexpr void MoveBy(const FloatPoint &delta) noexcept
    {
      _location.Move(delta.X(), delta.Y());
    }

    constexpr void Move(float dx, float dy) noexcept
    {
      _location.Move(dx, dy);
    }

    constexpr void Expand(const FloatSize &size) noexcept
    {
      _size += size;
    }

    void Expand(const FloatBoxExtent &box) noexcept
    {
      _location.Move(-box.Left(), -box.Top());
      _size.Expand(box.Left() + box.Right(), box.Top() + box.Bottom());
    }

    constexpr void Expand(float dw, float dh) noexcept
    {
      _size.Expand(dw, dh);
    }

    constexpr void Contract(const FloatSize &size) noexcept
    {
      _size -= size;
    }

    void Contract(const FloatBoxExtent &box) noexcept
    {
      _location.Move(box.Left(), box.Top());
      _size.Expand(-(box.Left() + box.Right()), -(box.Top() + box.Bottom()));
    }

    constexpr void Contract(float dw, float dh) noexcept
    {
      _size.Expand(-dw, -dh);
    }

    constexpr void ShiftEdgesTo(float left, float top, float right, float bottom) noexcept
    {
      _location.Set(left, top);
      _size.SetWidth(right - left);
      _size.SetHeight(bottom - top);
    }

    constexpr void ShiftXEdgeTo(float edge) noexcept
    {
      float delta = edge - X();
      SetX(edge);
      SetWidth(std::max(0.0f, Width() - delta));
    }

    constexpr void ShiftMaxXEdgeTo(float edge) noexcept
    {
      float delta = edge - MaxX();
      SetWidth(std::max(0.0f, Width() + delta));
    }

    constexpr void ShiftYEdgeTo(float edge) noexcept
    {
      float delta = edge - Y();
      SetY(edge);
      SetHeight(std::max(0.0f, Height() - delta));
    }

    constexpr void ShiftMaxYEdgeTo(float edge) noexcept
    {
      float delta = edge - MaxY();
      SetHeight(std::max(0.0f, Height() + delta));
    }

    constexpr void ShiftXEdgeBy(float delta) noexcept
    {
      Move(delta, 0);
      SetWidth(std::max(0.0f, Width() - delta));
    }

    constexpr void ShiftMaxXEdgeBy(float delta) noexcept
    {
      ShiftMaxXEdgeTo(MaxX() + delta);
    }

    constexpr void ShiftYEdgeBy(float delta) noexcept
    {
      Move(0, delta);
      SetHeight(std::max(0.0f, Height() - delta));
    }

    constexpr void ShiftMaxYEdgeBy(float delta) noexcept
    {
      ShiftMaxYEdgeTo(MaxY() + delta);
    }

    // Typically top left.
    KRYS_NODISCARD constexpr FloatPoint MinXMinYCorner() const noexcept
    {
      return _location;
    }

    // Typically top right.
    KRYS_NODISCARD constexpr FloatPoint MaxXMinYCorner() const noexcept
    {
      return FloatPoint(_location.X() + _size.Width(), _location.Y());
    }

    // Typically bottom left.
    KRYS_NODISCARD constexpr FloatPoint MinXMaxYCorner() const noexcept
    {
      return FloatPoint(_location.X(), _location.Y() + _size.Height());
    }

    // Typically bottom right.
    KRYS_NODISCARD constexpr FloatPoint MaxXMaxYCorner() const noexcept
    {
      return FloatPoint(_location.X() + _size.Width(), _location.Y() + _size.Height());
    }

    KRYS_NODISCARD bool Intersects(const FloatRect &other) const noexcept;
    KRYS_NODISCARD bool InclusivelyIntersects(const FloatRect &other) const noexcept;
    KRYS_NODISCARD bool Contains(const FloatRect &other) const noexcept;
    KRYS_NODISCARD bool Contains(const FloatPoint &point,
                                 ContainsMode containsMode = ContainsMode::InsideOrOnStroke) const noexcept;

    void Intersect(const FloatRect &other) noexcept;
    bool EdgeInclusiveIntersect(const FloatRect &other) noexcept;
    void Unite(const FloatRect &other) noexcept;
    void UniteEvenIfEmpty(const FloatRect &other) noexcept;
    void UniteIfNonZero(const FloatRect &other) noexcept;
    void Extend(FloatPoint point) noexcept;
    void Extend(FloatPoint minPoint, FloatPoint maxPoint) noexcept;

    // Note, this doesn't match what IntRect::contains(IntPoint&) does; the int version
    // is really checking for containment of 1x1 rect, but that doesn't make sense with floats.
    KRYS_NODISCARD constexpr bool Contains(float px, float py) const noexcept
    {
      return px >= X() && px <= MaxX() && py >= Y() && py <= MaxY();
    }

    KRYS_NODISCARD constexpr bool OverlapsYRange(float y1, float y2) const noexcept
    {
      return !IsEmpty() && y2 >= y1 && y2 >= Y() && y1 <= MaxY();
    }

    KRYS_NODISCARD constexpr bool OverlapsXRange(float x1, float x2) const noexcept
    {
      return !IsEmpty() && x2 >= x1 && x2 >= X() && x1 <= MaxX();
    }

    constexpr void InflateX(float dx) noexcept
    {
      _location.SetX(_location.X() - dx);
      _size.SetWidth(_size.Width() + dx + dx);
    }

    constexpr void InflateY(float dy) noexcept
    {
      _location.SetY(_location.Y() - dy);
      _size.SetHeight(_size.Height() + dy + dy);
    }

    constexpr void Inflate(float d) noexcept
    {
      InflateX(d);
      InflateY(d);
    }

    constexpr void Inflate(FloatSize size) noexcept
    {
      InflateX(size.Width());
      InflateY(size.Height());
    }

    constexpr void Inflate(float deltaX, float deltaY, float deltaMaxX, float deltaMaxY) noexcept
    {
      SetX(X() - deltaX);
      SetY(Y() - deltaY);
      SetWidth(Width() + deltaX + deltaMaxX);
      SetHeight(Height() + deltaY + deltaMaxY);
    }

    constexpr void Scale(float s) noexcept
    {
      Scale(s, s);
    }

    constexpr void Scale(float sx, float sy) noexcept
    {
      _location.SetX(X() * sx);
      _location.SetY(Y() * sy);
      _size.SetWidth(Width() * sx);
      _size.SetHeight(Height() * sy);
    }

    constexpr void Scale(FloatSize size) noexcept
    {
      Scale(size.Width(), size.Height());
    }

    KRYS_NODISCARD constexpr FloatRect TransposedRect() const noexcept
    {
      return FloatRect(_location.TransposedPoint(), _size.TransposedSize());
    }

    KRYS_NODISCARD constexpr static FloatRect InfiniteRect() noexcept
    {
      return {-Limits<float>::max() / 2, -Limits<float>::max() / 2, Limits<float>::max(),
              Limits<float>::max()};
    }

    KRYS_NODISCARD constexpr bool IsInfinite() const noexcept
    {
      return *this == InfiniteRect();
    }

    KRYS_NODISCARD constexpr static FloatRect SmallestRect() noexcept
    {
      return {Limits<float>::max() / 2, Limits<float>::max() / 2, -Limits<float>::max(),
              -Limits<float>::max()};
    }

    KRYS_NODISCARD constexpr bool IsSmallest() const noexcept
    {
      return *this == SmallestRect();
    }

    KRYS_NODISCARD constexpr static FloatRect NanRect() noexcept
    {
      return {Limits<float>::quiet_NaN(), Limits<float>::quiet_NaN(), Limits<float>::quiet_NaN(),
              Limits<float>::quiet_NaN()};
    }

    KRYS_NODISCARD constexpr bool IsNaN() const noexcept
    {
      // NaN is the only value that isn't equal to itself.
      return X() != X() || Y() != Y();
    }

    KRYS_NODISCARD friend constexpr bool operator==(const FloatRect &, const FloatRect &) noexcept = default;
  };

  KRYS_NODISCARD inline FloatRect Intersection(const FloatRect &a, const FloatRect &b) noexcept
  {
    FloatRect c = a;
    c.Intersect(b);
    return c;
  }

  KRYS_NODISCARD inline FloatRect UnionRect(const FloatRect &a, const FloatRect &b) noexcept
  {
    FloatRect c = a;
    c.Unite(b);
    return c;
  }

  constexpr FloatRect &operator+=(FloatRect &a, const FloatRect &b) noexcept
  {
    a.Move(b.X(), b.Y());
    a.SetWidth(a.Width() + b.Width());
    a.SetHeight(a.Height() + b.Height());
    return a;
  }

  KRYS_NODISCARD constexpr FloatRect operator+(const FloatRect &a, const FloatRect &b) noexcept
  {
    return FloatRect {
      a.X() + b.X(),
      a.Y() + b.Y(),
      a.Width() + b.Width(),
      a.Height() + b.Height(),
    };
  }

  KRYS_NODISCARD inline FloatRect operator+(const FloatRect &a, const FloatBoxExtent &b) noexcept
  {
    FloatRect c = a;
    c.Expand(b);
    return c;
  }

  KRYS_NODISCARD inline bool AreEssentiallyEqual(const FloatRect &a, const FloatRect &b) noexcept
  {
    return AreEssentiallyEqual(a.Location(), b.Location()) && AreEssentiallyEqual(a.Size(), b.Size());
  }

  KRYS_NODISCARD FloatRect NormaliseRect(const FloatRect &rect) noexcept;
  KRYS_NODISCARD FloatRect EncloseRectToDevicePixels(const FloatRect &rect, float deviceScaleFactor) noexcept;
  KRYS_NODISCARD IntRect EnclosingIntRect(const FloatRect &rect) noexcept;
  KRYS_NODISCARD IntRect EnclosingIntRectPreservingEmptyRects(const FloatRect &rect) noexcept;
  KRYS_NODISCARD IntRect RoundedIntRect(const FloatRect &rect) noexcept;

  TextStream &operator<<(TextStream &ts, const FloatRect &value) noexcept;
}

namespace krys
{
  template <>
  struct MarkableTraits<boo::FloatRect>
  {
    KRYS_NODISCARD constexpr static bool IsEmptyValue(const boo::FloatRect &rect) noexcept
    {
      return rect.IsNaN();
    }

    KRYS_NODISCARD constexpr static boo::FloatRect EmptyValue() noexcept
    {
      return boo::FloatRect::NanRect();
    }
  };
}
