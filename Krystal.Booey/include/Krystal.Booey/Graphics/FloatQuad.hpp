#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  // FIXME: Seems like this would be better as a struct.
  // A FloatQuad is a collection of 4 points, often representing the result of
  // mapping a rectangle through transforms. When initialized from a rect, the
  // points are in clockwise order from top left.
  /// @brief Four points, usually a rectangle mapped through a transform.
  class FloatQuad
  {
  private:
    FloatPoint _p1;
    FloatPoint _p2;
    FloatPoint _p3;
    FloatPoint _p4;

  public:
    constexpr FloatQuad() noexcept = default;

    constexpr FloatQuad(const FloatPoint &p1, const FloatPoint &p2, const FloatPoint &p3,
                        const FloatPoint &p4) noexcept
        : _p1(p1), _p2(p2), _p3(p3), _p4(p4)
    {
    }

    constexpr FloatQuad(const FloatRect &inRect) noexcept
        : _p1(inRect.Location()), _p2(inRect.MaxX(), inRect.Y()), _p3(inRect.MaxX(), inRect.MaxY()),
          _p4(inRect.X(), inRect.MaxY())
    {
    }

    KRYS_NODISCARD constexpr const FloatPoint &P1() const noexcept
    {
      return _p1;
    }

    KRYS_NODISCARD constexpr const FloatPoint &P2() const noexcept
    {
      return _p2;
    }

    KRYS_NODISCARD constexpr const FloatPoint &P3() const noexcept
    {
      return _p3;
    }

    KRYS_NODISCARD constexpr const FloatPoint &P4() const noexcept
    {
      return _p4;
    }

    constexpr void SetP1(const FloatPoint &p) noexcept
    {
      _p1 = p;
    }

    constexpr void SetP2(const FloatPoint &p) noexcept
    {
      _p2 = p;
    }

    constexpr void SetP3(const FloatPoint &p) noexcept
    {
      _p3 = p;
    }

    constexpr void SetP4(const FloatPoint &p) noexcept
    {
      _p4 = p;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept;

    // This method will not identify "slanted" empty quads.
    KRYS_NODISCARD bool BoundingBoxIsEmpty() const noexcept
    {
      return BoundingBox().IsEmpty();
    }

    // Tests whether this quad can be losslessly represented by a FloatRect,
    // that is, if two edges are parallel to the x-axis and the other two
    // are parallel to the y-axis. If this method returns true, the
    // corresponding FloatRect can be retrieved with boundingBox().
    KRYS_NODISCARD bool IsRectilinear() const noexcept;

    // Tests whether the given point is inside, or on an edge or corner of this quad.
    KRYS_NODISCARD bool ContainsPoint(const FloatPoint &p) const noexcept;

    // Tests whether the four corners of other are inside, or coincident with the sides of this quad.
    // Note that this only works for convex quads, but that includes all quads that originate
    // from transformed rects.
    KRYS_NODISCARD bool ContainsQuad(const FloatQuad &other) const noexcept;

    // Tests whether any part of the rectangle intersects with this quad.
    // This only works for convex quads.
    KRYS_NODISCARD bool IntersectsRect(const FloatRect &rect) const noexcept;

    // Test whether any part of the circle/ellipse intersects with this quad.
    // Note that these two functions only work for convex quads.
    KRYS_NODISCARD bool IntersectsCircle(const FloatPoint &center, float radius) const noexcept;
    KRYS_NODISCARD bool IntersectsEllipse(const FloatPoint &center, const FloatSize &radii) const noexcept;

    // The center of the quad. If the quad is the result of a affine-transformed rectangle this is the same as
    // the original center transformed.
    KRYS_NODISCARD constexpr FloatPoint Center() const noexcept
    {
      return FloatPoint(static_cast<float>((_p1.X() + _p2.X() + _p3.X() + _p4.X()) / 4.0),
                        static_cast<float>((_p1.Y() + _p2.Y() + _p3.Y() + _p4.Y()) / 4.0));
    }

    KRYS_NODISCARD FloatRect BoundingBox() const noexcept;

    KRYS_NODISCARD IntRect EnclosingBoundingBox() const noexcept
    {
      return EnclosingIntRect(BoundingBox());
    }

    constexpr void Move(const FloatSize &offset) noexcept
    {
      _p1 += offset;
      _p2 += offset;
      _p3 += offset;
      _p4 += offset;
    }

    constexpr void Move(float dx, float dy) noexcept
    {
      _p1.Move(dx, dy);
      _p2.Move(dx, dy);
      _p3.Move(dx, dy);
      _p4.Move(dx, dy);
    }

    constexpr void Scale(float s) noexcept
    {
      Scale(s, s);
    }

    constexpr void Scale(float dx, float dy) noexcept
    {
      _p1.Scale(dx, dy);
      _p2.Scale(dx, dy);
      _p3.Scale(dx, dy);
      _p4.Scale(dx, dy);
    }

    // Tests whether points are in clock-wise, or counter clock-wise order.
    // Note that output is undefined when all points are colinear.
    KRYS_NODISCARD bool IsCounterclockwise() const noexcept;

    KRYS_NODISCARD friend constexpr bool operator==(const FloatQuad &, const FloatQuad &) noexcept = default;
  };

  constexpr FloatQuad &operator+=(FloatQuad &a, const FloatSize &b) noexcept
  {
    a.Move(b);
    return a;
  }

  constexpr FloatQuad &operator-=(FloatQuad &a, const FloatSize &b) noexcept
  {
    a.Move(-b.Width(), -b.Height());
    return a;
  }

  KRYS_NODISCARD List<FloatRect> BoundingBoxes(Span<const FloatQuad> quads);
  KRYS_NODISCARD FloatRect UnitedBoundingBoxes(Span<const FloatQuad> quads) noexcept;
}
