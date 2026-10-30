#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Core/FloatConversion.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  FloatRect::FloatRect(const IntRect &r) noexcept : _location(r.Location()), _size(r.Size())
  {
  }

  FloatRect FloatRect::NarrowPrecision(double x, double y, double width, double height) noexcept
  {
    return FloatRect(NarrowPrecisionToFloat(x), NarrowPrecisionToFloat(y), NarrowPrecisionToFloat(width),
                     NarrowPrecisionToFloat(height));
  }

  bool FloatRect::IsExpressibleAsIntRect() const noexcept
  {
    return IsWithinIntRange(X()) && IsWithinIntRange(Y()) && IsWithinIntRange(Width())
           && IsWithinIntRange(Height()) && IsWithinIntRange(MaxX()) && IsWithinIntRange(MaxY());
  }

  bool FloatRect::InclusivelyIntersects(const FloatRect &other) const noexcept
  {
    return Width() >= 0 && Height() >= 0 && other.Width() >= 0 && other.Height() >= 0 && X() <= other.MaxX()
           && other.X() <= MaxX() && Y() <= other.MaxY() && other.Y() <= MaxY();
  }

  bool FloatRect::Intersects(const FloatRect &other) const noexcept
  {
    // Checking emptiness handles negative widths and heights as well as zero.
    return !IsEmpty() && !other.IsEmpty() && X() < other.MaxX() && other.X() < MaxX() && Y() < other.MaxY()
           && other.Y() < MaxY();
  }

  bool FloatRect::Contains(const FloatRect &other) const noexcept
  {
    return X() <= other.X() && MaxX() >= other.MaxX() && Y() <= other.Y() && MaxY() >= other.MaxY();
  }

  bool FloatRect::Contains(const FloatPoint &point, ContainsMode containsMode) const noexcept
  {
    if (containsMode == ContainsMode::InsideOrOnStroke)
    {
      return Contains(point.X(), point.Y());
    }
    return X() < point.X() && MaxX() > point.X() && Y() < point.Y() && MaxY() > point.Y();
  }

  void FloatRect::Intersect(const FloatRect &other) noexcept
  {
    float l = std::max(X(), other.X());
    float t = std::max(Y(), other.Y());
    float r = std::min(MaxX(), other.MaxX());
    float b = std::min(MaxY(), other.MaxY());

    // Return a clean empty rectangle for non-intersecting cases.
    if (l >= r || t >= b)
    {
      l = 0;
      t = 0;
      r = 0;
      b = 0;
    }

    ShiftEdgesTo(l, t, r, b);
  }

  bool FloatRect::EdgeInclusiveIntersect(const FloatRect &other) noexcept
  {
    FloatPoint newLocation(std::max(X(), other.X()), std::max(Y(), other.Y()));
    FloatPoint newMaxPoint(std::min(MaxX(), other.MaxX()), std::min(MaxY(), other.MaxY()));

    bool intersects = true;

    // Return a clean empty rectangle for non-intersecting cases.
    if (newLocation.X() > newMaxPoint.X() || newLocation.Y() > newMaxPoint.Y())
    {
      newLocation = {};
      newMaxPoint = {};
      intersects = false;
    }

    _location = newLocation;
    _size = newMaxPoint - newLocation;
    return intersects;
  }

  void FloatRect::Unite(const FloatRect &other) noexcept
  {
    // Handle empty special cases first.
    if (other.IsEmpty())
    {
      return;
    }
    if (IsEmpty())
    {
      *this = other;
      return;
    }

    UniteEvenIfEmpty(other);
  }

  void FloatRect::UniteEvenIfEmpty(const FloatRect &other) noexcept
  {
    float minX = std::min(X(), other.X());
    float minY = std::min(Y(), other.Y());
    float maxX = std::max(MaxX(), other.MaxX());
    float maxY = std::max(MaxY(), other.MaxY());

    ShiftEdgesTo(minX, minY, maxX, maxY);
  }

  void FloatRect::UniteIfNonZero(const FloatRect &other) noexcept
  {
    // Handle empty special cases first.
    if (other.IsZero())
    {
      return;
    }
    if (IsZero())
    {
      *this = other;
      return;
    }

    UniteEvenIfEmpty(other);
  }

  void FloatRect::Extend(FloatPoint p) noexcept
  {
    float minX = std::min(X(), p.X());
    float minY = std::min(Y(), p.Y());
    float maxX = std::max(MaxX(), p.X());
    float maxY = std::max(MaxY(), p.Y());

    ShiftEdgesTo(minX, minY, maxX, maxY);
  }

  void FloatRect::Extend(FloatPoint minPoint, FloatPoint maxPoint) noexcept
  {
    krys_debug_assert(minPoint.X() <= maxPoint.X() && minPoint.Y() <= maxPoint.Y());

    float minX = std::min(X(), minPoint.X());
    float minY = std::min(Y(), minPoint.Y());
    float maxX = std::max(MaxX(), maxPoint.X());
    float maxY = std::max(MaxY(), maxPoint.Y());

    ShiftEdgesTo(minX, minY, maxX, maxY);
  }

  FloatRect NormaliseRect(const FloatRect &rect) noexcept
  {
    return FloatRect(std::min(rect.X(), rect.MaxX()), std::min(rect.Y(), rect.MaxY()),
                     std::max(rect.Width(), -rect.Width()), std::max(rect.Height(), -rect.Height()));
  }

  FloatRect EncloseRectToDevicePixels(const FloatRect &rect, float deviceScaleFactor) noexcept
  {
    FloatPoint location = FloorPointToDevicePixels(rect.MinXMinYCorner(), deviceScaleFactor);
    FloatPoint maxPoint = CeilPointToDevicePixels(rect.MaxXMaxYCorner(), deviceScaleFactor);

    return FloatRect(location, maxPoint - location);
  }

  IntRect EnclosingIntRect(const FloatRect &rect) noexcept
  {
    FloatPoint location = FlooredIntPoint(rect.MinXMinYCorner());
    FloatPoint maxPoint = CeiledIntPoint(rect.MaxXMaxYCorner());

    return IntRect(IntPoint(location), IntSize(maxPoint - location));
  }

  IntRect EnclosingIntRectPreservingEmptyRects(const FloatRect &rect) noexcept
  {
    // Empty rects with fractional x, y values turn into non-empty rects when converting to enclosing.
    // We want to ensure that empty rects stay empty after the conversion, since some callers
    // prefer this behavior.
    FloatPoint location = FlooredIntPoint(rect.MinXMinYCorner());
    if (rect.IsEmpty())
    {
      return IntRect(IntPoint(location), {});
    }

    FloatPoint maxPoint = CeiledIntPoint(rect.MaxXMaxYCorner());
    return IntRect(IntPoint(location), IntSize(maxPoint - location));
  }

  IntRect RoundedIntRect(const FloatRect &rect) noexcept
  {
    return IntRect(RoundedIntPoint(rect.Location()), RoundedIntSize(rect.Size()));
  }

  TextStream &operator<<(TextStream &ts, const FloatRect &r) noexcept
  {
    if (ts.HasFormattingFlag(TextStream::Formatting::SVGStyleRect))
    {
      // FIXME: callers should use the NumberRespectingIntegers flag.
      return ts << "at (" << TextStream::FormatNumberRespectingIntegers(r.X()) << ','
                << TextStream::FormatNumberRespectingIntegers(r.Y()) << ") size "
                << TextStream::FormatNumberRespectingIntegers(r.Width()) << "x"
                << TextStream::FormatNumberRespectingIntegers(r.Height());
    }

    return ts << r.Location() << ' ' << r.Size();
  }
}
