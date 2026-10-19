#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Rendering/HitTestLocation.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief A clip rect, which may be infinite, and whether a border radius affects it.
  class ClipRect
  {
  private:
    LayoutRect _rect;
    bool _affectedByRadius {false};

  public:
    ClipRect() noexcept = default;

    ClipRect(const LayoutRect &rect) noexcept : _rect(rect)
    {
    }

    KRYS_NODISCARD const LayoutRect &Rect() const noexcept
    {
      return _rect;
    }

    void Reset() noexcept
    {
      _rect = LayoutRect::InfiniteRect();
    }

    KRYS_NODISCARD bool AffectedByRadius() const noexcept
    {
      return _affectedByRadius;
    }
    void SetAffectedByRadius(bool affectedByRadius) noexcept
    {
      _affectedByRadius = affectedByRadius;
    }

    KRYS_NODISCARD friend bool operator==(const ClipRect &, const ClipRect &) noexcept = default;

    void Intersect(const LayoutRect &other) noexcept
    {
      if (other.IsInfinite())
      {
        return;
      }
      if (IsInfinite())
      {
        _rect = other;
      }
      else
      {
        _rect.Intersect(other);
      }
    }

    void Intersect(const ClipRect &other) noexcept
    {
      Intersect(other.Rect());
      if (other.AffectedByRadius())
      {
        _affectedByRadius = true;
      }
    }

    void Move(LayoutUnit x, LayoutUnit y) noexcept
    {
      _rect.Move(x, y);
    }
    void Move(const LayoutSize &size) noexcept
    {
      _rect.Move(size);
    }
    void MoveBy(const LayoutPoint &point) noexcept
    {
      _rect.MoveBy(point);
    }

    KRYS_NODISCARD bool Intersects(const LayoutRect &rect) const noexcept
    {
      if (IsInfinite() || rect.IsInfinite())
      {
        return true;
      }
      return _rect.Intersects(rect);
    }
    KRYS_NODISCARD bool Intersects(const HitTestLocation &hitTestLocation) const noexcept
    {
      if (IsInfinite())
      {
        return true;
      }
      return hitTestLocation.Intersects(_rect);
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _rect.IsEmpty();
    }
    KRYS_NODISCARD bool IsInfinite() const noexcept
    {
      return _rect.IsInfinite();
    }

    void InflateX(LayoutUnit dx) noexcept
    {
      _rect.InflateX(dx);
    }
    void InflateY(LayoutUnit dy) noexcept
    {
      _rect.InflateY(dy);
    }
    void Inflate(LayoutUnit d) noexcept
    {
      InflateX(d);
      InflateY(d);
    }
  };

  KRYS_NODISCARD inline ClipRect Intersection(const ClipRect &a, const ClipRect &b) noexcept
  {
    ClipRect c = a;
    c.Intersect(b);
    return c;
  }
}
