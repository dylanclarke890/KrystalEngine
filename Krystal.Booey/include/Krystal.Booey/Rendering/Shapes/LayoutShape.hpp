#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Style/Values/Shapes/BasicShape.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class LayoutRoundedRect;

  /// @brief A horizontal (logical) segment of a line.
  struct LineSegment
  {
    float LogicalLeft {0.f};
    float LogicalRight {0.f};
    bool IsValid {false};

    constexpr LineSegment() noexcept = default;

    constexpr LineSegment(float logicalLeft, float logicalRight) noexcept
        : LogicalLeft(logicalLeft), LogicalRight(logicalRight), IsValid(true)
    {
    }
  };

  /// @brief A representation of a BasicShape that enables layout code to determine how to break a line up
  /// into segments that will fit within or around a shape. The line is defined by a pair of logical Y
  /// coordinates and the computed segments are returned as pairs of logical X coordinates. The BasicShape
  /// itself is defined in physical coordinates.
  // FIXME: The shapes' display paths (buildDisplayPaths), for highlighting them, and raster shapes
  // (createRasterShape).
  class LayoutShape : public RefCounted<LayoutShape>
  {
  private:
    WritingMode _writingMode;
    float _margin {0.f};

  public:
    KRYS_NODISCARD static Ref<const LayoutShape>
      CreateShape(const style::BasicShape &basicShape, const LayoutPoint &borderBoxOffset,
                  const LayoutSize &logicalBoxSize, WritingMode writingMode, float logicalMargin) noexcept;
    KRYS_NODISCARD static Ref<const LayoutShape> CreateBoxShape(const LayoutRoundedRect &roundedRect,
                                                                WritingMode writingMode,
                                                                float logicalMargin) noexcept;

    virtual ~LayoutShape() noexcept = default;

    KRYS_NODISCARD virtual LayoutRect ShapeMarginLogicalBoundingBox() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsEmpty() const noexcept = 0;
    KRYS_NODISCARD virtual LineSegment GetExcludedInterval(LayoutUnit logicalTop,
                                                           LayoutUnit logicalHeight) const noexcept = 0;

    KRYS_NODISCARD bool LineOverlapsShapeMarginBounds(LayoutUnit lineTop,
                                                      LayoutUnit lineHeight) const noexcept
    {
      return LineOverlapsBoundingBox(lineTop, lineHeight, ShapeMarginLogicalBoundingBox());
    }

  protected:
    KRYS_NODISCARD float ShapeMargin() const noexcept
    {
      return _margin;
    }
    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept
    {
      return _writingMode;
    }
    KRYS_NODISCARD static bool ShouldFlipStartAndEndPoints(WritingMode writingMode) noexcept;

  private:
    KRYS_NODISCARD bool LineOverlapsBoundingBox(LayoutUnit lineTop, LayoutUnit lineHeight,
                                                const LayoutRect &rect) const noexcept
    {
      if (rect.IsEmpty())
      {
        return false;
      }
      return (lineTop < rect.MaxY() && lineTop + lineHeight > rect.Y())
             || (!lineHeight && lineTop == rect.Y());
    }
  };
}
