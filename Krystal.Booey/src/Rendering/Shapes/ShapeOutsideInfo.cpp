#include "Krystal.Booey/Rendering/Shapes/ShapeOutsideInfo.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/FloatingObjects.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Shapes/BoxLayoutShape.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <cmath>

namespace krys::boo
{
  LayoutRect ShapeOutsideInfo::ComputedShapePhysicalBoundingBox() const noexcept
  {
    LayoutRect physicalBoundingBox = ComputedShape().ShapeMarginLogicalBoundingBox();
    if (_renderer.Style().GetWritingMode().IsBlockFlipped())
    {
      physicalBoundingBox.SetY(_renderer.LogicalHeight() - physicalBoundingBox.MaxY());
    }
    if (!_renderer.IsHorizontalWritingMode())
    {
      physicalBoundingBox = physicalBoundingBox.TransposedRect();
    }
    return physicalBoundingBox;
  }

  FloatPoint ShapeOutsideInfo::ShapeToRendererPoint(const FloatPoint &point) const noexcept
  {
    FloatPoint result = point;
    if (_renderer.Style().GetWritingMode().IsBlockFlipped())
    {
      result.SetY(_renderer.LogicalHeight().ToFloat() - result.Y());
    }
    if (!_renderer.IsHorizontalWritingMode())
    {
      result = result.TransposedPoint();
    }
    return result;
  }

  namespace
  {
    KRYS_NODISCARD LayoutSize ComputeLogicalBoxSize(const RenderBox &renderer,
                                                    bool isHorizontalWritingMode) noexcept
    {
      const auto &shapeOutside = renderer.Style().GetShapeOutside();
      auto size = isHorizontalWritingMode ? renderer.Size() : renderer.Size().TransposedSize();
      switch (shapeOutside.EffectiveCSSBox())
      {
        case CSSBoxType::MarginBox:
          if (isHorizontalWritingMode)
          {
            size.Expand(renderer.HorizontalMarginExtent(), renderer.VerticalMarginExtent());
          }
          else
          {
            size.Expand(renderer.VerticalMarginExtent(), renderer.HorizontalMarginExtent());
          }
          break;
        case CSSBoxType::BorderBox: break;
        case CSSBoxType::PaddingBox:
          if (isHorizontalWritingMode)
          {
            size.Shrink(renderer.HorizontalBorderExtent(), renderer.VerticalBorderExtent());
          }
          else
          {
            size.Shrink(renderer.VerticalBorderExtent(), renderer.HorizontalBorderExtent());
          }
          break;
        case CSSBoxType::ContentBox:
          if (isHorizontalWritingMode)
          {
            size.Shrink(renderer.HorizontalBorderAndPaddingExtent(),
                        renderer.VerticalBorderAndPaddingExtent());
          }
          else
          {
            size.Shrink(renderer.VerticalBorderAndPaddingExtent(),
                        renderer.HorizontalBorderAndPaddingExtent());
          }
          break;
        case CSSBoxType::FillBox:
        case CSSBoxType::StrokeBox:
        case CSSBoxType::ViewBox:
        case CSSBoxType::BoxMissing: krys_debug_assert(false); break;
      }
      return size;
    }

    KRYS_NODISCARD LayoutUnit BorderBeforeInWritingMode(const RenderBox &renderer,
                                                        WritingMode writingMode) noexcept
    {
      switch (writingMode.BlockDirection())
      {
        case FlowDirection::TopToBottom: return renderer.BorderTop();
        case FlowDirection::BottomToTop: return renderer.BorderBottom();
        case FlowDirection::LeftToRight: return renderer.BorderLeft();
        case FlowDirection::RightToLeft: return renderer.BorderRight();
      }

      krys_debug_assert(false);
      return renderer.BorderBefore();
    }

    KRYS_NODISCARD LayoutUnit BorderAndPaddingBeforeInWritingMode(const RenderBox &renderer,
                                                                  WritingMode writingMode) noexcept
    {
      switch (writingMode.BlockDirection())
      {
        case FlowDirection::TopToBottom: return renderer.BorderTop() + renderer.PaddingTop();
        case FlowDirection::BottomToTop: return renderer.BorderBottom() + renderer.PaddingBottom();
        case FlowDirection::LeftToRight: return renderer.BorderLeft() + renderer.PaddingLeft();
        case FlowDirection::RightToLeft: return renderer.BorderRight() + renderer.PaddingRight();
      }

      krys_debug_assert(false);
      return renderer.BorderAndPaddingBefore();
    }

    KRYS_NODISCARD LayoutUnit LogicalTopOffset(const RenderBox &renderer) noexcept
    {
      switch (renderer.Style().GetShapeOutside().EffectiveCSSBox())
      {
        case CSSBoxType::MarginBox:
          return -renderer.MarginBefore(renderer.ContainingBlock()->Style().GetWritingMode());
        case CSSBoxType::BorderBox: return 0_lu;
        case CSSBoxType::PaddingBox:
          return BorderBeforeInWritingMode(renderer, renderer.ContainingBlock()->Style().GetWritingMode());
        case CSSBoxType::ContentBox:
          return BorderAndPaddingBeforeInWritingMode(renderer,
                                                     renderer.ContainingBlock()->Style().GetWritingMode());
        case CSSBoxType::FillBox:
        case CSSBoxType::StrokeBox:
        case CSSBoxType::ViewBox:
        case CSSBoxType::BoxMissing: break;
      }

      krys_debug_assert(false);
      return 0_lu;
    }

    KRYS_NODISCARD LayoutUnit BorderStartWithStyleForWritingMode(const RenderBox &renderer,
                                                                 const WritingMode writingMode) noexcept
    {
      if (writingMode.IsHorizontal())
      {
        if (writingMode.IsInlineLeftToRight())
        {
          return renderer.BorderLeft();
        }

        return renderer.BorderRight();
      }
      if (writingMode.IsInlineTopToBottom())
      {
        return renderer.BorderTop();
      }

      return renderer.BorderBottom();
    }

    KRYS_NODISCARD LayoutUnit BorderAndPaddingStartWithStyleForWritingMode(
      const RenderBox &renderer, const WritingMode writingMode) noexcept
    {
      if (writingMode.IsHorizontal())
      {
        if (writingMode.IsInlineLeftToRight())
        {
          return renderer.BorderLeft() + renderer.PaddingLeft();
        }

        return renderer.BorderRight() + renderer.PaddingRight();
      }
      if (writingMode.IsInlineTopToBottom())
      {
        return renderer.BorderTop() + renderer.PaddingTop();
      }

      return renderer.BorderBottom() + renderer.PaddingBottom();
    }

    KRYS_NODISCARD LayoutUnit LogicalLeftOffset(const RenderBox &renderer) noexcept
    {
      // FIXME: A fragment container's offset is 0 (isRenderFragmentContainer), once there are fragmented
      // flows.

      switch (renderer.Style().GetShapeOutside().EffectiveCSSBox())
      {
        case CSSBoxType::MarginBox:
          return -renderer.MarginStart(renderer.ContainingBlock()->Style().GetWritingMode());
        case CSSBoxType::BorderBox: return 0_lu;
        case CSSBoxType::PaddingBox:
          return BorderStartWithStyleForWritingMode(renderer,
                                                    renderer.ContainingBlock()->Style().GetWritingMode());
        case CSSBoxType::ContentBox:
          return BorderAndPaddingStartWithStyleForWritingMode(
            renderer, renderer.ContainingBlock()->Style().GetWritingMode());
        case CSSBoxType::FillBox:
        case CSSBoxType::StrokeBox:
        case CSSBoxType::ViewBox:
        case CSSBoxType::BoxMissing: break;
      }

      krys_debug_assert(false);
      return 0_lu;
    }
  }

  void ShapeOutsideInfo::InvalidateForSizeChangeIfNeeded() noexcept
  {
    auto newSize = ComputeLogicalBoxSize(_renderer, _renderer.ContainingBlock()->IsHorizontalWritingMode());
    if (_cachedShapeLogicalSize == newSize)
    {
      return;
    }

    MarkShapeAsDirty();
    _cachedShapeLogicalSize = newSize;
  }

  Ref<const LayoutShape> MakeShapeForShapeOutside(const RenderBox &renderer) noexcept
  {
    const auto &style = renderer.Style();
    const auto &containingBlock = *renderer.ContainingBlock();
    auto writingMode = containingBlock.Style().GetWritingMode();
    bool isHorizontalWritingMode = containingBlock.IsHorizontalWritingMode();
    const auto &shapeOutside = style.GetShapeOutside();

    auto boxSize = ComputeLogicalBoxSize(renderer, isHorizontalWritingMode);

    auto logicalMargin = [&]
    {
      auto shapeMargin =
        style::Evaluate<LayoutUnit>(style.GetShapeMargin(), containingBlock.ContentBoxLogicalWidth(),
                                    style::ZoomNeeded {})
          .ToFloat();
      return std::isnan(shapeMargin) ? 0.0f : shapeMargin;
    }();

    return shapeOutside.SwitchOn(
      [&](const style::ShapeOutside::Shape &shape)
      {
        auto offset = LayoutPoint {LogicalLeftOffset(renderer), LogicalTopOffset(renderer)};
        return LayoutShape::CreateShape(shape, offset, boxSize, writingMode, logicalMargin);
      },
      [&](const style::ShapeOutside::ShapeAndShapeBox &shapeAndShapeBox)
      {
        auto offset = LayoutPoint {LogicalLeftOffset(renderer), LogicalTopOffset(renderer)};
        return LayoutShape::CreateShape(shapeAndShapeBox.Shape, offset, boxSize, writingMode, logicalMargin);
      },
      [&](const style::ShapeOutside::Image &)
      {
        // FIXME: The image's raster shape (createRasterShape, with the shape-image-threshold).
        // ShapeOutsideInfo isn't enabled for images until then.
        krys_debug_assert(false);
        return LayoutShape::CreateBoxShape(LayoutRoundedRect {LayoutRect {}}, writingMode, 0);
      },
      [&](const style::ShapeOutside::ShapeBox &)
      {
        auto shapeRect = ComputeRoundedRectForBoxShape(shapeOutside.EffectiveCSSBox(), renderer);
        auto flipForWritingAndInlineDirection = [&]
        {
          // FIXME: We should consider this moving to LayoutRoundedRect::transposedRect.
          if (!isHorizontalWritingMode)
          {
            shapeRect = shapeRect.TransposedRect();
            auto radiiForBlockDirection = shapeRect.GetRadii();
            if (writingMode.IsLineOverLeft()) // sideways-lr
            {
              shapeRect.SetRadii({radiiForBlockDirection.BottomLeft(), radiiForBlockDirection.TopLeft(),
                                  radiiForBlockDirection.BottomRight(), radiiForBlockDirection.TopRight()});
            }
            else if (writingMode.IsBlockLeftToRight()) // vertical-lr
            {
              shapeRect.SetRadii({radiiForBlockDirection.TopLeft(), radiiForBlockDirection.BottomLeft(),
                                  radiiForBlockDirection.TopRight(), radiiForBlockDirection.BottomRight()});
            }
            else // vertical-rl, sideways-rl
            {
              shapeRect.SetRadii({radiiForBlockDirection.TopRight(), radiiForBlockDirection.BottomRight(),
                                  radiiForBlockDirection.TopLeft(), radiiForBlockDirection.BottomLeft()});
            }
          }
          if (writingMode.IsBidiRTL())
          {
            auto radii = shapeRect.GetRadii();
            shapeRect.SetRadii({radii.TopRight(), radii.TopLeft(), radii.BottomRight(), radii.BottomLeft()});
          }
        };
        flipForWritingAndInlineDirection();
        return LayoutShape::CreateBoxShape(shapeRect, writingMode, logicalMargin);
      },
      [&](const css::keywords::None &)
      {
        krys_debug_assert(false);
        return LayoutShape::CreateBoxShape(LayoutRoundedRect {LayoutRect {}}, writingMode, 0);
      });
  }

  const LayoutShape &ShapeOutsideInfo::ComputedShape() const noexcept
  {
    if (!_shape)
    {
      _shape = MakeShapeForShapeOutside(_renderer);
    }

    return *_shape;
  }

  bool ShapeOutsideInfo::IsEnabledFor(const RenderBox &box) noexcept
  {
    if (!box.IsFloating())
    {
      return false;
    }

    return box.Style().GetShapeOutside().SwitchOn(
      [](const css::keywords::None &) { return false; }, [](const style::ShapeOutside::Shape &)
      { return true; }, [](const style::ShapeOutside::ShapeBox &) { return true; },
      [](const style::ShapeOutside::ShapeAndShapeBox &) { return true; },
      // FIXME: Valid images whose origin is clean (checkShapeImageOrigin), once image shapes can read pixels:
      // image buffers record display lists (the raster shape draws the image into one to read its alpha).
      [](const style::ShapeOutside::Image &) { return false; });
  }

  ShapeOutsideDeltas
    ShapeOutsideInfo::ComputeDeltasForContainingBlockLine(const RenderBlockFlow &containingBlock,
                                                          const FloatingObject &floatingObject,
                                                          LayoutUnit lineTop, LayoutUnit lineHeight) noexcept
  {
    // If we never constructed this shape during layout, we propably don't need to know about it outside of
    // layout in the context of "containing block line".
    if (!_shape && !containingBlock.View().FrameView().LayoutContext().IsInLayout())
    {
      return {};
    }

    krys_debug_assert(lineHeight >= 0);
    LayoutUnit borderBoxTop =
      containingBlock.LogicalTopForFloat(floatingObject) + containingBlock.MarginBeforeForChild(_renderer);
    LayoutUnit borderBoxLineTop = lineTop - borderBoxTop;

    if (IsShapeDirty() || !_shapeOutsideDeltas.IsForLine(borderBoxLineTop, lineHeight))
    {
      LayoutUnit floatMarginBoxWidth =
        std::max<LayoutUnit>(0_lu, containingBlock.LogicalWidthForFloat(floatingObject));

      if (ComputedShape().LineOverlapsShapeMarginBounds(borderBoxLineTop, lineHeight))
      {
        LineSegment segment = ComputedShape().GetExcludedInterval(
          borderBoxLineTop, std::min(lineHeight, ShapeLogicalBottom() - borderBoxLineTop));
        if (segment.IsValid)
        {
          LayoutUnit logicalLeftMargin = containingBlock.Style().GetWritingMode().IsLogicalLeftInlineStart()
                                           ? containingBlock.MarginStartForChild(_renderer)
                                           : containingBlock.MarginEndForChild(_renderer);
          LayoutUnit rawLeftMarginBoxDelta {segment.LogicalLeft + logicalLeftMargin.ToFloat()};
          LayoutUnit leftMarginBoxDelta =
            std::clamp<LayoutUnit>(rawLeftMarginBoxDelta, 0_lu, floatMarginBoxWidth);

          LayoutUnit logicalRightMargin = containingBlock.Style().GetWritingMode().IsLogicalLeftInlineStart()
                                            ? containingBlock.MarginEndForChild(_renderer)
                                            : containingBlock.MarginStartForChild(_renderer);
          LayoutUnit rawRightMarginBoxDelta {segment.LogicalRight
                                             - containingBlock.LogicalWidthForChild(_renderer).ToFloat()
                                             - logicalRightMargin.ToFloat()};
          LayoutUnit rightMarginBoxDelta =
            std::clamp<LayoutUnit>(rawRightMarginBoxDelta, -floatMarginBoxWidth, 0_lu);

          _shapeOutsideDeltas =
            ShapeOutsideDeltas(leftMarginBoxDelta, rightMarginBoxDelta, true, borderBoxLineTop, lineHeight);
          return _shapeOutsideDeltas;
        }
      }

      // Lines that do not overlap the shape should act as if the float
      // wasn't there for layout purposes. So we set the deltas to remove the
      // entire width of the float
      _shapeOutsideDeltas =
        ShapeOutsideDeltas(floatMarginBoxWidth, -floatMarginBoxWidth, false, borderBoxLineTop, lineHeight);
    }

    return _shapeOutsideDeltas;
  }
}
