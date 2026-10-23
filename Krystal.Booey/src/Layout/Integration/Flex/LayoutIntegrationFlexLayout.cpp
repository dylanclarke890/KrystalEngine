#include "Krystal.Booey/Layout/Integration/Flex/LayoutIntegrationFlexLayout.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingUtils.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxGeometryUpdater.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/FormattingContextBoxIterator.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::layout_integration
{
  namespace
  {
    KRYS_NODISCARD inline bool MayHaveScrollbarOrScrollableOverflow(const RenderStyle &style) noexcept
    {
      return !style.IsOverflowVisible() || !style.GetScrollbarGutter().IsAuto();
    }

    KRYS_NODISCARD inline layout::ConstraintsForFlexContent
      ConstraintsForFlexContent(const layout::ElementBox &flexContainer) noexcept
    {
      auto &flexContainerRenderer = Downcast<RenderFlexibleBox>(*flexContainer.RendererForIntegration());
      auto &flexBoxStyle = flexContainer.Style();
      auto boxSizingIsContentBox = flexBoxStyle.GetBoxSizing() == BoxSizing::ContentBox;
      auto availableLogicalWidth = flexContainerRenderer.ContentBoxRect().Width();
      // FIXME: Use root's BoxGeometry which first needs to stop flipping for the formatting context.
      auto horizontalBorderAndPadding =
        flexContainerRenderer.BorderAndPaddingStart() + flexContainerRenderer.BorderAndPaddingEnd();
      auto verticalBorderAndPadding =
        flexContainerRenderer.BorderAndPaddingBefore() + flexContainerRenderer.BorderAndPaddingAfter();

      auto widthValue = [&]<typename TSize>(const TSize &computedValue) -> Maybe<LayoutUnit>
      {
        if (auto fixedWidth = computedValue.TryFixed())
        {
          auto value = style::Evaluate<LayoutUnit>(*fixedWidth, flexBoxStyle.UsedZoomForLength());
          return boxSizingIsContentBox ? value : value - horizontalBorderAndPadding;
        }
        if (auto percentageWidth = computedValue.TryPercentage())
        {
          auto value = style::Evaluate<LayoutUnit>(*percentageWidth,
                                                   flexContainerRenderer.ContainingBlock()->LogicalWidth());
          return boxSizingIsContentBox ? value : value - horizontalBorderAndPadding;
        }
        return {};
      };

      auto heightValue = [&]<typename TSize>(const TSize &computedValue,
                                             bool callRendererForPercentValue = false) -> Maybe<LayoutUnit>
      {
        if (auto fixedHeight = computedValue.TryFixed())
        {
          auto value = style::Evaluate<LayoutUnit>(*fixedHeight, flexBoxStyle.UsedZoomForLength());
          return boxSizingIsContentBox ? value : value - verticalBorderAndPadding;
        }

        if (auto percentageHeight = computedValue.TryPercentage())
        {
          if (callRendererForPercentValue)
          {
            return flexContainerRenderer.ComputePercentageLogicalHeight(
              TSize {*percentageHeight}, RenderBox::UpdatePercentageHeightDescendants::No);
          }

          auto *containingBlock = flexContainerRenderer.ContainingBlock();
          if (auto fixedContainingBlockHeight = containingBlock->Style().Height().TryFixed())
          {
            auto containingBlockHeightValue = style::Evaluate<LayoutUnit>(
              *fixedContainingBlockHeight, containingBlock->Style().UsedZoomForLength());
            auto value = style::Evaluate<LayoutUnit>(*percentageHeight, containingBlockHeightValue);
            return boxSizingIsContentBox ? value : value - verticalBorderAndPadding;
          }
        }
        return {};
      };

      auto widthGeometry = [&]() -> layout::ConstraintsForFlexContent::AxisGeometry
      {
        return {widthValue(flexBoxStyle.MinWidth()), widthValue(flexBoxStyle.MaxWidth()),
                availableLogicalWidth ? Maybe<LayoutUnit>(availableLogicalWidth)
                                      : widthValue(flexBoxStyle.Width()),
                flexContainerRenderer.ContentBoxLocation().X()};
      };

      auto heightGeometry = [&]() -> layout::ConstraintsForFlexContent::AxisGeometry
      {
        auto availableSize = heightValue(flexBoxStyle.Height(), true);
        auto logicalMinHeight = heightValue(flexBoxStyle.MinHeight()).value_or(0_lu);
        auto logicalMaxHeight = heightValue(flexBoxStyle.MaxHeight());
        if (!availableSize || (logicalMaxHeight && *logicalMaxHeight < *availableSize))
        {
          availableSize = logicalMaxHeight;
        }

        return {logicalMinHeight, logicalMaxHeight, availableSize,
                flexContainerRenderer.ContentBoxLocation().Y()};
      };

      return layout::FlexFormattingUtils::IsMainAxisParallelWithInlineAxis(flexContainer)
               ? layout::ConstraintsForFlexContent(widthGeometry(), heightGeometry(), false)
               : layout::ConstraintsForFlexContent(heightGeometry(), widthGeometry(), false);
    }
  }

  FlexLayout::FlexLayout(RenderFlexibleBox &flexBoxRenderer) noexcept
      : _flexBox(ShareCheckedPtr(BoxTreeUpdater {flexBoxRenderer}.Build().get())),
        _layoutState(WeakPtr<layout::LayoutState>::WithRef(flexBoxRenderer.View().GetLayoutState()))
  {
  }

  FlexLayout::~FlexLayout() noexcept
  {
    auto &renderer = FlexBoxRenderer();
    _flexBox = nullptr;

    BoxTreeUpdater {renderer}.TearDown();
  }

  bool FlexLayout::CanUseFor(const RenderFlexibleBox &flexBox) noexcept
  {
    if (!flexBox.FirstInFlowChild())
    {
      return false;
    }

    auto &flexBoxStyle = flexBox.Style();
    if (flexBoxStyle.Display() == DisplayType::InlineFlex)
    {
      // The flex box needs a baseline.
      return false;
    }

    auto isColumnDirection = flexBoxStyle.GetFlexDirection() == FlexDirection::Column
                             || flexBoxStyle.GetFlexDirection() == FlexDirection::ColumnReverse;
    auto isHorizontalWritingMode = flexBoxStyle.GetWritingMode().IsHorizontal();
    if (!isHorizontalWritingMode)
    {
      // FIXME: Integration layer needs to do geometry translate from logical to physical (or whatever render
      // tree needs).
      return false;
    }

    if (((isHorizontalWritingMode && isColumnDirection) || (!isHorizontalWritingMode && !isColumnDirection))
        && !flexBoxStyle.Height().IsFixed())
    {
      // The flex box has a non-fixed height in its main axis.
      return false;
    }

    if (MayHaveScrollbarOrScrollableOverflow(flexBoxStyle))
    {
      return false;
    }

    if (flexBoxStyle.GetMarginTrim() != style::ComputedStyle::InitialMarginTrim())
    {
      return false;
    }

    auto isFlexBoxInsideBFC = [&]
    {
      // FIXME: This may be somewhat expensive in some deeply nested cases.
      for (auto *containingBlock = flexBox.ContainingBlock();
           containingBlock && !Is<RenderView>(*containingBlock);
           containingBlock = containingBlock->ContainingBlock())
      {
        if (containingBlock->Style().Display() != DisplayType::Block)
        {
          return false;
        }
        if (containingBlock->CreatesNewFormattingContext())
        {
          return true;
        }
      }
      return true;
    };
    if (!isFlexBoxInsideBFC())
    {
      // FIXME: Nested flexing content requires integration layer handling "overriding" values (when e.g.
      // parent flex/grid FC flexes this FC)
      return false;
    }

    if (flexBoxStyle.MinHeight() != style::ComputedStyle::InitialMinHeight()
        || flexBoxStyle.MaxHeight() != style::ComputedStyle::InitialMaxHeight())
    {
      return false;
    }

    if (flexBox.IsOutOfFlowPositioned())
    {
      // FIXME: Flex box needs to be able stretch (top/left/bottom/right).
      return false;
    }

    for (auto &flexItem : ChildrenOfType<RenderElement>(flexBox))
    {
      auto &flexItemStyle = flexItem.Style();

      if (!flexItemStyle.GetFlexBasis().IsFixed())
      {
        // Note that percentage values of flex-basis are resolved against the flex item's containing block and
        // if that containing block's size is indefinite, the used value for flex-basis is content.
        return false;
      }

      if (!Is<RenderBlock>(flexItem) || flexItem.IsFieldset() || flexItem.IsRenderTextControl()
          || flexItem.IsRenderTable())
      {
        return false;
      }

      if (flexItem.IsOutOfFlowPositioned())
      {
        return false;
      }

      if (!flexItem.IsHorizontalWritingMode())
      {
        return false;
      }

      // FIXME: SVG roots are unsupported too, once there's SVG.

      if (flexItem.IsFlexibleBoxIncludingDeprecated())
      {
        return false;
      }

      if (!flexItemStyle.Height().IsFixed())
      {
        return false;
      }

      if (flexItemStyle.MinHeight() != style::ComputedStyle::InitialMinHeight()
          || flexItemStyle.MaxHeight() != style::ComputedStyle::InitialMaxHeight())
      {
        return false;
      }

      if (flexItemStyle.UsedContain().Contains(style::ContainValue::Size))
      {
        return false;
      }

      if (MayHaveScrollbarOrScrollableOverflow(flexItemStyle))
      {
        return false;
      }

      auto *flexItemBox = DynamicDowncast<RenderBox>(flexItem);
      if ((flexItemBox && flexItemBox->HasIntrinsicAspectRatio()) || flexItemStyle.HasAspectRatio())
      {
        return false;
      }

      auto isBaseline = !flexItemStyle.GetAlignSelf().IsAuto() ? flexItemStyle.GetAlignSelf().IsBaseline()
                                                               : flexBoxStyle.GetAlignItems().IsBaseline();
      if (isBaseline)
      {
        return false;
      }

      for (auto &child : ChildrenOfType<RenderElement>(flexItem))
      {
        // FIXME: Should check descendants too?
        if (child.Style().Height().IsPercentOrCalculated())
        {
          return false;
        }
      }
    }
    return true;
  }

  void FlexLayout::UpdateFormattingContextGeometries() noexcept
  {
    auto boxGeometryUpdater = BoxGeometryUpdater {GetLayoutState(), FlexBox()};
    boxGeometryUpdater.SetFormattingContextRootGeometry(
      FlexBoxRenderer().ContainingBlock()->ContentBoxLogicalWidth());
    boxGeometryUpdater.SetFormattingContextContentGeometry(
      GetLayoutState().GeometryForBox(FlexBox()).ContentBoxWidth(), {});
  }

  void FlexLayout::UpdateStyle(const RenderBlock &, const RenderStyle &) noexcept
  {
  }

  Pair<LayoutUnit, LayoutUnit> FlexLayout::ComputeIntrinsicWidthConstraints() noexcept
  {
    auto flexFormattingContext = layout::FlexFormattingContext {FlexBox(), GetLayoutState()};
    auto constraints = flexFormattingContext.ComputedIntrinsicWidthConstraints();

    return {constraints.Minimum, constraints.Maximum};
  }

  void FlexLayout::Layout() noexcept
  {
    layout::FlexFormattingContext {FlexBox(), GetLayoutState()}.Layout(ConstraintsForFlexContent(FlexBox()));

    UpdateRenderers();

    auto relayoutFlexItems = [&]
    {
      // Flex items need to be laid out now with their final size (and through
      // setOverridingBorderBoxLogicalWidth/Height) Note that they may re-size themselves.
      auto flexContainerIsHorizontal = FlexBox().GetWritingMode().IsHorizontal();
      for (auto &layoutBox : layout::FormattingContextBoxes(FlexBox()))
      {
        auto &renderer = Downcast<RenderBox>(*layoutBox.RendererForIntegration());
        auto isOrthogonal = flexContainerIsHorizontal != renderer.GetWritingMode().IsHorizontal();
        auto borderBox = layout::BoxGeometry::BorderBoxRect(GetLayoutState().GeometryForBox(layoutBox));

        renderer.SetWidth(LayoutUnit {});
        renderer.SetHeight(LayoutUnit {});
        // logical here means width and height constraints for the _content_ of the flex items not the flex
        // items' own dimension inside the flex container.
        renderer.SetOverridingBorderBoxLogicalWidth(isOrthogonal ? borderBox.Height() : borderBox.Width());
        renderer.SetOverridingBorderBoxLogicalHeight(isOrthogonal ? borderBox.Width() : borderBox.Height());

        renderer.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
        renderer.LayoutIfNeeded();
        renderer.ClearOverridingSize();

        renderer.SetWidth(flexContainerIsHorizontal ? borderBox.Width() : borderBox.Height());
        renderer.SetHeight(flexContainerIsHorizontal ? borderBox.Height() : borderBox.Width());
      }
    };
    relayoutFlexItems();
  }

  void FlexLayout::UpdateRenderers() noexcept
  {
    auto flexContainerIsHorizontal = FlexBox().GetWritingMode().IsHorizontal();
    for (auto &layoutBox : layout::FormattingContextBoxes(FlexBox()))
    {
      auto &renderer = Downcast<RenderBox>(*layoutBox.RendererForIntegration());
      auto &flexItemGeometry = GetLayoutState().GeometryForBox(layoutBox);
      auto borderBox = layout::BoxGeometry::BorderBoxRect(flexItemGeometry);
      renderer.SetLocation(flexContainerIsHorizontal ? borderBox.TopLeft()
                                                     : borderBox.TopLeft().TransposedPoint());
      renderer.SetWidth(flexContainerIsHorizontal ? borderBox.Width() : borderBox.Height());
      renderer.SetHeight(flexContainerIsHorizontal ? borderBox.Height() : borderBox.Width());

      renderer.SetMarginStart(flexItemGeometry.MarginStart());
      renderer.SetMarginEnd(flexItemGeometry.MarginEnd());
      renderer.SetMarginBefore(flexItemGeometry.MarginBefore());
      renderer.SetMarginAfter(flexItemGeometry.MarginAfter());

      if (!renderer.EverHadLayout() || renderer.CheckForRepaintDuringLayout())
      {
        renderer.Repaint();
      }
    }
  }

  void FlexLayout::CollectOverflow() noexcept
  {
  }

  LayoutUnit FlexLayout::ContentBoxLogicalHeight() const noexcept
  {
    auto contentLogicalBottom = LayoutUnit {};
    for (auto &layoutBox : layout::FormattingContextBoxes(FlexBox()))
    {
      contentLogicalBottom =
        std::max(contentLogicalBottom,
                 layout::BoxGeometry::MarginBoxRect(GetLayoutState().GeometryForBox(layoutBox)).Bottom());
    }
    return contentLogicalBottom - GetLayoutState().GeometryForBox(FlexBox()).ContentBoxTop();
  }

  const RenderFlexibleBox &FlexLayout::FlexBoxRenderer() const noexcept
  {
    return Downcast<RenderFlexibleBox>(*_flexBox->RendererForIntegration());
  }

  RenderFlexibleBox &FlexLayout::FlexBoxRenderer() noexcept
  {
    return Downcast<RenderFlexibleBox>(*_flexBox->RendererForIntegration());
  }
}
