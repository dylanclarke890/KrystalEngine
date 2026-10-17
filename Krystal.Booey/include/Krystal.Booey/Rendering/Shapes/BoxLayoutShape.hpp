#pragma once

#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
#include "Krystal.Booey/Rendering/Shapes/LayoutShape.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"

namespace krys::boo
{
  class RenderBox;

  /// @brief The rounded rect of @p renderer's @p box, for a box shape.
  KRYS_NODISCARD LayoutRoundedRect ComputeRoundedRectForBoxShape(CSSBoxType box,
                                                                 const RenderBox &renderer) noexcept;

  /// @brief A rounded rect shape: an `inset()`, or a reference box.
  class BoxLayoutShape final : public LayoutShape
  {
  private:
    FloatRoundedRect _bounds;

  public:
    explicit BoxLayoutShape(const FloatRoundedRect &bounds) noexcept : _bounds(bounds)
    {
    }

    KRYS_NODISCARD LayoutRect ShapeMarginLogicalBoundingBox() const noexcept override;
    KRYS_NODISCARD bool IsEmpty() const noexcept override
    {
      return _bounds.IsEmpty();
    }
    KRYS_NODISCARD LineSegment GetExcludedInterval(LayoutUnit logicalTop,
                                                   LayoutUnit logicalHeight) const noexcept override;

  private:
    KRYS_NODISCARD FloatRoundedRect ShapeMarginBounds() const noexcept;
  };
}
