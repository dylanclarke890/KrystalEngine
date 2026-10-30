#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Rendering/Shapes/LayoutShape.hpp"

namespace krys::boo
{
  /// @brief An ellipse inscribed in a rect: a `circle()` or an `ellipse()`.
  class RectangleLayoutShape final : public LayoutShape
  {
  private:
    FloatRect _bounds;
    FloatSize _radii;
    float _boxLogicalWidth {0.f};

  public:
    RectangleLayoutShape(const FloatRect &bounds, const FloatSize &radii, float boxLogicalWidth) noexcept
        : _bounds(bounds), _radii(radii), _boxLogicalWidth(boxLogicalWidth)
    {
    }

    KRYS_NODISCARD LayoutRect ShapeMarginLogicalBoundingBox() const noexcept override
    {
      return LayoutRect(ShapeMarginBounds());
    }
    KRYS_NODISCARD bool IsEmpty() const noexcept override
    {
      return _bounds.IsEmpty();
    }
    KRYS_NODISCARD LineSegment GetExcludedInterval(LayoutUnit logicalTop,
                                                   LayoutUnit logicalHeight) const noexcept override;

  private:
    KRYS_NODISCARD FloatRect ShapeMarginBounds() const noexcept;

    KRYS_NODISCARD float Rx() const noexcept
    {
      return _radii.Width();
    }
    KRYS_NODISCARD float Ry() const noexcept
    {
      return _radii.Height();
    }
    KRYS_NODISCARD float X() const noexcept
    {
      return _bounds.X();
    }
    KRYS_NODISCARD float Y() const noexcept
    {
      return _bounds.Y();
    }
    KRYS_NODISCARD float Width() const noexcept
    {
      return _bounds.Width();
    }
    KRYS_NODISCARD float Height() const noexcept
    {
      return _bounds.Height();
    }
  };
}
