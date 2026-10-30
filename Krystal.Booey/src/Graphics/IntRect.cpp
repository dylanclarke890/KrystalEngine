#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD constexpr bool SumOverflows(int32 a, int32 b) noexcept
    {
      int64 sum = static_cast<int64>(a) + b;
      return sum > Limits<int32>::max() || sum < Limits<int32>::min();
    }

    KRYS_NODISCARD constexpr int32 DistanceToInterval(int32 pos, int32 start, int32 end) noexcept
    {
      if (pos < start)
      {
        return start - pos;
      }
      if (pos > end)
      {
        return end - pos;
      }
      return 0;
    }
  }

  IntRect::IntRect(const FloatRect &r) noexcept
      : _location(ClampToInteger(r.X()), ClampToInteger(r.Y())),
        _size(ClampToInteger(r.Width()), ClampToInteger(r.Height()))
  {
  }

  IntRect::IntRect(const LayoutRect &r) noexcept
      : _location(r.X().ToInt(), r.Y().ToInt()), _size(r.Width().ToInt(), r.Height().ToInt())
  {
  }

  bool IntRect::Intersects(const IntRect &other) const noexcept
  {
    // Checking emptiness handles negative widths as well as zero.
    return !IsEmpty() && !other.IsEmpty() && X() < other.MaxX() && other.X() < MaxX() && Y() < other.MaxY()
           && other.Y() < MaxY();
  }

  bool IntRect::Contains(const IntRect &other) const noexcept
  {
    return X() <= other.X() && MaxX() >= other.MaxX() && Y() <= other.Y() && MaxY() >= other.MaxY();
  }

  void IntRect::Intersect(const IntRect &other) noexcept
  {
    int32 l = std::max(X(), other.X());
    int32 t = std::max(Y(), other.Y());
    int32 r = std::min(MaxX(), other.MaxX());
    int32 b = std::min(MaxY(), other.MaxY());

    // Return a clean empty rectangle for non-intersecting cases.
    if (l >= r || t >= b)
    {
      l = 0;
      t = 0;
      r = 0;
      b = 0;
    }

    _location.SetX(l);
    _location.SetY(t);
    _size.SetWidth(r - l);
    _size.SetHeight(b - t);
  }

  void IntRect::Unite(const IntRect &other) noexcept
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

    int32 l = std::min(X(), other.X());
    int32 t = std::min(Y(), other.Y());
    int32 r = std::max(MaxX(), other.MaxX());
    int32 b = std::max(MaxY(), other.MaxY());

    _location.SetX(l);
    _location.SetY(t);
    _size.SetWidth(r - l);
    _size.SetHeight(b - t);
  }

  void IntRect::UniteIfNonZero(const IntRect &other) noexcept
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

    int32 left = std::min(X(), other.X());
    int32 top = std::min(Y(), other.Y());
    int32 right = std::max(MaxX(), other.MaxX());
    int32 bottom = std::max(MaxY(), other.MaxY());

    _location.SetX(left);
    _location.SetY(top);
    _size.SetWidth(right - left);
    _size.SetHeight(bottom - top);
  }

  void IntRect::Scale(float s) noexcept
  {
    _location.SetX(static_cast<int32>(X() * s));
    _location.SetY(static_cast<int32>(Y() * s));
    _size.SetWidth(static_cast<int32>(Width() * s));
    _size.SetHeight(static_cast<int32>(Height() * s));
  }

  IntSize IntRect::DifferenceToPoint(const IntPoint &point) const noexcept
  {
    int32 xdistance = DistanceToInterval(point.X(), X(), MaxX());
    int32 ydistance = DistanceToInterval(point.Y(), Y(), MaxY());
    return IntSize(xdistance, ydistance);
  }

  bool IntRect::IsValid() const noexcept
  {
    return !SumOverflows(_location.X(), _size.Width()) && !SumOverflows(_location.Y(), _size.Height());
  }

  IntRect IntRect::ToRectWithExtentsClippedToNumericLimits() const noexcept
  {
    IntRect clippedRect {*this};
    constexpr auto max = Limits<int32>::max();

    if (SumOverflows(X(), Width()))
    {
      clippedRect.SetWidth(max - X());
    }

    if (SumOverflows(Y(), Height()))
    {
      clippedRect.SetHeight(max - Y());
    }

    return clippedRect;
  }

  TextStream &operator<<(TextStream &ts, const IntRect &r) noexcept
  {
    if (ts.HasFormattingFlag(TextStream::Formatting::SVGStyleRect))
    {
      return ts << "at (" << r.X() << ',' << r.Y() << ") size " << r.Width() << 'x' << r.Height();
    }

    return ts << r.Location() << ' ' << r.Size();
  }
}
