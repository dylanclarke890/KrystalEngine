#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxGeometryUpdater.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/InlineWalker.hpp"
#include "Krystal.Booey/Rendering/LegacyRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderDeprecatedFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTextControlMultiLine.hpp"
#include "Krystal.Booey/Rendering/RenderTextControlSingleLine.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Shapes/ShapeOutsideInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"

namespace krys::boo::layout_integration
{
  namespace
  {
    enum class SnapDirection : uint8
    {
      Floor,
      Ceil,
      Round
    };

    // Subpixel inline layout is off, so values snap to integers.
    KRYS_NODISCARD LayoutUnit SnapToInt(LayoutUnit value,
                                        SnapDirection direction = SnapDirection::Round) noexcept
    {
      switch (direction)
      {
        case SnapDirection::Floor: return LayoutUnit {std::floor(value.ToFloat())};
        case SnapDirection::Ceil:  return LayoutUnit {std::ceil(value.ToFloat())};
        case SnapDirection::Round: return LayoutUnit {std::round(value.ToFloat())};
      }
      krys_debug_assert(false);
      return {};
    }

    KRYS_NODISCARD float Ascent(const RenderObject &renderer) noexcept
    {
      return static_cast<float>(renderer.FirstLineStyle().MetricsOfPrimaryFont().IntAscent());
    }

    KRYS_NODISCARD LayoutUnit UsedValueOrZero(const style::MarginEdge &marginEdge,
                                              Maybe<LayoutUnit> availableWidth,
                                              const style::ZoomFactor &zoomFactor) noexcept
    {
      if (auto fixed = marginEdge.TryFixed())
      {
        return LayoutUnit {fixed->ResolveZoom(zoomFactor)};
      }

      if (marginEdge.IsAuto() || !availableWidth)
      {
        return {};
      }

      return style::EvaluateMinimum<LayoutUnit>(marginEdge, *availableWidth, zoomFactor);
    }

    KRYS_NODISCARD LayoutUnit UsedValueOrZero(const style::PaddingEdge &paddingEdge,
                                              Maybe<LayoutUnit> availableWidth,
                                              const style::ZoomFactor &usedZoom) noexcept
    {
      if (auto fixed = paddingEdge.TryFixed())
      {
        return LayoutUnit {fixed->ResolveZoom(usedZoom)};
      }

      if (!availableWidth)
      {
        return {};
      }

      return style::EvaluateMinimum<LayoutUnit>(paddingEdge, *availableWidth, usedZoom);
    }

    // FIXME: The fieldset's intrinsic border (intrinsicBorderForFieldset), once there are form controls.
    inline void AdjustBorderForTableAndFieldset(const RenderBoxModelObject &renderer,
                                                RectEdges<LayoutUnit> &borderWidths) noexcept
    {
      if (auto *table = DynamicDowncast<RenderTable>(renderer); table && table->CollapseBorders())
      {
        borderWidths = table->BorderWidths();
        return;
      }

      if (auto *tableCell = DynamicDowncast<RenderTableCell>(renderer);
          tableCell && tableCell->Table()->CollapseBorders())
      {
        borderWidths = tableCell->BorderWidths();
        return;
      }
    }

    KRYS_NODISCARD inline layout::BoxGeometry::VerticalEdges
      IntrinsicPaddingForTableCell(const RenderBox &renderer) noexcept
    {
      if (auto *tableCell = DynamicDowncast<RenderTableCell>(renderer))
      {
        return {tableCell->IntrinsicPaddingBefore(), tableCell->IntrinsicPaddingAfter()};
      }
      return {};
    }

    KRYS_NODISCARD inline LayoutUnit ContentLogicalWidthForRenderer(const RenderBox &renderer) noexcept
    {
      return renderer.Parent()->GetWritingMode().IsHorizontal() ? renderer.ContentBoxWidth()
                                                                : renderer.ContentBoxHeight();
    }

    KRYS_NODISCARD inline LayoutUnit ContentLogicalHeightForRenderer(const RenderBox &renderer) noexcept
    {
      return renderer.Parent()->GetWritingMode().IsHorizontal() ? renderer.ContentBoxHeight()
                                                                : renderer.ContentBoxWidth();
    }

    KRYS_NODISCARD inline LayoutSize ScrollbarLogicalSize(const RenderBox &renderer) noexcept
    {
      // Scrollbars eat into the padding box area. They never stretch the border box but they may shrink the
      // padding box. In the render tree, RenderBox::ContentBoxWidth/Height values are adjusted to accommodate
      // the scrollbar width/height. e.g. <div style="width: 10px; overflow: scroll;">content</div>,
      // RenderBox::ContentBoxWidth won't be returning the value of 10px but instead 0px (10px - 15px).
      auto horizontalSpaceReservedForScrollbar =
        std::max(0_lu, renderer.PaddingBoxRectIncludingScrollbar().Width() - renderer.PaddingBoxWidth());
      auto verticalSpaceReservedForScrollbar =
        std::max(0_lu, renderer.PaddingBoxRectIncludingScrollbar().Height() - renderer.PaddingBoxHeight());
      return {horizontalSpaceReservedForScrollbar, verticalSpaceReservedForScrollbar};
    }

    KRYS_NODISCARD LayoutUnit FontMetricsBasedBaseline(const RenderBox &renderBox) noexcept
    {
      auto &fontMetrics = renderBox.FirstLineStyle().MetricsOfPrimaryFont();
      auto fontHeight =
        SnapToInt(LayoutUnit {fontMetrics.Ascent()}) + SnapToInt(LayoutUnit {fontMetrics.Descent()});
      return LayoutUnit {Ascent(renderBox) + (renderBox.LineHeight() - fontHeight).ToFloat() / 2};
    }

    KRYS_NODISCARD bool ShouldUseMarginBoxAsBaseline(const RenderBox &renderBox) noexcept
    {
      if (Is<RenderTable>(renderBox))
      {
        return true;
      }

      if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(renderBox))
      {
        // The baseline of an 'inline-block' is the baseline of its last line box in the normal flow, unless
        // it has either no in-flow line boxes or if its 'overflow' property has a computed value other than
        // 'visible'. see https://www.w3.org/TR/CSS22/visudet.html
        if (blockFlow->Style().Display() == DisplayType::InlineBlock
            && !blockFlow->Style().IsOverflowVisible())
        {
          return true;
        }

        if (blockFlow->ChildrenInline() && !blockFlow->HasContentfulInlineOrBlockLine()
            && !blockFlow->HasLineIfEmpty())
        {
          return true;
        }

        // CSS2.1 states that the baseline of an inline block is the baseline of the last line box in
        // the normal flow. We make an exception for marquees, since their baselines are meaningless
        // (the content inside them moves). We also give up on finding a baseline if we have a vertical
        // scrollbar, or if we are scrolled vertically (e.g., an overflow:hidden block that has had scrollTop
        // moved).
        auto *scrollableArea = blockFlow->Layer() ? blockFlow->Layer()->GetScrollableArea() : nullptr;
        if (scrollableArea)
        {
          // FIXME: Marquees, once there's a marquee.
          if (blockFlow->ContainingBlock()->GetWritingMode().IsHorizontal())
          {
            return scrollableArea->VerticalScrollbar() || scrollableArea->GetScrollOffset().Y();
          }
          return scrollableArea->HorizontalScrollbar() || scrollableArea->GetScrollOffset().X();
        }
      }
      return false;
    }

    KRYS_NODISCARD Maybe<LayoutUnit> BaselineForBox(const RenderBox &renderBox) noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit> LastInflowBoxBaseline(const RenderBlock &blockContainer) noexcept
    {
      auto *lastInFlowChild = blockContainer.LastInFlowChildBox();
      for (auto *inflowBox = lastInFlowChild; inflowBox; inflowBox = inflowBox->PreviousInFlowSiblingBox())
      {
        if (inflowBox->IsWritingModeRoot())
        {
          continue;
        }

        if (inflowBox->ShouldApplyLayoutContainment())
        {
          continue;
        }

        if (ShouldUseMarginBoxAsBaseline(*inflowBox))
        {
          // We need to find a better candidate for baseline.
          continue;
        }

        // FIXME: Menu lists, once there are menu lists.
        if (Is<RenderFlexibleBox>(*inflowBox) || Is<RenderGrid>(*inflowBox) || Is<RenderBlockFlow>(*inflowBox)
            || Is<RenderTextControlInnerContainer>(*inflowBox))
        {
          if (auto baseline = BaselineForBox(*inflowBox))
          {
            auto baselineValue = inflowBox->LogicalTop() + *baseline;
            return SnapToInt(baselineValue, SnapDirection::Floor);
          }
          continue;
        }
      }

      if (!lastInFlowChild && blockContainer.HasLineIfEmpty())
      {
        auto baselineValue = FontMetricsBasedBaseline(blockContainer)
                             + (blockContainer.ContainingBlock()->GetWritingMode().IsHorizontal()
                                  ? blockContainer.BorderTop() + blockContainer.PaddingTop()
                                  : blockContainer.BorderRight() + blockContainer.PaddingRight());
        return SnapToInt(baselineValue, SnapDirection::Floor);
      }

      return null;
    }

    KRYS_NODISCARD Maybe<LayoutUnit> BaselineForBox(const RenderBox &renderBox) noexcept
    {
      auto writingMode = renderBox.ContainingBlock()->GetWritingMode();
      if (renderBox.ShouldApplyLayoutContainment())
      {
        return null;
      }

      if (writingMode.ComputedWritingMode() != renderBox.GetWritingMode().ComputedWritingMode())
      {
        return null;
      }

      // FIXME: Iframes, embedded objects, canvases and view transition captures have no baseline; images,
      // attachments, buttons, list boxes, file upload controls and sliders have their own, once there are
      // those renderers.
      if (Is<LegacyRenderSVGRoot>(renderBox) || Is<RenderTextControlMultiLine>(renderBox))
      {
        return null;
      }

      auto marginBoxBottom = renderBox.MarginBoxLogicalHeight(writingMode)
                             - (writingMode.IsHorizontal() ? renderBox.MarginTop() : renderBox.MarginRight());

      if (auto *textControl = DynamicDowncast<RenderTextControlSingleLine>(renderBox))
      {
        if (auto *innerTextRenderer = textControl->InnerTextRenderer())
        {
          auto baseline = LayoutUnit {};
          if (innerTextRenderer->InlineLayout())
          {
            auto marginBoxLogicalHeight = innerTextRenderer->MarginBoxLogicalHeight(writingMode);
            auto lastLineBaseline = innerTextRenderer->InlineLayout()->LastLineBaseline().value_or(0_lu);
            baseline = std::min(marginBoxLogicalHeight, lastLineBaseline);
          }
          else
          {
            baseline = FontMetricsBasedBaseline(*innerTextRenderer);
          }
          baseline = SnapToInt(innerTextRenderer->LogicalTop() + baseline, SnapDirection::Floor);
          for (auto *ancestor = innerTextRenderer->ContainingBlock(); ancestor && ancestor != textControl;
               ancestor = ancestor->ContainingBlock())
          {
            baseline = SnapToInt(ancestor->LogicalTop() + baseline, SnapDirection::Floor);
          }
          return baseline;
        }
        // input::-webkit-textfield-decoration-container { display: none }
        return null;
      }

      // FIXME: MathML blocks use their first line's baseline, once there's MathML layout.

      if (Is<RenderTable>(renderBox))
      {
        return renderBox.FirstLineBaseline();
      }

      // FIXME: Menu lists, which also behave differently from flex boxes, with them.
      if (Is<RenderTextControlInnerContainer>(renderBox))
      {
        // Inner containers are a type of flex box but they behave slightly differently so always check them
        // before checking for flex.
        return LastInflowBoxBaseline(Downcast<RenderBlock>(renderBox));
      }

      if (Is<RenderFlexibleBox>(renderBox) || Is<RenderGrid>(renderBox))
      {
        return renderBox.FirstLineBaseline();
      }

      if (renderBox.IsFieldset())
      {
        // Note that <fieldset> may simply be a flex/grid box (a non-RenderBlockFlow RenderBlock) and already
        // handled above.
        if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(renderBox))
        {
          // <fieldset> with no legend.
          if (auto *inlineLayout = blockFlow->InlineLayout())
          {
            return inlineLayout->LastLineBaseline().value_or(0_lu);
          }
          return LastInflowBoxBaseline(*blockFlow);
        }
        return null;
      }

      if (Is<html::HTMLFormControlElement>(renderBox.GetElement()))
      {
        // For "leaf" theme objects like checkbox, let the theme decide what the baseline position is.
        auto &theme = RenderTheme::Singleton();
        if (renderBox.Style().HasUsedAppearance()
            && !theme.IsControlContainer(renderBox.Style().UsedAppearance()))
        {
          return LayoutUnit(theme.BaselinePosition(renderBox));
        }

        // Non-RenderTextControlSingleLine input type like input type color.
        if (auto *container = DynamicDowncast<RenderBox>(renderBox.FirstInFlowChild()))
        {
          if (auto baseline = container->FirstLineBaseline())
          {
            return container->LogicalTop() + *baseline;
          }
        }
        // e.g. leaf theme objects with no appearance (none) and empty content (e.g. before pseudo and
        // content:
        // "").
        return null;
      }

      if (auto *element = renderBox.GetElement())
      {
        if (Is<html::HTMLFormControlElement>(dom::TreeQueries::ShadowHost(*element)))
        {
          // Inside RenderTextControl's shadow DOM (e.g. strong-password text)
          auto lastBaseline = Maybe<LayoutUnit> {};
          if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(renderBox))
          {
            if (auto *inlineLayout = blockFlow->InlineLayout())
            {
              lastBaseline = inlineLayout->LastLineBaseline().value_or(0_lu);
            }
          }
          if (!lastBaseline)
          {
            lastBaseline =
              SnapToInt(FontMetricsBasedBaseline(renderBox)
                          + (writingMode.IsHorizontal() ? renderBox.BorderTop() + renderBox.PaddingTop()
                                                        : renderBox.BorderRight() + renderBox.PaddingRight()),
                        SnapDirection::Floor);
          }
          return std::min(marginBoxBottom, *lastBaseline);
        }
      }

      if (auto *deprecatedFlexBox = DynamicDowncast<RenderDeprecatedFlexibleBox>(renderBox))
      {
        // Historically, we did this check for all baselines. But we can't remove this code from deprecated
        // flexbox, because it effectively breaks -webkit-line-clamp, which is used in the wild -- we would
        // calculate the baseline as if -webkit-line-clamp wasn't used. For simplicity, we use this for all
        // uses of deprecated flexbox.
        auto bottomOfContent = deprecatedFlexBox->BorderBefore() + deprecatedFlexBox->PaddingBefore()
                               + deprecatedFlexBox->ContentBoxLogicalHeight();
        auto baseline = LastInflowBoxBaseline(*deprecatedFlexBox);
        if (baseline && *baseline <= bottomOfContent)
        {
          return *baseline;
        }
        return null;
      }

      if (auto *listMarker = DynamicDowncast<RenderListMarker>(renderBox))
      {
        if (listMarker->ListItem() && !listMarker->IsImage())
        {
          return SnapToInt(FontMetricsBasedBaseline(*listMarker), SnapDirection::Floor);
        }
        return null;
      }

      if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(renderBox))
      {
        if (ShouldUseMarginBoxAsBaseline(*blockFlow) || blockFlow->Style().OverflowY() != Overflow::Visible)
        {
          return marginBoxBottom;
        }

        // Note that here we only take the left and bottom into consideration. Our caller takes the right and
        // top into consideration.
        if (!blockFlow->ChildrenInline())
        {
          return LastInflowBoxBaseline(*blockFlow);
        }

        if (!blockFlow->HasContentfulInlineOrBlockLine())
        {
          krys_debug_assert(blockFlow->HasLineIfEmpty());
          return SnapToInt(FontMetricsBasedBaseline(*blockFlow)
                             + (writingMode.IsHorizontal()
                                  ? blockFlow->BorderTop() + blockFlow->PaddingTop()
                                  : blockFlow->BorderRight() + blockFlow->PaddingRight()),
                           SnapDirection::Floor);
        }

        if (auto *inlineLayout = blockFlow->InlineLayout())
        {
          return inlineLayout->LastLineBaseline().value_or(0_lu);
        }

        if (blockFlow->SvgTextLayout())
        {
          auto &style = blockFlow->FirstLineStyle();
          // LegacyInlineFlowBox::PlaceBoxesInBlockDirection will flip lines in case of verticalLR mode, so we
          // can assume verticalRL for now.
          return LayoutUnit(
            blockFlow->LegacyRootBox()->LogicalTop()
            + style.MetricsOfPrimaryFont().IntAscent(blockFlow->LegacyRootBox()->BaselineType()));
        }

        krys_debug_assert(false);
        return marginBoxBottom;
      }

      // FIXME: Replaced boxes other than images, once there are any.
      return null;
    }

    inline void SetIntegrationBaseline(const RenderBox &renderBox) noexcept
    {
      if (renderBox.IsFloatingOrOutOfFlowPositioned())
      {
        return;
      }

      auto hasNonSyntheticBaseline = [&]
      {
        if (auto *renderListMarker = DynamicDowncast<RenderListMarker>(renderBox))
        {
          return !renderListMarker->IsImage();
        }

        // FIXME: List boxes, sliders, attachments, MathML blocks and buttons, once there are those renderers.
        if ((Is<RenderReplaced>(renderBox) && renderBox.Style().Display() == DisplayType::Inline)
            || Is<RenderTextControlMultiLine>(renderBox) || Is<RenderTable>(renderBox)
            || Is<RenderGrid>(renderBox) || Is<RenderFlexibleBox>(renderBox)
            || Is<RenderDeprecatedFlexibleBox>(renderBox))
        {
          // These are special RenderBlock renderers that override the default baseline position behavior of
          // the inline block box.
          return true;
        }
        auto *blockFlow = DynamicDowncast<RenderBlockFlow>(renderBox);
        if (!blockFlow)
        {
          return false;
        }
        auto hasAppareance =
          blockFlow->Style().HasUsedAppearance()
          && !RenderTheme::Singleton().IsControlContainer(blockFlow->Style().UsedAppearance());
        return hasAppareance || !blockFlow->ChildrenInline() || blockFlow->HasContentfulInlineOrBlockLine()
               || blockFlow->HasLineIfEmpty();
      };

      if (hasNonSyntheticBaseline())
      {
        auto baselinePosition = [&]() -> LayoutUnit
        {
          auto rootWritingMode = renderBox.ContainingBlock()->GetWritingMode();
          auto marginBefore =
            renderBox.GetWritingMode().IsHorizontal() ? renderBox.MarginTop() : renderBox.MarginRight();

          if (auto baseline = BaselineForBox(renderBox))
          {
            *baseline = rootWritingMode.IsLineInverted() ? renderBox.LogicalHeight() - *baseline : *baseline;
            return marginBefore + *baseline;
          }

          auto marginBoxLogicalHeight = renderBox.MarginBoxLogicalHeight(rootWritingMode);
          auto isWritingModeRoot =
            rootWritingMode.ComputedWritingMode() != renderBox.GetWritingMode().ComputedWritingMode();

          if (renderBox.IsFieldset())
          {
            if (isWritingModeRoot || renderBox.ShouldApplyLayoutContainment())
            {
              return marginBoxLogicalHeight;
            }
            return SnapToInt(marginBoxLogicalHeight);
          }

          // FIXME: Buttons use their content box bottom, once there are buttons.

          return SnapToInt(marginBoxLogicalHeight);
        };
        const_cast<layout::ElementBox &>(*renderBox.LayoutBox())
          .SetBaselineForIntegration(baselinePosition());
      }
    }
  }

  BoxGeometryUpdater::BoxGeometryUpdater(layout::LayoutState &layoutState,
                                         const layout::ElementBox &rootLayoutBox) noexcept
      : _layoutState(WeakPtr<layout::LayoutState>::WithRef(layoutState)),
        _rootLayoutBox(ShareCheckedPtr(&rootLayoutBox))
  {
  }

  void BoxGeometryUpdater::Clear() noexcept
  {
    _rootLayoutBox = nullptr;
    _nestedListMarkerOffsets.clear();
  }

  const RenderBlock &BoxGeometryUpdater::RootRenderer() const noexcept
  {
    return Downcast<RenderBlock>(*RootLayoutBox().RendererForIntegration());
  }

  WritingMode BoxGeometryUpdater::GetWritingMode() const noexcept
  {
    return RootRenderer().GetWritingMode();
  }

  void BoxGeometryUpdater::SetListMarkerOffsetForMarkerOutside(const RenderListMarker &listMarker) noexcept
  {
    auto &layoutBox = *listMarker.LayoutBox();
    krys_debug_assert(layoutBox.IsListMarkerOutside());
    auto *ancestor = listMarker.ContainingBlock();

    auto offsetFromParentListItem = [&]
    {
      auto hasAccountedForBorderAndPadding = false;
      auto offset = LayoutUnit {};
      for (; ancestor; ancestor = ancestor->ContainingBlock())
      {
        if (!hasAccountedForBorderAndPadding)
        {
          offset -= (ancestor->BorderStart() + ancestor->PaddingStart());
        }
        if (Is<RenderListItem>(*ancestor))
        {
          break;
        }
        offset -= (ancestor->MarginStart());
        if (ancestor->IsFlexItem())
        {
          offset -= ancestor->LogicalLeft();
          hasAccountedForBorderAndPadding = true;
          continue;
        }
        hasAccountedForBorderAndPadding = false;
      }
      return offset;
    }();

    auto offsetFromAssociatedListItem = [&]
    {
      auto *associatedListItem = listMarker.ListItem();
      if (ancestor == associatedListItem || !ancestor)
      {
        // FIXME: Handle column spanner case when ancestor is null here.
        return offsetFromParentListItem;
      }
      auto offset = offsetFromParentListItem;
      for (ancestor = ancestor->ContainingBlock(); ancestor; ancestor = ancestor->ContainingBlock())
      {
        offset -= (ancestor->MarginStart() + ancestor->BorderStart() + ancestor->PaddingStart());
        if (ancestor == associatedListItem)
        {
          break;
        }
      }
      return offset;
    }();

    if (offsetFromAssociatedListItem)
    {
      auto &listMarkerGeometry = GetLayoutState().EnsureGeometryForBox(layoutBox);
      // Make sure that the line content does not get pulled in to logical left direction due to
      // the large negative margin (i.e. this ensures that logical left of the list content stays at the line
      // start)
      listMarkerGeometry.SetHorizontalMargin({listMarkerGeometry.MarginStart() + offsetFromParentListItem,
                                              listMarkerGeometry.MarginEnd() - offsetFromParentListItem});
      if (auto nestedOffset = offsetFromAssociatedListItem - offsetFromParentListItem)
      {
        _nestedListMarkerOffsets.insert_or_assign(&layoutBox, nestedOffset);
      }
    }
  }

  layout::BoxGeometry::HorizontalEdges
    BoxGeometryUpdater::HorizontalLogicalMargin(const RenderBoxModelObject &renderer,
                                                Maybe<LayoutUnit> availableWidth, WritingMode writingMode,
                                                bool retainMarginStart, bool retainMarginEnd) noexcept
  {
    auto &style = renderer.Style();
    const auto zoomFactor = style.UsedZoomForLength();

    if (writingMode.IsHorizontal())
    {
      auto marginInlineStart =
        retainMarginStart
          ? UsedValueOrZero(writingMode.IsInlineLeftToRight() ? style.MarginLeft() : style.MarginRight(),
                            availableWidth, zoomFactor)
          : 0_lu;
      auto marginInlineEnd =
        retainMarginEnd
          ? UsedValueOrZero(writingMode.IsInlineLeftToRight() ? style.MarginRight() : style.MarginLeft(),
                            availableWidth, zoomFactor)
          : 0_lu;

      return {marginInlineStart, marginInlineEnd};
    }

    auto marginInlineStart =
      retainMarginStart
        ? UsedValueOrZero(writingMode.IsInlineTopToBottom() ? style.MarginTop() : style.MarginBottom(),
                          availableWidth, zoomFactor)
        : 0_lu;
    auto marginInlineEnd =
      retainMarginEnd
        ? UsedValueOrZero(writingMode.IsInlineTopToBottom() ? style.MarginBottom() : style.MarginTop(),
                          availableWidth, zoomFactor)
        : 0_lu;

    return {marginInlineStart, marginInlineEnd};
  }

  layout::BoxGeometry::VerticalEdges BoxGeometryUpdater::VerticalLogicalMargin(
    const RenderBoxModelObject &renderer, Maybe<LayoutUnit> availableWidth, WritingMode writingMode) noexcept
  {
    auto &style = renderer.Style();
    auto zoomFactor = style.UsedZoomForLength();
    if (writingMode.IsHorizontal())
    {
      return {UsedValueOrZero(style.MarginTop(), availableWidth, zoomFactor),
              UsedValueOrZero(style.MarginBottom(), availableWidth, zoomFactor)};
    }
    if (writingMode.IsLineOverLeft())
    {
      return {UsedValueOrZero(style.MarginLeft(), availableWidth, zoomFactor),
              UsedValueOrZero(style.MarginRight(), availableWidth, zoomFactor)};
    }
    return {UsedValueOrZero(style.MarginRight(), availableWidth, zoomFactor),
            UsedValueOrZero(style.MarginLeft(), availableWidth, zoomFactor)};
  }

  layout::BoxGeometry::Edges BoxGeometryUpdater::LogicalBorder(const RenderBoxModelObject &renderer,
                                                               WritingMode writingMode,
                                                               bool isIntrinsicWidthMode,
                                                               bool retainBorderStart,
                                                               bool retainBorderEnd) noexcept
  {
    auto &style = renderer.Style();

    auto borderWidths =
      RectEdges<LayoutUnit> {style::Evaluate<LayoutUnit>(style.BorderTopWidth(), style::ZoomNeeded {}),
                             style::Evaluate<LayoutUnit>(style.BorderRightWidth(), style::ZoomNeeded {}),
                             style::Evaluate<LayoutUnit>(style.BorderBottomWidth(), style::ZoomNeeded {}),
                             style::Evaluate<LayoutUnit>(style.BorderLeftWidth(), style::ZoomNeeded {})};

    if (!isIntrinsicWidthMode)
    {
      AdjustBorderForTableAndFieldset(renderer, borderWidths);
    }

    if (writingMode.IsHorizontal())
    {
      auto borderInlineStart =
        retainBorderStart ? (writingMode.IsInlineLeftToRight() ? borderWidths.Left() : borderWidths.Right())
                          : 0_lu;
      auto borderInlineEnd =
        retainBorderEnd ? (writingMode.IsInlineLeftToRight() ? borderWidths.Right() : borderWidths.Left())
                        : 0_lu;
      return {{borderInlineStart, borderInlineEnd}, {borderWidths.Top(), borderWidths.Bottom()}};
    }

    auto borderInlineStart =
      retainBorderStart ? (writingMode.IsInlineTopToBottom() ? borderWidths.Top() : borderWidths.Bottom())
                        : 0_lu;
    auto borderInlineEnd =
      retainBorderEnd ? (writingMode.IsInlineTopToBottom() ? borderWidths.Bottom() : borderWidths.Top())
                      : 0_lu;
    auto borderLineOver = writingMode.IsLineOverRight() ? borderWidths.Right() : borderWidths.Left();
    auto borderLineUnder = writingMode.IsLineOverRight() ? borderWidths.Left() : borderWidths.Right();
    return {{borderInlineStart, borderInlineEnd}, {borderLineOver, borderLineUnder}};
  }

  layout::BoxGeometry::Edges BoxGeometryUpdater::LogicalPadding(const RenderBoxModelObject &renderer,
                                                                Maybe<LayoutUnit> availableWidth,
                                                                WritingMode writingMode,
                                                                bool retainPaddingStart,
                                                                bool retainPaddingEnd) noexcept
  {
    auto &style = renderer.Style();
    auto usedZoom = style.UsedZoomForLength();

    auto paddingLeft = UsedValueOrZero(style.PaddingLeft(), availableWidth, usedZoom);
    auto paddingRight = UsedValueOrZero(style.PaddingRight(), availableWidth, usedZoom);
    auto paddingTop = UsedValueOrZero(style.PaddingTop(), availableWidth, usedZoom);
    auto paddingBottom = UsedValueOrZero(style.PaddingBottom(), availableWidth, usedZoom);

    if (writingMode.IsHorizontal())
    {
      auto paddingInlineStart =
        retainPaddingStart ? (writingMode.IsInlineLeftToRight() ? paddingLeft : paddingRight) : 0_lu;
      auto paddingInlineEnd =
        retainPaddingEnd ? (writingMode.IsInlineLeftToRight() ? paddingRight : paddingLeft) : 0_lu;
      return {{paddingInlineStart, paddingInlineEnd}, {paddingTop, paddingBottom}};
    }

    auto paddingInlineStart =
      retainPaddingStart ? (writingMode.IsInlineTopToBottom() ? paddingTop : paddingBottom) : 0_lu;
    auto paddingInlineEnd =
      retainPaddingEnd ? (writingMode.IsInlineTopToBottom() ? paddingBottom : paddingTop) : 0_lu;
    auto paddingLineOver = writingMode.IsLineOverRight() ? paddingRight : paddingLeft;
    auto paddingLineUnder = writingMode.IsLineOverRight() ? paddingLeft : paddingRight;
    return {{paddingInlineStart, paddingInlineEnd}, {paddingLineOver, paddingLineUnder}};
  }

  void BoxGeometryUpdater::UpdateLayoutBoxDimensions(
    const RenderBox &renderBox, Maybe<LayoutUnit> availableWidth,
    Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode) noexcept
  {
    auto &layoutBox = *renderBox.LayoutBox();
    auto &boxGeometry = GetLayoutState().EnsureGeometryForBox(layoutBox);
    auto writingMode = renderBox.Parent()->GetWritingMode();

    auto inlineMargin = HorizontalLogicalMargin(renderBox, availableWidth, writingMode);
    auto border = LogicalBorder(renderBox, writingMode, intrinsicWidthMode.has_value());
    auto padding = LogicalPadding(renderBox, availableWidth, writingMode);
    if (!intrinsicWidthMode)
    {
      padding.Vertical += IntrinsicPaddingForTableCell(renderBox);
    }

    auto scrollbarSize = ScrollbarLogicalSize(renderBox);

    if (intrinsicWidthMode)
    {
      boxGeometry.SetHorizontalSpaceForScrollbar(scrollbarSize.Width());
      auto contentBoxLogicalWidth = [&]
      {
        auto preferredWidth = *intrinsicWidthMode == layout::IntrinsicWidthMode::Minimum
                                ? renderBox.MinPreferredLogicalWidth()
                                : renderBox.MaxPreferredLogicalWidth();
        return preferredWidth
               - (border.Horizontal.Start + border.Horizontal.End + padding.Horizontal.Start
                  + padding.Horizontal.End);
      };
      boxGeometry.SetContentBoxWidth(contentBoxLogicalWidth());
      boxGeometry.SetHorizontalMargin(inlineMargin);
      boxGeometry.SetHorizontalBorder(border.Horizontal);
      boxGeometry.SetHorizontalPadding(padding.Horizontal);
      return;
    }

    boxGeometry.SetSpaceForScrollbar(scrollbarSize);
    if (!renderBox.IsOutOfFlowPositioned())
    {
      boxGeometry.SetContentBoxWidth(ContentLogicalWidthForRenderer(renderBox));
      boxGeometry.SetContentBoxHeight(ContentLogicalHeightForRenderer(renderBox));
    }
    boxGeometry.SetVerticalMargin(VerticalLogicalMargin(renderBox, availableWidth, writingMode));
    boxGeometry.SetHorizontalMargin(inlineMargin);
    boxGeometry.SetBorder(border);
    boxGeometry.SetPadding(padding);
  }

  void BoxGeometryUpdater::UpdateLineBreakBoxDimensions(const RenderLineBreak &lineBreakBox) noexcept
  {
    // This is just a box geometry reset.
    GetLayoutState().EnsureGeometryForBox(*lineBreakBox.LayoutBox()).Reset();
  }

  void BoxGeometryUpdater::UpdateInlineBoxDimensions(
    const RenderInline &renderInline, Maybe<LayoutUnit> availableWidth,
    Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode) noexcept
  {
    auto &boxGeometry = GetLayoutState().EnsureGeometryForBox(*renderInline.LayoutBox());

    // FIXME: A continuation keeps only one side's margin, border and padding, once there are continuations.
    auto writingMode = renderInline.GetWritingMode();

    auto inlineMargin = HorizontalLogicalMargin(renderInline, availableWidth, writingMode);
    auto border = LogicalBorder(renderInline, writingMode, intrinsicWidthMode.has_value());
    auto padding = LogicalPadding(renderInline, availableWidth, writingMode);

    if (intrinsicWidthMode)
    {
      boxGeometry.SetHorizontalMargin(inlineMargin);
      boxGeometry.SetHorizontalBorder(border.Horizontal);
      boxGeometry.SetHorizontalPadding(padding.Horizontal);
      return;
    }

    boxGeometry.SetHorizontalMargin(inlineMargin);
    boxGeometry.SetVerticalMargin(VerticalLogicalMargin(renderInline, availableWidth, writingMode));
    boxGeometry.SetBorder(border);
    boxGeometry.SetPadding(padding);
  }

  void BoxGeometryUpdater::SetFormattingContextContentGeometry(
    Maybe<LayoutUnit> availableLogicalWidth, Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode) noexcept
  {
    krys_debug_assert(availableLogicalWidth || intrinsicWidthMode);

    if (RootLayoutBox().EstablishesInlineFormattingContext())
    {
      for (auto walker = InlineWalker(Downcast<RenderBlockFlow>(RootRenderer())); !walker.AtEnd();
           walker.Advance())
      {
        if (!Is<RenderText>(walker.Current()))
        {
          UpdateBoxGeometry(Downcast<RenderElement>(*walker.Current()), availableLogicalWidth,
                            intrinsicWidthMode);
        }
      }
      return;
    }

    if (RootLayoutBox().EstablishesFlexFormattingContext()
        || RootLayoutBox().EstablishesGridFormattingContext())
    {
      for (auto *child = RootLayoutBox().FirstChild(); child; child = child->NextSibling())
      {
        UpdateBoxGeometry(Downcast<RenderElement>(*child->RendererForIntegration()), availableLogicalWidth,
                          intrinsicWidthMode);
      }
      return;
    }

    krys_debug_assert(false);
  }

  void BoxGeometryUpdater::SetFormattingContextRootGeometry(LayoutUnit availableWidth) noexcept
  {
    // FIXME: BFC should be responsible for creating the box geometry for this block box (IFC root) as part of
    // the block layout. This is really only required by float layout as IFC does not consult the root
    // geometry directly.
    auto &rootRenderer = RootRenderer();
    auto writingMode = GetWritingMode();

    auto padding = LogicalPadding(rootRenderer, availableWidth, writingMode);
    auto border = LogicalBorder(rootRenderer, writingMode);
    if (writingMode.IsVertical() && !RootLayoutBox().GetWritingMode().IsBlockFlipped())
    {
      padding.Vertical = {padding.Vertical.After, padding.Vertical.Before};
      border.Vertical = {border.Vertical.After, border.Vertical.Before};
    }

    auto &rootGeometry = GetLayoutState().EnsureGeometryForBox(RootLayoutBox());
    rootGeometry.SetContentBoxWidth(writingMode.IsHorizontal() ? rootRenderer.ContentBoxWidth()
                                                               : rootRenderer.ContentBoxHeight());
    rootGeometry.SetPadding(padding);
    rootGeometry.SetBorder(border);
    rootGeometry.SetSpaceForScrollbar(ScrollbarLogicalSize(rootRenderer));
    rootGeometry.SetHorizontalMargin(HorizontalLogicalMargin(rootRenderer, availableWidth, writingMode));
    rootGeometry.SetVerticalMargin(VerticalLogicalMargin(rootRenderer, availableWidth, writingMode));
  }

  layout::ConstraintsForInlineContent
    BoxGeometryUpdater::FormattingContextConstraints(LayoutUnit availableWidth) noexcept
  {
    auto &rootRenderer = RootRenderer();
    auto writingMode = GetWritingMode();

    if (rootRenderer.IsRenderSVGText())
    {
      auto horizontalConstraints = layout::HorizontalConstraints {0_lu, LayoutUnit::Max()};
      return {{horizontalConstraints, 0_lu}, 0_lu, rootRenderer.Size()};
    }

    auto padding = LogicalPadding(rootRenderer, availableWidth, writingMode);
    auto border = LogicalBorder(rootRenderer, writingMode);
    if (writingMode.IsVertical() && writingMode.IsLineInverted())
    {
      padding.Vertical = {padding.Vertical.After, padding.Vertical.Before};
      border.Vertical = {border.Vertical.After, border.Vertical.Before};
    }
    padding.Vertical += IntrinsicPaddingForTableCell(rootRenderer);

    auto scrollbarSize = ScrollbarLogicalSize(rootRenderer);
    auto shouldPlaceVerticalScrollbarOnLeft = rootRenderer.Style().ShouldPlaceVerticalScrollbarOnLeft();

    auto contentBoxWidth =
      writingMode.IsHorizontal() ? rootRenderer.ContentBoxWidth() : rootRenderer.ContentBoxHeight();
    auto contentBoxLeft = border.Horizontal.Start + padding.Horizontal.Start;
    auto contentBoxTop = border.Vertical.Before + padding.Vertical.Before;
    if (writingMode.IsInlineLeftToRight())
    {
      contentBoxLeft += shouldPlaceVerticalScrollbarOnLeft ? scrollbarSize.Width() : 0_lu;
    }
    else if (writingMode.IsBlockLeftToRight())
    {
      contentBoxTop += shouldPlaceVerticalScrollbarOnLeft ? scrollbarSize.Width() : 0_lu;
    }

    auto horizontalConstraints = layout::HorizontalConstraints {contentBoxLeft, contentBoxWidth};
    auto visualLeft = writingMode.IsBidiRTL() || shouldPlaceVerticalScrollbarOnLeft
                        ? border.Horizontal.End + scrollbarSize.Width() + padding.Horizontal.End
                        : contentBoxLeft;

    return {{horizontalConstraints, contentBoxTop}, visualLeft, rootRenderer.Size()};
  }

  void BoxGeometryUpdater::UpdateBoxGeometryAfterIntegrationLayout(const layout::ElementBox &layoutBox,
                                                                   LayoutUnit availableWidth) noexcept
  {
    auto *renderBox = DynamicDowncast<RenderBox>(layoutBox.RendererForIntegration());
    if (!renderBox)
    {
      krys_debug_assert(false);
      return;
    }

    auto &boxGeometry = GetLayoutState().EnsureGeometryForBox(layoutBox);
    boxGeometry.SetContentBoxSize(renderBox->ContentBoxLogicalSize());
    boxGeometry.SetSpaceForScrollbar(ScrollbarLogicalSize(*renderBox));

    auto integrationAdjustments = [&]
    {
      // FIXME: These should eventually be all absorbed by LFC layout.
      SetIntegrationBaseline(*renderBox);

      if (auto *renderListMarker = DynamicDowncast<RenderListMarker>(*renderBox))
      {
        auto &style = layoutBox.Parent().Style();
        boxGeometry.SetHorizontalMargin(
          HorizontalLogicalMargin(*renderListMarker, null, style.GetWritingMode()));
        if (!renderListMarker->IsInside())
        {
          SetListMarkerOffsetForMarkerOutside(*renderListMarker);
        }
        auto layoutBounds = renderListMarker->LayoutBounds();
        const_cast<layout::ElementBox &>(layoutBox).SetListMarkerLayoutBounds(
          {static_cast<int32>(layoutBounds.first), static_cast<int32>(layoutBounds.second)});
      }

      if (Is<RenderTable>(*renderBox))
      {
        // Tables have their special collapsed border values (updated at layout).
        auto &style = layoutBox.Parent().Style();
        boxGeometry.SetBorder(LogicalBorder(*renderBox, style.GetWritingMode()));
      }

      auto needsFullGeometryUpdate = [&]
      {
        if (renderBox->IsFieldset())
        {
          // Fieldsets with legends have intrinsic padding values.
          return true;
        }
        if (renderBox->IsWritingModeRoot())
        {
          // Currently we've got one BoxGeometry for a layout box, but it represents geometry when
          // it is a root but also when it is an inline level box (e.g. floats, inline-blocks).
          return true;
        }
        if (!layoutBox.IsInitialContainingBlock() && layoutBox.EstablishesFormattingContext()
            && layoutBox.GetWritingMode().IsInlineOpposing(layoutBox.Parent().GetWritingMode()))
        {
          return true;
        }
        return false;
      };
      if (needsFullGeometryUpdate())
      {
        UpdateLayoutBoxDimensions(*renderBox, availableWidth);
      }

      if (auto *shapeOutsideInfo = renderBox->GetShapeOutsideInfo())
      {
        const_cast<layout::ElementBox &>(layoutBox).SetShape(ShareRefPtr(&shapeOutsideInfo->ComputedShape()));
      }
    };
    integrationAdjustments();
  }

  void BoxGeometryUpdater::UpdateBoxGeometry(const RenderElement &renderer, Maybe<LayoutUnit> availableWidth,
                                             Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode) noexcept
  {
    krys_debug_assert(availableWidth || intrinsicWidthMode);

    if (auto *renderBox = DynamicDowncast<RenderBox>(renderer))
    {
      UpdateLayoutBoxDimensions(*renderBox, availableWidth, intrinsicWidthMode);
      if (auto *renderListMarker = DynamicDowncast<RenderListMarker>(renderer);
          renderListMarker && !renderListMarker->IsInside())
      {
        SetListMarkerOffsetForMarkerOutside(*renderListMarker);
      }
      return;
    }

    if (auto *renderLineBreak = DynamicDowncast<RenderLineBreak>(renderer))
    {
      UpdateLineBreakBoxDimensions(*renderLineBreak);
      return;
    }

    if (auto *renderInline = DynamicDowncast<RenderInline>(renderer))
    {
      UpdateInlineBoxDimensions(*renderInline, availableWidth, intrinsicWidthMode);
    }
  }
}
