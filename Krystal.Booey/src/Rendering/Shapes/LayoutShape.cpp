#include "Krystal.Booey/Rendering/Shapes/LayoutShape.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Rendering/Shapes/BoxLayoutShape.hpp"
#include "Krystal.Booey/Rendering/Shapes/PolygonLayoutShape.hpp"
#include "Krystal.Booey/Rendering/Shapes/RectangleLayoutShape.hpp"
#include "Krystal.Booey/Style/Values/Shapes/BasicShape.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD Ref<LayoutShape> CreateInsetShape(const FloatRoundedRect &bounds) noexcept
    {
      krys_debug_assert(bounds.Rect().Width() >= 0 && bounds.Rect().Height() >= 0);
      return AdoptRef(*new BoxLayoutShape(bounds));
    }

    KRYS_NODISCARD Ref<LayoutShape> CreateCircleShape(const FloatPoint &center, float radius,
                                                      float boxLogicalWidth) noexcept
    {
      krys_debug_assert(radius >= 0);
      return AdoptRef(
        *new RectangleLayoutShape(FloatRect(center.X() - radius, center.Y() - radius, radius * 2, radius * 2),
                                  FloatSize(radius, radius), boxLogicalWidth));
    }

    KRYS_NODISCARD Ref<LayoutShape> CreateEllipseShape(const FloatPoint &center, const FloatSize &radii,
                                                       float boxLogicalWidth) noexcept
    {
      krys_debug_assert(radii.Width() >= 0 && radii.Height() >= 0);
      return AdoptRef(
        *new RectangleLayoutShape(FloatRect(center.X() - radii.Width(), center.Y() - radii.Height(),
                                            radii.Width() * 2, radii.Height() * 2),
                                  radii, boxLogicalWidth));
    }

    KRYS_NODISCARD Ref<LayoutShape> CreatePolygonShape(List<FloatPoint> &&vertices,
                                                       float boxLogicalWidth) noexcept
    {
      return AdoptRef(*new PolygonLayoutShape(krys::move(vertices), boxLogicalWidth));
    }

    KRYS_NODISCARD FloatRect PhysicalRectToLogical(const FloatRect &rect, float logicalBoxHeight,
                                                   WritingMode writingMode) noexcept
    {
      if (writingMode.IsHorizontal())
      {
        return rect;
      }
      if (writingMode.IsBlockFlipped())
      {
        return FloatRect(rect.Y(), logicalBoxHeight - rect.MaxX(), rect.Height(), rect.Width());
      }
      return rect.TransposedRect();
    }

    KRYS_NODISCARD FloatPoint PhysicalPointToLogical(const FloatPoint &point, float logicalBoxHeight,
                                                     WritingMode writingMode) noexcept
    {
      if (writingMode.IsHorizontal())
      {
        return point;
      }
      if (writingMode.IsBlockFlipped())
      {
        return FloatPoint(point.Y(), logicalBoxHeight - point.X());
      }
      return point.TransposedPoint();
    }

    KRYS_NODISCARD FloatSize PhysicalSizeToLogical(const FloatSize &size, WritingMode writingMode) noexcept
    {
      if (writingMode.IsHorizontal())
      {
        return size;
      }
      return size.TransposedSize();
    }
  }

  Ref<const LayoutShape> LayoutShape::CreateShape(const style::BasicShape &basicShape,
                                                  const LayoutPoint &borderBoxOffset,
                                                  const LayoutSize &logicalBoxSize, WritingMode writingMode,
                                                  float logicalMargin) noexcept
  {
    bool horizontalWritingMode = writingMode.IsHorizontal();
    float boxWidth =
      horizontalWritingMode ? logicalBoxSize.Width().ToFloat() : logicalBoxSize.Height().ToFloat();
    float boxHeight =
      horizontalWritingMode ? logicalBoxSize.Height().ToFloat() : logicalBoxSize.Width().ToFloat();
    FloatPoint borderBoxOffsetPoint = borderBoxOffset;

    auto shape = krys::SwitchOn(
      basicShape,
      [&](const style::CircleFunction &circle) -> Ref<LayoutShape>
      {
        auto boxSize = FloatSize {boxWidth, boxHeight};
        auto center = style::ResolvePosition(*circle, boxSize);
        auto radius = style::ResolveRadius(*circle, boxSize, center);

        auto logicalCenter = PhysicalPointToLogical(center, logicalBoxSize.Height().ToFloat(), writingMode);
        logicalCenter.MoveBy(borderBoxOffsetPoint);

        return CreateCircleShape(logicalCenter, radius, logicalBoxSize.Width().ToFloat());
      },
      [&](const style::EllipseFunction &ellipse) -> Ref<LayoutShape>
      {
        auto boxSize = FloatSize {boxWidth, boxHeight};
        auto center = style::ResolvePosition(*ellipse, boxSize);
        auto radii = style::ResolveRadii(*ellipse, boxSize, center);

        auto logicalCenter = PhysicalPointToLogical(center, logicalBoxSize.Height().ToFloat(), writingMode);
        logicalCenter.MoveBy(borderBoxOffsetPoint);
        if (!horizontalWritingMode)
        {
          radii = radii.TransposedSize();
        }

        return CreateEllipseShape(logicalCenter, radii, logicalBoxSize.Width().ToFloat());
      },
      [&](const style::InsetFunction &inset) -> Ref<LayoutShape>
      {
        auto left = style::Evaluate<float>(inset->Insets.Left(), boxWidth, style::ZoomNeeded {});
        auto top = style::Evaluate<float>(inset->Insets.Top(), boxHeight, style::ZoomNeeded {});

        FloatRect rect {
          left, top,
          std::max<float>(boxWidth - left
                            - style::Evaluate<float>(inset->Insets.Right(), boxWidth, style::ZoomNeeded {}),
                          0),
          std::max<float>(boxHeight - top
                            - style::Evaluate<float>(inset->Insets.Bottom(), boxHeight, style::ZoomNeeded {}),
                          0)};

        auto logicalRect = PhysicalRectToLogical(rect, logicalBoxSize.Height().ToFloat(), writingMode);
        logicalRect.MoveBy(borderBoxOffsetPoint);

        auto boxSize = FloatSize(boxWidth, boxHeight);
        auto isBlockLeftToRight = writingMode.IsBlockLeftToRight();
        const auto &radii = inset->Radii;
        auto topLeftRadius = PhysicalSizeToLogical(
          style::Evaluate<FloatSize>(horizontalWritingMode || isBlockLeftToRight ? radii.TopLeft()
                                                                                 : radii.TopRight(),
                                     boxSize, style::ZoomNeeded {}),
          writingMode);
        auto topRightRadius =
          PhysicalSizeToLogical(style::Evaluate<FloatSize>(horizontalWritingMode ? radii.TopRight()
                                                           : isBlockLeftToRight  ? radii.BottomLeft()
                                                                                 : radii.BottomRight(),
                                                           boxSize, style::ZoomNeeded {}),
                                writingMode);
        auto bottomLeftRadius =
          PhysicalSizeToLogical(style::Evaluate<FloatSize>(horizontalWritingMode ? radii.BottomLeft()
                                                           : isBlockLeftToRight  ? radii.TopRight()
                                                                                 : radii.TopLeft(),
                                                           boxSize, style::ZoomNeeded {}),
                                writingMode);
        auto bottomRightRadius =
          PhysicalSizeToLogical(style::Evaluate<FloatSize>(horizontalWritingMode ? radii.BottomRight()
                                                           : isBlockLeftToRight  ? radii.BottomRight()
                                                                                 : radii.BottomLeft(),
                                                           boxSize, style::ZoomNeeded {}),
                                writingMode);
        auto cornerRadii =
          FloatRoundedRect::Radii(topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius);
        if (ShouldFlipStartAndEndPoints(writingMode))
        {
          cornerRadii = {cornerRadii.TopRight(), cornerRadii.TopLeft(), cornerRadii.BottomRight(),
                         cornerRadii.BottomLeft()};
        }

        cornerRadii.Scale(CalcBorderRadiiConstraintScaleFor(logicalRect, cornerRadii));
        return CreateInsetShape(FloatRoundedRect {logicalRect, cornerRadii});
      },
      [&](const style::PolygonFunction &polygon) -> Ref<LayoutShape>
      {
        auto boxSize = FloatSize {boxWidth, boxHeight};

        List<FloatPoint> vertices;
        vertices.reserve(polygon->Vertices.Value.size());
        for (const auto &vertex : polygon->Vertices.Value)
        {
          vertices.push_back(
            PhysicalPointToLogical(style::Evaluate<FloatPoint>(vertex, boxSize, style::ZoomNeeded {})
                                     + FloatSize(borderBoxOffsetPoint.X(), borderBoxOffsetPoint.Y()),
                                   logicalBoxSize.Height().ToFloat(), writingMode));
        }

        return CreatePolygonShape(krys::move(vertices), logicalBoxSize.Width().ToFloat());
      },
      [&](const style::PathFunction &) -> Ref<LayoutShape> { krys_unreachable(); },
      [&](const style::ShapeFunction &) -> Ref<LayoutShape> { krys_unreachable(); });

    shape->_writingMode = writingMode;
    shape->_margin = logicalMargin;

    return shape;
  }

  Ref<const LayoutShape> LayoutShape::CreateBoxShape(const LayoutRoundedRect &roundedRect,
                                                     WritingMode writingMode, float logicalMargin) noexcept
  {
    krys_debug_assert(roundedRect.Rect().Width() >= 0 && roundedRect.Rect().Height() >= 0);

    FloatRoundedRect bounds {roundedRect};
    auto shape = AdoptRef(*new BoxLayoutShape(bounds));
    shape->_writingMode = writingMode;
    shape->_margin = logicalMargin;

    return shape;
  }

  bool LayoutShape::ShouldFlipStartAndEndPoints(WritingMode writingMode) noexcept
  {
    // Start and end flip in horizontal/vertical/sideways-rl with RTL and sideways-lr with LTR (line is either
    // right to left or bottom to top). (see https://www.w3.org/TR/css-writing-modes-4/#line-directions)
    return (writingMode.IsHorizontal() && !writingMode.IsInlineLeftToRight())
           || (!writingMode.IsHorizontal() && !writingMode.IsInlineTopToBottom());
  }
}
