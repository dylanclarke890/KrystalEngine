#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"

namespace krys::boo
{
  RenderInline::RenderInline(Type type, dom::Element &element, RenderStyle &&style) noexcept
      : RenderBoxModelObject(type, element, krys::move(style), TypeFlag::IsRenderInline, {})
  {
    SetChildrenInline(true);
    krys_debug_assert(IsRenderInline());
  }

  RenderInline::RenderInline(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderBoxModelObject(type, document, krys::move(style), TypeFlag::IsRenderInline, {})
  {
    SetChildrenInline(true);
    krys_debug_assert(IsRenderInline());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderInline::~RenderInline() noexcept = default;

  void RenderInline::UpdateFromStyle() noexcept
  {
    RenderBoxModelObject::UpdateFromStyle();

    // FIXME: Support transforms and reflections on inline flows someday.
    SetHasTransformRelatedProperty(false);
  }

  void RenderInline::Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (auto *lineLayout = layout_integration::LineLayout::Containing(*this))
    {
      lineLayout->Paint(paintInfo, paintOffset, this);
    }
  }

  utf8_stringview RenderInline::RenderName() const noexcept
  {
    if (IsRelativelyPositioned())
    {
      return u8"RenderInline (relative positioned)";
    }
    if (IsStickilyPositioned())
    {
      return u8"RenderInline (sticky positioned)";
    }
    // FIXME: Temporary hack while the new generated content system is being implemented.
    if (IsPseudoElement())
    {
      return u8"RenderInline (generated)";
    }
    if (IsAnonymous())
    {
      return u8"RenderInline (generated)";
    }
    return u8"RenderInline";
  }

  LayoutUnit RenderInline::InnerPaddingBoxWidth() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderInline::InnerPaddingBoxHeight() const noexcept
  {
    krys_unreachable();
  }

  bool IsEmptyInline(const RenderInline &renderer) noexcept
  {
    for (auto &current : ChildrenOfType<RenderObject>(renderer))
    {
      if (current.IsFloatingOrOutOfFlowPositioned())
      {
        continue;
      }
      if (auto *text = DynamicDowncast<RenderText>(current))
      {
        if (!text->ContainsOnlyCollapsibleWhitespace())
        {
          return false;
        }
        continue;
      }
      auto *renderInline = DynamicDowncast<RenderInline>(current);
      if (!renderInline || !IsEmptyInline(*renderInline))
      {
        return false;
      }
    }
    return true;
  }

  namespace
  {
    KRYS_NODISCARD LayoutUnit ComputeMargin(const RenderInline *renderer, const style::MarginEdge &margin,
                                            const style::ZoomFactor &zoomFactor) noexcept
    {
      return style::EvaluateMinimum<LayoutUnit>(
        margin,
        [&] { return std::max<LayoutUnit>(0_lu, renderer->ContainingBlock()->ContentBoxLogicalWidth()); },
        zoomFactor);
    }
  }

  LayoutUnit RenderInline::MarginLeft() const noexcept
  {
    return ComputeMargin(this, Style().MarginLeft(), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginRight() const noexcept
  {
    return ComputeMargin(this, Style().MarginRight(), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginTop() const noexcept
  {
    return ComputeMargin(this, Style().MarginTop(), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginBottom() const noexcept
  {
    return ComputeMargin(this, Style().MarginBottom(), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginStart(WritingMode writingMode) const noexcept
  {
    return ComputeMargin(this, Style().MarginStart(writingMode), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginEnd(WritingMode writingMode) const noexcept
  {
    return ComputeMargin(this, Style().MarginEnd(writingMode), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginBefore(WritingMode writingMode) const noexcept
  {
    return ComputeMargin(this, Style().MarginBefore(writingMode), Style().UsedZoomForLength());
  }

  LayoutUnit RenderInline::MarginAfter(WritingMode writingMode) const noexcept
  {
    return ComputeMargin(this, Style().MarginAfter(writingMode), Style().UsedZoomForLength());
  }

  namespace
  {
    // The rects of an inline's line boxes, or a single empty rect when it has none.
    KRYS_NODISCARD List<FloatRect> LineBoxRects(const RenderInline &renderInline) noexcept
    {
      if (auto *lineLayout = layout_integration::LineLayout::Containing(renderInline))
      {
        auto inlineBoxRects = lineLayout->CollectInlineBoxRects(renderInline);
        if (inlineBoxRects.empty())
        {
          return {FloatRect()};
        }
        return inlineBoxRects;
      }
      return {FloatRect()};
    }
  }

  void RenderInline::BoundingRects(List<LayoutRect> &rects,
                                   const LayoutPoint &accumulatedOffset) const noexcept
  {
    for (const auto &rect : LineBoxRects(*this))
    {
      auto layoutRect = LayoutRect(rect);
      layoutRect.MoveBy(accumulatedOffset);
      rects.push_back(layoutRect);
    }
    // There are no continuations: blocks in inlines are laid out in the inline's lines.
  }

  void RenderInline::AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed) const noexcept
  {
    for (const auto &rect : LineBoxRects(*this))
    {
      quads.push_back(LocalToAbsoluteQuad(rect, MapCoordinatesMode::UseTransforms, wasFixed));
    }
  }

  LayoutSize RenderInline::OffsetForInFlowPositionedInline(const RenderBox *child) const noexcept
  {
    // FIXME: This function isn't right with mixed writing modes.
    if (!IsInFlowPositioned())
    {
      krys_debug_assert(false);
      return {};
    }

    if (!HasLayer())
    {
      // It looks like we are inflow positioned but no layer created yet. It essentially means we don't have
      // an position offset yet.
      return {};
    }

    // When we have an enclosing relpositioned inline, we need to add in the offset of the first line
    // box from the rest of the content, but only in the cases where we know we're positioned
    // relative to the inline itself.
    auto inlinePosition = Layer()->StaticInlinePosition();
    auto blockPosition = Layer()->StaticBlockPosition();
    if (layout_integration::LineLayout::Containing(*this))
    {
      if (!LayoutBox())
      {
        // Repaint may be issued on subtrees during content mutation with newly inserted renderers.
        krys_debug_assert(NeedsLayout());
        return {};
      }
      if (auto inlineBox = inline_iterator::LineLeftmostInlineBoxFor(*this))
      {
        inlinePosition = LayoutUnit::FromFloatRound(inlineBox->LogicalLeftIgnoringInlineDirection());
        blockPosition = LayoutUnit(inlineBox->LogicalTop());
      }
      else if (auto *blockContainer = ContainingBlock())
      {
        // This must be a block with no in-flow content e.g. <div><span><abs pos box></span></div> where we
        // don't construct any display box at all.
        auto contentBoxLocation = blockContainer->ContentBoxLocation();
        inlinePosition = contentBoxLocation.X();
        blockPosition = contentBoxLocation.Y();
      }
    }

    // Per http://www.w3.org/TR/CSS2/visudet.html#abs-non-replaced-width an absolute positioned box with a
    // static position should locate itself as though it is a normal flow box in relation to its containing
    // block.
    LayoutSize logicalOffset;
    const auto &childStyle = child->Style();
    auto isHorizontal = GetWritingMode().IsHorizontal();
    if (!childStyle.HasStaticInlinePosition(isHorizontal) || !childStyle.GetPositionArea().IsNone()
        || childStyle.GetJustifySelf().IsAnchorCenter())
    {
      logicalOffset.SetWidth(inlinePosition);
    }

    if (!childStyle.HasStaticBlockPosition(isHorizontal) || !childStyle.GetPositionArea().IsNone()
        || childStyle.GetAlignSelf().IsAnchorCenter())
    {
      logicalOffset.SetHeight(blockPosition);
    }

    return isHorizontal ? logicalOffset : logicalOffset.TransposedSize();
  }

  LayoutSize RenderInline::OffsetFromContainer(KRYS_MAYBE_UNUSED const RenderElement &container,
                                               const LayoutPoint &, bool *offsetDependsOnPoint) const noexcept
  {
    krys_debug_assert(&container == Container());

    LayoutSize offset;
    if (IsInFlowPositioned())
    {
      offset += OffsetForInFlowPosition();
    }

    if (auto *box = DynamicDowncast<RenderBox>(container))
    {
      offset -= ToLayoutSize(LayoutPoint(box->GetScrollPosition()));
    }

    if (offsetDependsOnPoint)
    {
      // FIXME: Fragmented flows make the offset depend on the point too, once there are those.
      *offsetDependsOnPoint = Is<RenderBox>(container) && container.GetWritingMode().IsBlockFlipped();
    }

    return offset;
  }

  void RenderInline::MapLocalToContainer(const RenderLayerModelObject *ancestorContainer,
                                         TransformState &transformState, EnumSet<MapCoordinatesMode> mode,
                                         bool *wasFixed) const noexcept
  {
    if (ancestorContainer == this)
    {
      return;
    }

    if (View().FrameView().LayoutContext().IsPaintOffsetCacheEnabled() && !ancestorContainer)
    {
      auto *layoutState = View().FrameView().LayoutContext().LayoutState();
      auto offset = layoutState->PaintOffset();
      if (Style().HasInFlowPosition() && Layer())
      {
        offset += Layer()->OffsetForInFlowPosition();
      }
      transformState.Move(offset);
      return;
    }

    bool containerSkipped = false;
    auto *container = Container(ancestorContainer, containerSkipped);
    if (!container)
    {
      return;
    }

    if (mode.Contains(MapCoordinatesMode::ApplyContainerFlip))
    {
      if (auto *box = DynamicDowncast<RenderBox>(*container))
      {
        if (container->GetWritingMode().IsBlockFlipped())
        {
          auto centerPoint = LayoutPoint(transformState.MappedPoint());
          transformState.Move(box->FlipForWritingMode(centerPoint) - centerPoint);
        }
        mode.Remove(MapCoordinatesMode::ApplyContainerFlip);
      }
    }

    auto containerOffset = OffsetFromContainer(*container, LayoutPoint(transformState.MappedPoint()));

    PushOntoTransformState(transformState, mode, ancestorContainer, container, containerOffset,
                           containerSkipped);
    if (containerSkipped)
    {
      return;
    }

    container->MapLocalToContainer(ancestorContainer, transformState, mode, wasFixed);
  }

  IntRect RenderInline::LinesBoundingBox() const noexcept
  {
    if (auto *layout = layout_integration::LineLayout::Containing(*this))
    {
      if (!LayoutBox() || !layout->Contains(*this))
      {
        // Repaint may be issued on subtrees during content mutation with newly inserted renderers
        // (or we just forgot to initiate layout before querying geometry on stale content after moving inline
        // boxes between blocks).
        krys_debug_assert(NeedsLayout());
        return {};
      }
      // FIXME: SVG inlines build the bounding box from their inline boxes, once there's SVG.
      return EnclosingIntRect(layout->EnclosingBorderBoxRectFor(*this));
    }

    // Without a line layout (the legacy line boxes are not supported), the inline has no lines.
    return {};
  }

  bool RenderInline::RequiresLayer() const noexcept
  {
    // FIXME: Running accelerated animations too, once there are any.
    return IsInFlowPositioned() || CreatesGroup() || HasClipPath()
           || Style().GetWillChange().CanCreateStackingContext()
           || RequiresRenderingConsolidationForViewTransition();
  }

  void RenderInline::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    RenderBoxModelObject::StyleWillChange(diff, newStyle);

    // RenderInlines forward their absolute positioned descendants to their (non-anonymous) containing block.
    // Check if this non-anonymous containing block can hold the absolute positioned elements when the inline
    // is no longer positioned.
    auto *container = ContainingBlock();
    if (!container)
    {
      return;
    }

    const RenderStyle *oldStyle = HasInitializedStyle() ? &Style() : nullptr;
    if (oldStyle)
    {
      RemoveOutOfFlowBoxesIfNeededOnStyleChange(*container, *oldStyle, newStyle);
    }
  }
}
