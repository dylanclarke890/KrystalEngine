#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD float Dot(const FloatSize &a, const FloatSize &b) noexcept
    {
      return a.Width() * b.Width() + a.Height() * b.Height();
    }

    KRYS_NODISCARD float Determinant(const FloatSize &a, const FloatSize &b) noexcept
    {
      return a.Width() * b.Height() - a.Height() * b.Width();
    }

    KRYS_NODISCARD bool IsPointInTriangle(const FloatPoint &p, const FloatPoint &t1, const FloatPoint &t2,
                                          const FloatPoint &t3) noexcept
    {
      // Compute vectors
      FloatSize v0 = t3 - t1;
      FloatSize v1 = t2 - t1;
      FloatSize v2 = p - t1;

      // Compute dot products
      float dot00 = Dot(v0, v0);
      float dot01 = Dot(v0, v1);
      float dot02 = Dot(v0, v2);
      float dot11 = Dot(v1, v1);
      float dot12 = Dot(v1, v2);

      // Compute barycentric coordinates
      float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
      float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
      float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

      // Check if point is in triangle
      return (u >= 0) && (v >= 0) && (u + v <= 1);
    }

    KRYS_NODISCARD float ClampToIntRange(float value) noexcept
    {
      if (std::isinf(value) || std::abs(value) > (static_cast<float>(Limits<int>::max()))) KRYS_UNLIKELY
      {
        return std::signbit(value) ? static_cast<float>(Limits<int>::min())
                                   : (static_cast<float>(Limits<int>::max()));
      }
      return value;
    }

    KRYS_NODISCARD FloatPoint RightMostCornerToVector(const FloatRect &rect, const FloatSize &vector) noexcept
    {
      // Return the corner of the rectangle that if it is to the left of the vector
      // would mean all of the rectangle is to the left of the vector.
      // The vector here represents the side between two points in a clockwise convex polygon.
      //
      //  Q  XXX
      // QQQ XXX   If the lower left corner of X is left of the vector that goes from the top corner of Q to
      //  QQQ      the right corner of Q, then all of X is left of the vector, and intersection impossible.
      //   Q
      //
      FloatPoint point;
      if (vector.Width() >= 0)
      {
        point.SetY(rect.MaxY());
      }
      else
      {
        point.SetY(rect.Y());
      }
      if (vector.Height() >= 0)
      {
        point.SetX(rect.X());
      }
      else
      {
        point.SetX(rect.MaxX());
      }
      return point;
    }

    // Tests whether the line is contained by or intersected with the circle.
    KRYS_NODISCARD bool LineIntersectsCircle(const FloatPoint &center, float radius, const FloatPoint &p0,
                                             const FloatPoint &p1) noexcept
    {
      float x0 = p0.X() - center.X(), y0 = p0.Y() - center.Y();
      float x1 = p1.X() - center.X(), y1 = p1.Y() - center.Y();
      float radius2 = radius * radius;
      if ((x0 * x0 + y0 * y0) <= radius2 || (x1 * x1 + y1 * y1) <= radius2)
      {
        return true;
      }
      if (p0 == p1)
      {
        return false;
      }

      float a = y0 - y1;
      float b = x1 - x0;
      float c = x0 * y1 - x1 * y0;
      float distance2 = c * c / (a * a + b * b);
      // If distance between the center point and the line > the radius,
      // the line doesn't cross (or is contained by) the ellipse.
      if (distance2 > radius2)
      {
        return false;
      }

      // The nearest point on the line is between p0 and p1?
      float x = -a * c / (a * a + b * b);
      float y = -b * c / (a * a + b * b);
      return (((x0 <= x && x <= x1) || (x0 >= x && x >= x1))
              && ((y0 <= y && y <= y1) || (y1 <= y && y <= y0)));
    }
  }

  FloatRect FloatQuad::BoundingBox() const noexcept
  {
    float left = ClampToIntRange(Min4(_p1.X(), _p2.X(), _p3.X(), _p4.X()));
    float top = ClampToIntRange(Min4(_p1.Y(), _p2.Y(), _p3.Y(), _p4.Y()));

    float right = ClampToIntRange(Max4(_p1.X(), _p2.X(), _p3.X(), _p4.X()));
    float bottom = ClampToIntRange(Max4(_p1.Y(), _p2.Y(), _p3.Y(), _p4.Y()));

    return FloatRect(left, top, right - left, bottom - top);
  }

  bool FloatQuad::IsRectilinear() const noexcept
  {
    return (AreEssentiallyEqual(_p1.X(), _p2.X()) && AreEssentiallyEqual(_p2.Y(), _p3.Y())
            && AreEssentiallyEqual(_p3.X(), _p4.X()) && AreEssentiallyEqual(_p4.Y(), _p1.Y()))
           || (AreEssentiallyEqual(_p1.Y(), _p2.Y()) && AreEssentiallyEqual(_p2.X(), _p3.X())
               && AreEssentiallyEqual(_p3.Y(), _p4.Y()) && AreEssentiallyEqual(_p4.X(), _p1.X()));
  }

  bool FloatQuad::ContainsPoint(const FloatPoint &p) const noexcept
  {
    return IsPointInTriangle(p, _p1, _p2, _p3) || IsPointInTriangle(p, _p1, _p3, _p4);
  }

  // Note that we only handle convex quads here.
  bool FloatQuad::ContainsQuad(const FloatQuad &other) const noexcept
  {
    return ContainsPoint(other.P1()) && ContainsPoint(other.P2()) && ContainsPoint(other.P3())
           && ContainsPoint(other.P4());
  }

  bool FloatQuad::IntersectsRect(const FloatRect &rect) const noexcept
  {
    // For each side of the quad clockwise we check if the rectangle is to the left of it
    // since only content on the right can onlap with the quad.
    // This only works if the quad is convex.
    FloatSize v1, v2, v3, v4;

    // Ensure we use clockwise vectors.
    if (!IsCounterclockwise())
    {
      v1 = _p2 - _p1;
      v2 = _p3 - _p2;
      v3 = _p4 - _p3;
      v4 = _p1 - _p4;
    }
    else
    {
      v1 = _p4 - _p1;
      v2 = _p1 - _p2;
      v3 = _p2 - _p3;
      v4 = _p3 - _p4;
    }

    FloatPoint p = RightMostCornerToVector(rect, v1);
    if (Determinant(v1, p - _p1) < 0)
    {
      return false;
    }

    p = RightMostCornerToVector(rect, v2);
    if (Determinant(v2, p - _p2) < 0)
    {
      return false;
    }

    p = RightMostCornerToVector(rect, v3);
    if (Determinant(v3, p - _p3) < 0)
    {
      return false;
    }

    p = RightMostCornerToVector(rect, v4);
    if (Determinant(v4, p - _p4) < 0)
    {
      return false;
    }

    // If not all of the rectangle is outside one of the quad's four sides, then that means at least
    // a part of the rectangle is overlapping the quad.
    return true;
  }

  bool FloatQuad::IntersectsCircle(const FloatPoint &center, float radius) const noexcept
  {
    return ContainsPoint(center) // The circle may be totally contained by the quad.
           || LineIntersectsCircle(center, radius, _p1, _p2) || LineIntersectsCircle(center, radius, _p2, _p3)
           || LineIntersectsCircle(center, radius, _p3, _p4)
           || LineIntersectsCircle(center, radius, _p4, _p1);
  }

  bool FloatQuad::IntersectsEllipse(const FloatPoint &center, const FloatSize &radii) const noexcept
  {
    // Transform the ellipse to an origin-centered circle whose radius is the product of major radius and
    // minor radius. Here we apply the same transformation to the quad.
    FloatQuad transformedQuad(*this);
    transformedQuad.Move(-center.X(), -center.Y());
    transformedQuad.Scale(radii.Height(), radii.Width());

    FloatPoint originPoint;
    return transformedQuad.IntersectsCircle(originPoint, radii.Height() * radii.Width());
  }

  bool FloatQuad::IsCounterclockwise() const noexcept
  {
    // Return if the two first vectors are turning clockwise. If the quad is convex then all following vectors
    // will turn the same way.
    return Determinant(_p2 - _p1, _p3 - _p2) < 0;
  }

  bool FloatQuad::IsEmpty() const noexcept
  {
    if (AreEssentiallyEqual(_p1, _p3) || AreEssentiallyEqual(_p2, _p4))
    {
      // If either diagonal is zero length, then the "quad" either consists of 1 or 2 line segments, or it's
      // just a point.
      return true;
    }

    if (AreEssentiallyEqual(_p1, _p2) && AreEssentiallyEqual(_p3, _p4))
    {
      // If both top points and both bottom points are equal, then the "quad" is just a single line segment.
      return true;
    }

    if (AreEssentiallyEqual(_p1, _p4) && AreEssentiallyEqual(_p2, _p3))
    {
      // If both left points and both right points are equal, then the "quad" is just a single line segment.
      return true;
    }

    // Fall back to checking whether the 4 points of the quad are colinear (in other words, check whether the
    // three vectors from one point to each of the other points are capable of forming a 2D basis).
    auto b1 = _p1 - _p2;
    auto b2 = _p1 - _p3;
    auto b3 = _p1 - _p4;

    if (!b1.IsZero())
    {
      b1 = b1 / b1.DiagonalLength();
    }

    if (!b2.IsZero())
    {
      b2 = b2 / b2.DiagonalLength();
    }

    if (!b3.IsZero())
    {
      b3 = b3 / b3.DiagonalLength();
    }

    auto areNormalisedVectorsLinearlyIndependent = [](const FloatSize &u, const FloatSize &v)
    {
      if (u.IsZero() || v.IsZero())
      {
        return false;
      }

      auto dotProduct = u.Width() * v.Width() + u.Height() * v.Height();
      return !AreEssentiallyEqual<float>(dotProduct, 1) && !AreEssentiallyEqual<float>(dotProduct, -1);
    };

    return !areNormalisedVectorsLinearlyIndependent(b1, b2)
           && !areNormalisedVectorsLinearlyIndependent(b2, b3)
           && !areNormalisedVectorsLinearlyIndependent(b1, b3);
  }

  List<FloatRect> BoundingBoxes(Span<const FloatQuad> quads)
  {
    List<FloatRect> boxes;
    boxes.reserve(quads.size());
    for (auto &quad : quads)
    {
      boxes.push_back(quad.BoundingBox());
    }
    return boxes;
  }

  FloatRect UnitedBoundingBoxes(Span<const FloatQuad> quads) noexcept
  {
    auto size = quads.size();
    if (!size)
    {
      return {};
    }

    auto result = quads[0].BoundingBox();
    for (size_t i = 1; i < size; ++i)
    {
      result.UniteEvenIfEmpty(quads[i].BoundingBox());
    }

    return result;
  }
}
