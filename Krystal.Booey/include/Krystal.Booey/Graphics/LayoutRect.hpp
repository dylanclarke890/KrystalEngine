#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <algorithm>

namespace krys::boo
{
  /// @brief An axis-aligned rectangle in layout units.
  class LayoutRect
  {
  private:
    LayoutPoint _location;
    LayoutSize _size;

  public:
    constexpr LayoutRect() noexcept = default;
    constexpr LayoutRect(const LayoutPoint &location, const LayoutSize &size) noexcept
        : _location(location), _size(size)
    {
    }

    template <typename T1, typename T2, typename U1, typename U2>
    constexpr LayoutRect(T1 x, T2 y, U1 width, U2 height) noexcept
        : _location(LayoutPoint(x, y)), _size(LayoutSize(width, height))
    {
    }

    constexpr LayoutRect(const LayoutPoint &topLeft, const LayoutPoint &bottomRight) noexcept
        : _location(topLeft), _size(LayoutSize(bottomRight.X() - topLeft.X(), bottomRight.Y() - topLeft.Y()))
    {
    }

    constexpr LayoutRect(const FloatPoint &location, const FloatSize &size) noexcept
        : _location(location), _size(size)
    {
    }

    constexpr LayoutRect(const IntRect &rect) noexcept : _location(rect.Location()), _size(rect.Size())
    {
    }

    // Lossy, so not implicit.
    explicit LayoutRect(const FloatRect &rect) noexcept;

    KRYS_NODISCARD constexpr LayoutPoint Location() const noexcept
    {
      return _location;
    }

    KRYS_NODISCARD constexpr LayoutSize Size() const noexcept
    {
      return _size;
    }

    constexpr void SetLocation(const LayoutPoint &location) noexcept
    {
      _location = location;
    }

    constexpr void SetSize(const LayoutSize &size) noexcept
    {
      _size = size;
    }

    KRYS_NODISCARD constexpr LayoutUnit X() const noexcept
    {
      return _location.X();
    }

    KRYS_NODISCARD constexpr LayoutUnit Y() const noexcept
    {
      return _location.Y();
    }

    KRYS_NODISCARD constexpr LayoutUnit MaxX() const noexcept
    {
      return X() + Width();
    }

    KRYS_NODISCARD constexpr LayoutUnit MaxY() const noexcept
    {
      return Y() + Height();
    }

    KRYS_NODISCARD constexpr LayoutUnit Width() const noexcept
    {
      return _size.Width();
    }

    KRYS_NODISCARD constexpr LayoutUnit Height() const noexcept
    {
      return _size.Height();
    }

    template <typename T>
    constexpr void SetX(T x) noexcept
    {
      _location.SetX(x);
    }

    template <typename T>
    constexpr void SetY(T y) noexcept
    {
      _location.SetY(y);
    }

    template <typename T>
    constexpr void SetWidth(T width) noexcept
    {
      _size.SetWidth(width);
    }

    template <typename T>
    constexpr void SetHeight(T height) noexcept
    {
      _size.SetHeight(height);
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return _size.IsEmpty();
    }

    // NOTE: The result is rounded to integer values, and thus may be not the exact
    // center point.
    KRYS_NODISCARD constexpr LayoutPoint Center() const noexcept
    {
      return LayoutPoint(X() + Width() / 2, Y() + Height() / 2);
    }

    constexpr void Move(const LayoutSize &size) noexcept
    {
      _location += size;
    }

    constexpr void MoveBy(const LayoutPoint &offset) noexcept
    {
      _location.Move(offset.X(), offset.Y());
    }

    template <typename T, typename U>
    constexpr void Move(T dx, U dy) noexcept
    {
      _location.Move(dx, dy);
    }

    constexpr void Expand(const LayoutSize &size) noexcept
    {
      _size += size;
    }

    void Expand(const LayoutBoxExtent &box) noexcept
    {
      _location.Move(-box.Left(), -box.Top());
      _size.Expand(box.Left() + box.Right(), box.Top() + box.Bottom());
    }

    void ExpandToInfiniteY() noexcept;
    void ExpandToInfiniteX() noexcept;

    template <typename T, typename U>
    constexpr void Expand(T dw, U dh) noexcept
    {
      _size.Expand(dw, dh);
    }

    constexpr void Contract(const LayoutSize &size) noexcept
    {
      _size -= size;
    }

    void Contract(const LayoutBoxExtent &box) noexcept
    {
      _location.Move(box.Left(), box.Top());
      _size.Shrink(box.Left() + box.Right(), box.Top() + box.Bottom());
    }

    template <typename T, typename U>
    constexpr void Contract(T dw, U dh) noexcept
    {
      _size.Expand(-dw, -dh);
    }

    constexpr void ShiftEdgesTo(LayoutUnit left, LayoutUnit top, LayoutUnit right, LayoutUnit bottom) noexcept
    {
      _location = {left, top};
      _size.SetWidth(right - left);
      _size.SetHeight(bottom - top);
    }

    constexpr void ShiftXEdgeTo(LayoutUnit edge) noexcept
    {
      LayoutUnit delta = edge - X();
      SetX(edge);
      SetWidth(std::max<LayoutUnit>(0, Width() - delta));
    }

    constexpr void ShiftMaxXEdgeTo(LayoutUnit edge) noexcept
    {
      LayoutUnit delta = edge - MaxX();
      SetWidth(std::max<LayoutUnit>(0, Width() + delta));
    }

    constexpr void ShiftYEdgeTo(LayoutUnit edge) noexcept
    {
      LayoutUnit delta = edge - Y();
      SetY(edge);
      SetHeight(std::max<LayoutUnit>(0, Height() - delta));
    }

    constexpr void ShiftMaxYEdgeTo(LayoutUnit edge) noexcept
    {
      LayoutUnit delta = edge - MaxY();
      SetHeight(std::max<LayoutUnit>(0, Height() + delta));
    }

    template <typename T>
    constexpr void ShiftXEdgeTo(T edge) noexcept
    {
      ShiftXEdgeTo(LayoutUnit(edge));
    }

    template <typename T>
    constexpr void ShiftMaxXEdgeTo(T edge) noexcept
    {
      ShiftMaxXEdgeTo(LayoutUnit(edge));
    }

    template <typename T>
    constexpr void ShiftYEdgeTo(T edge) noexcept
    {
      ShiftYEdgeTo(LayoutUnit(edge));
    }

    template <typename T>
    constexpr void ShiftMaxYEdgeTo(T edge) noexcept
    {
      ShiftMaxYEdgeTo(LayoutUnit(edge));
    }

    constexpr void ShiftXEdgeBy(LayoutUnit delta) noexcept
    {
      Move(delta, 0);
      SetWidth(std::max<LayoutUnit>(0, Width() - delta));
    }

    constexpr void ShiftMaxXEdgeBy(LayoutUnit delta) noexcept
    {
      SetWidth(std::max<LayoutUnit>(0, Width() + delta));
    }

    constexpr void ShiftYEdgeBy(LayoutUnit delta) noexcept
    {
      Move(0, delta);
      SetHeight(std::max<LayoutUnit>(0, Height() - delta));
    }

    constexpr void ShiftMaxYEdgeBy(LayoutUnit delta) noexcept
    {
      SetHeight(std::max<LayoutUnit>(0, Height() + delta));
    }

    // Typically top left.
    KRYS_NODISCARD constexpr LayoutPoint MinXMinYCorner() const noexcept
    {
      return _location;
    }

    // Typically top right.
    KRYS_NODISCARD constexpr LayoutPoint MaxXMinYCorner() const noexcept
    {
      return LayoutPoint(_location.X() + _size.Width(), _location.Y());
    }

    // Typically bottom left.
    KRYS_NODISCARD constexpr LayoutPoint MinXMaxYCorner() const noexcept
    {
      return LayoutPoint(_location.X(), _location.Y() + _size.Height());
    }

    // Typically bottom right.
    KRYS_NODISCARD constexpr LayoutPoint MaxXMaxYCorner() const noexcept
    {
      return LayoutPoint(_location.X() + _size.Width(), _location.Y() + _size.Height());
    }

    KRYS_NODISCARD bool IsMaxXMaxYRepresentable() const noexcept
    {
      FloatRect rect = *this;
      float maxX = rect.MaxX();
      float maxY = rect.MaxY();
      return maxX > LayoutUnit::NearlyMin() && maxX < LayoutUnit::NearlyMax()
             && maxY > LayoutUnit::NearlyMin() && maxY < LayoutUnit::NearlyMax();
    }

    KRYS_NODISCARD bool Intersects(const LayoutRect &other) const noexcept;
    KRYS_NODISCARD bool Contains(const LayoutRect &other) const noexcept;

    // This checks to see if the rect contains x,y in the traditional sense.
    // Equivalent to checking if the rect contains a 1x1 rect below and to the right of (px,py).
    KRYS_NODISCARD constexpr bool Contains(LayoutUnit px, LayoutUnit py) const noexcept
    {
      return px >= X() && px < MaxX() && py >= Y() && py < MaxY();
    }

    KRYS_NODISCARD constexpr bool Contains(const LayoutPoint &point) const noexcept
    {
      return Contains(point.X(), point.Y());
    }

    void Intersect(const LayoutRect &other) noexcept;
    bool EdgeInclusiveIntersect(const LayoutRect &other) noexcept;
    void Unite(const LayoutRect &other) noexcept;
    void UniteEvenIfEmpty(const LayoutRect &other) noexcept;
    void UniteIfNonZero(const LayoutRect &other) noexcept;
    KRYS_NODISCARD bool CheckedUnite(const LayoutRect &other) noexcept;

    constexpr void InflateX(LayoutUnit dx) noexcept
    {
      _location.SetX(_location.X() - dx);
      _size.SetWidth(_size.Width() + dx + dx);
    }

    constexpr void InflateY(LayoutUnit dy) noexcept
    {
      _location.SetY(_location.Y() - dy);
      _size.SetHeight(_size.Height() + dy + dy);
    }

    constexpr void Inflate(LayoutSize size) noexcept
    {
      InflateX(size.Width());
      InflateY(size.Height());
    }

    template <typename T>
    constexpr void InflateX(T dx) noexcept
    {
      InflateX(LayoutUnit(dx));
    }

    template <typename T>
    constexpr void InflateY(T dy) noexcept
    {
      InflateY(LayoutUnit(dy));
    }

    template <typename T>
    constexpr void Inflate(T d) noexcept
    {
      InflateX(d);
      InflateY(d);
    }

    void Scale(float scaleValue) noexcept;
    void Scale(float xScale, float yScale) noexcept;

    KRYS_NODISCARD constexpr LayoutRect TransposedRect() const noexcept
    {
      return LayoutRect(_location.TransposedPoint(), _size.TransposedSize());
    }

    KRYS_NODISCARD constexpr bool IsInfinite() const noexcept
    {
      return *this == InfiniteRect();
    }

    KRYS_NODISCARD constexpr static LayoutRect InfiniteRect() noexcept
    {
      // Return a rect that is slightly smaller than the true max rect to allow pixelSnapping to round up to
      // the nearest IntRect without overflowing.
      return LayoutRect(LayoutUnit::NearlyMin() / 2, LayoutUnit::NearlyMin() / 2, LayoutUnit::NearlyMax(),
                        LayoutUnit::NearlyMax());
    }

    constexpr operator FloatRect() const noexcept
    {
      return FloatRect(_location, _size);
    }

    KRYS_NODISCARD friend constexpr bool operator==(const LayoutRect &,
                                                    const LayoutRect &) noexcept = default;
  };

  KRYS_NODISCARD inline LayoutRect Intersection(const LayoutRect &a, const LayoutRect &b) noexcept
  {
    LayoutRect c = a;
    c.Intersect(b);
    return c;
  }

  KRYS_NODISCARD inline LayoutRect UnionRect(const LayoutRect &a, const LayoutRect &b) noexcept
  {
    LayoutRect c = a;
    c.Unite(b);
    return c;
  }

  KRYS_NODISCARD LayoutRect UnionRect(Span<const LayoutRect> rects) noexcept;

  // Integral snapping functions.
  KRYS_NODISCARD constexpr IntRect SnappedIntRect(const LayoutRect &rect) noexcept
  {
    return IntRect(RoundedIntPoint(rect.Location()), SnappedIntSize(rect.Size(), rect.Location()));
  }

  KRYS_NODISCARD constexpr IntRect SnappedIntRect(LayoutUnit left, LayoutUnit top, LayoutUnit width,
                                                  LayoutUnit height) noexcept
  {
    return IntRect(IntPoint(left.Round(), top.Round()),
                   SnappedIntSize(LayoutSize(width, height), LayoutPoint(left, top)));
  }

  KRYS_NODISCARD constexpr IntRect SnappedIntRect(LayoutPoint location, LayoutSize size) noexcept
  {
    return IntRect(RoundedIntPoint(location), SnappedIntSize(size, location));
  }

  KRYS_NODISCARD IntRect EnclosingIntRect(const LayoutRect &rect) noexcept;
  KRYS_NODISCARD LayoutRect EnclosingLayoutRect(const FloatRect &rect) noexcept;

  // Device pixel snapping functions.
  KRYS_NODISCARD inline FloatRect SnapRectToDevicePixels(const LayoutRect &rect,
                                                         float pixelSnappingFactor) noexcept
  {
    return FloatRect(FloatPoint(RoundToDevicePixel(rect.X(), pixelSnappingFactor),
                                RoundToDevicePixel(rect.Y(), pixelSnappingFactor)),
                     SnapSizeToDevicePixel(rect.Size(), rect.Location(), pixelSnappingFactor));
  }

  KRYS_NODISCARD inline FloatRect SnapRectToDevicePixels(LayoutUnit x, LayoutUnit y, LayoutUnit width,
                                                         LayoutUnit height,
                                                         float pixelSnappingFactor) noexcept
  {
    return SnapRectToDevicePixels(LayoutRect(x, y, width, height), pixelSnappingFactor);
  }

  // FIXME: This needs to take vertical centering into account too.
  KRYS_NODISCARD inline FloatRect SnapRectToDevicePixelsWithWritingDirection(const LayoutRect &rect,
                                                                             float deviceScaleFactor,
                                                                             bool ltr) noexcept
  {
    if (!ltr)
    {
      FloatPoint snappedTopRight = RoundPointToDevicePixels(rect.MaxXMinYCorner(), deviceScaleFactor, ltr);
      FloatSize snappedSize = SnapSizeToDevicePixel(rect.Size(), rect.MaxXMinYCorner(), deviceScaleFactor);
      return FloatRect(snappedTopRight.X() - snappedSize.Width(), snappedTopRight.Y(), snappedSize.Width(),
                       snappedSize.Height());
    }
    return SnapRectToDevicePixels(rect, deviceScaleFactor);
  }

  KRYS_NODISCARD FloatRect EncloseRectToDevicePixels(const LayoutRect &rect,
                                                     float pixelSnappingFactor) noexcept;
}
