#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderGeometryMap.hpp"
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

  bool RenderInline::MayAffectLayout() const noexcept
  {
    const auto *parentStyle = &Parent()->Style();
    auto *parentRenderInline = DynamicDowncast<RenderInline>(*Parent());
    auto hasHardLineBreakChildOnly = FirstChild() && FirstChild() == LastChild() && FirstChild()->IsBR();
    bool checkFonts = GetDocument().InNoQuirksMode();
    auto mayAffectLayout =
      (parentRenderInline && parentRenderInline->MayAffectLayout())
      || (parentRenderInline && !HoldsAlternative<css::keywords::Baseline>(parentStyle->GetVerticalAlign()))
      || !HoldsAlternative<css::keywords::Baseline>(Style().GetVerticalAlign())
      || !Style().GetTextEmphasisStyle().IsNone()
      || (checkFonts
          && (!parentStyle->GetFontCascade().MetricsOfPrimaryFont().HasIdenticalAscentDescentAndLineGap(
                Style().GetFontCascade().MetricsOfPrimaryFont())
              || parentStyle->SpecifiedLineHeight() != Style().SpecifiedLineHeight()))
      || hasHardLineBreakChildOnly;

    if (!mayAffectLayout && checkFonts)
    {
      // Have to check the first line style as well.
      parentStyle = &Parent()->FirstLineStyle();
      const auto &childStyle = FirstLineStyle();
      mayAffectLayout =
        !parentStyle->GetFontCascade().MetricsOfPrimaryFont().HasIdenticalAscentDescentAndLineGap(
          childStyle.GetFontCascade().MetricsOfPrimaryFont())
        || !HoldsAlternative<css::keywords::Baseline>(childStyle.GetVerticalAlign())
        || parentStyle->SpecifiedLineHeight() != childStyle.SpecifiedLineHeight();
    }
    return mayAffectLayout;
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

  void RenderInline::CollectLineBoxRects(List<LayoutRect> &rects,
                                         const LayoutPoint &additionalOffset) const noexcept
  {
    for (const auto &rect : LineBoxRects(*this))
    {
      if (rect.IsEmpty())
      {
        continue;
      }
      auto layoutRect = LayoutRect(rect);
      layoutRect.MoveBy(additionalOffset);
      rects.push_back(layoutRect);
    }
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

  const RenderElement *RenderInline::PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                            RenderGeometryMap &geometryMap) const noexcept
  {
    krys_debug_assert(ancestorToStopAt != this);

    bool ancestorSkipped;
    RenderElement *container = this->Container(ancestorToStopAt, ancestorSkipped);
    if (!container)
    {
      return nullptr;
    }

    PushOntoGeometryMap(geometryMap, ancestorToStopAt, container, ancestorSkipped);

    return ancestorSkipped ? ancestorToStopAt : container;
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

  LayoutRect RenderInline::LinesVisualOverflowBoundingBox() const noexcept
  {
    if (auto *layout = layout_integration::LineLayout::Containing(*this))
    {
      if (!LayoutBox())
      {
        // Repaint may be issued on subtrees during content mutation with newly inserted renderers.
        krys_debug_assert(NeedsLayout());
        return {};
      }
      return layout->InkOverflowBoundingBoxRectFor(*this);
    }

    // Without a line layout (the legacy line boxes are not supported), the inline has no lines.
    return {};
  }

#pragma region Repaint

  LayoutRect RenderInline::ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                               VisibleRectContext context) const noexcept
  {
    // Only first-letter renderers are allowed in here during layout. They mutate the tree triggering
    // repaints.
#if KRYS_DEBUG
    auto insideSelfPaintingInlineBox = [&]
    {
      if (HasSelfPaintingLayer())
      {
        return true;
      }
      auto *containingBlock = ContainingBlock();
      for (auto *ancestor = Parent(); ancestor && ancestor != containingBlock; ancestor = ancestor->Parent())
      {
        if (ancestor->HasSelfPaintingLayer())
        {
          return true;
        }
      }
      return false;
    };
    krys_debug_assert(!View().FrameView().LayoutContext().IsPaintOffsetCacheEnabled()
                      || Style().GetPseudoElementType() == PseudoElementType::FirstLetter
                      || insideSelfPaintingInlineBox());
#endif

    auto knownEmpty = [&]
    {
      // FIXME: Continuations, once there are any.
      if (layout_integration::LineLayout::Containing(*this))
      {
        return false;
      }
      return true;
    };

    if (knownEmpty())
    {
      return LayoutRect();
    }

    auto repaintRect = LinesVisualOverflowBoundingBox();
    bool hitRepaintContainer = false;

    // We need to add in the in-flow position offsets of any inlines (including us) up to our
    // containing block.
    RenderBlock *containingBlock = ContainingBlock();
    for (const RenderElement *inlineFlow = this; inlineFlow; inlineFlow = inlineFlow->Parent())
    {
      auto *renderInline = DynamicDowncast<RenderInline>(*inlineFlow);
      if (!renderInline || inlineFlow == containingBlock)
      {
        break;
      }
      if (inlineFlow == repaintContainer)
      {
        hitRepaintContainer = true;
        break;
      }
      if (inlineFlow->Style().HasInFlowPosition() && inlineFlow->HasLayer())
      {
        repaintRect.Move(renderInline->Layer()->OffsetForInFlowPosition());
      }
    }

    LayoutUnit outlineSize {Style().OutlineSize()};
    repaintRect.Inflate(outlineSize);

    if (hitRepaintContainer || !containingBlock)
    {
      return repaintRect;
    }

    auto rects = RepaintRects {repaintRect};

    if (containingBlock->HasNonVisibleOverflow())
    {
      containingBlock->ApplyCachedClipAndScrollPosition(rects, repaintContainer, context);
    }

    rects = containingBlock->ComputeRects(rects, repaintContainer, context);
    repaintRect = rects.ClippedOverflowRect;

    if (outlineSize)
    {
      for (auto &child : ChildrenOfType<RenderElement>(*this))
      {
        repaintRect.Unite(child.RectWithOutlineForRepaint(repaintContainer, outlineSize));
      }
      // FIXME: The continuation, once there are any.
    }

    return repaintRect;
  }

  auto RenderInline::RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                   RepaintOutlineBounds) const noexcept -> RepaintRects
  {
    // RepaintOutlineBounds is unused for inlines.
    return RepaintRects {ClippedOverflowRect(repaintContainer, VisibleRectContextForRepaint())};
  }

  LayoutRect RenderInline::RectWithOutlineForRepaint(const RenderLayerModelObject *repaintContainer,
                                                     LayoutUnit outlineWidth) const noexcept
  {
    LayoutRect r(RenderBoxModelObject::RectWithOutlineForRepaint(repaintContainer, outlineWidth));
    for (auto &child : ChildrenOfType<RenderElement>(*this))
    {
      r.Unite(child.RectWithOutlineForRepaint(repaintContainer, outlineWidth));
    }
    return r;
  }

  auto RenderInline::ComputeVisibleRectsUsingPaintOffset(const RepaintRects &rects) const noexcept
    -> RepaintRects
  {
    auto adjustedRects = rects;
    auto *layoutState = View().FrameView().LayoutContext().LayoutState();
    if (Style().HasInFlowPosition() && Layer())
    {
      adjustedRects.Move(Layer()->OffsetForInFlowPosition());
    }
    adjustedRects.Move(layoutState->PaintOffset());
    if (layoutState->IsClipped())
    {
      adjustedRects.ClippedOverflowRect.Intersect(layoutState->ClipRect());
    }
    return adjustedRects;
  }

  auto RenderInline::ComputeVisibleRectsInContainer(const RepaintRects &rects,
                                                    const RenderLayerModelObject *container,
                                                    VisibleRectContext context) const noexcept
    -> Maybe<RepaintRects>
  {
    // Repaint offset cache is only valid for root-relative repainting
    if (View().FrameView().LayoutContext().IsPaintOffsetCacheEnabled() && !container
        && !context.Options.Contains(VisibleRectContext::Option::UseEdgeInclusiveIntersection))
    {
      return ComputeVisibleRectsUsingPaintOffset(rects);
    }

    if (container == this)
    {
      return rects;
    }

    bool containerSkipped;
    RenderElement *localContainer = Container(container, containerSkipped);
    if (!localContainer)
    {
      return rects;
    }

    auto adjustedRects = rects;
    if (Style().HasInFlowPosition() && Layer())
    {
      // Apply the in-flow position offset when invalidating a rectangle. The layer
      // is translated, but the render box isn't, so we need to do this to get the
      // right dirty rect. Since this is called from RenderObject::setStyle, the relative or sticky position
      // flag on the RenderObject has been cleared, so use the one on the style().
      auto offsetForInFlowPosition = Layer()->OffsetForInFlowPosition();
      adjustedRects.Move(offsetForInFlowPosition);
    }

    if (localContainer->HasNonVisibleOverflow())
    {
      // FIXME: Respect the value of context.options.
      context.Options.Add(VisibleRectContext::Option::ApplyCompositedContainerScrolls);
      bool isEmpty = !Downcast<RenderLayerModelObject>(*localContainer)
                        .ApplyCachedClipAndScrollPosition(adjustedRects, container, context);
      if (isEmpty)
      {
        if (context.Options.Contains(VisibleRectContext::Option::UseEdgeInclusiveIntersection))
        {
          return null;
        }
        return adjustedRects;
      }
    }

    if (containerSkipped)
    {
      // If the repaintContainer is below o, then we need to map the rect into repaintContainer's coordinates.
      auto containerOffset = container->OffsetFromAncestorContainer(*localContainer);
      adjustedRects.Move(-containerOffset);
      return adjustedRects;
    }

    return localContainer->ComputeVisibleRectsInContainer(adjustedRects, container, context);
  }

#pragma endregion

  bool RenderInline::RequiresLayer() const noexcept
  {
    return IsInFlowPositioned() || CreatesGroup() || HasClipPath()
           || Style().GetWillChange().CanCreateStackingContext() || HasRunningAcceleratedAnimations()
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

  dom::PositionWithAffinity RenderInline::PositionForPoint(const LayoutPoint &point,
                                                           HitTestSource source) noexcept
  {
    // FIXME: Translating the point to a continuation's block, once there are continuations.
    return ContainingBlock()->PositionForPoint(point, source);
  }

  bool RenderInline::NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                                 const HitTestLocation &locationInContainer,
                                 const LayoutPoint &accumulatedOffset, HitTestAction hitTestAction) noexcept
  {
    krys_debug_assert(Layer());
    if (auto *lineLayout = layout_integration::LineLayout::Containing(*this))
    {
      return lineLayout->HitTest(request, result, locationInContainer, accumulatedOffset, hitTestAction,
                                 this);
    }
    return false;
  }

  void RenderInline::UpdateHitTestResult(HitTestResult &result, const LayoutPoint &point) const noexcept
  {
    if (result.InnerNode())
    {
      return;
    }

    // There are no continuations whose points would need moving into the principal renderer's containing
    // block.
    LayoutPoint localPoint(point);
    if (auto *node = NodeForHitTest())
    {
      result.SetInnerNode(node);
      if (!result.InnerNonSharedNode())
      {
        result.SetInnerNonSharedNode(node);
      }
      result.SetPseudoElementIdentifier(Style().GetPseudoElementIdentifier());
      result.SetLocalPoint(localPoint);
    }
  }

  void RenderInline::ImageChanged(WrappedImagePtr, const IntRect *) noexcept
  {
    if (!Parent())
    {
      return;
    }

    // FIXME: Tell the document an image loaded (didLoadImage), once there are paint milestones.

    // FIXME: We can do better.
    Repaint();
  }
}
