#include "Krystal.Booey/Layout/Integration/LayoutIntegrationUtils.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationFormattingContextLayout.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"

namespace krys::boo::layout
{
  IntegrationUtils::IntegrationUtils(const LayoutState &globalLayoutState) noexcept
      : _globalLayoutState(ShareCheckedRef(globalLayoutState))
  {
  }

  void IntegrationUtils::LayoutWithFormattingContextForBox(const ElementBox &box,
                                                           Maybe<LayoutUnit> widthConstraint,
                                                           Maybe<LayoutUnit> heightConstraint) const noexcept
  {
    _globalLayoutState->LayoutWithFormattingContextForBox(box, widthConstraint, heightConstraint);
  }

  LayoutUnit IntegrationUtils::MaxContentWidth(const ElementBox &box) const noexcept
  {
    krys_debug_assert(box.IsFlexItem());
    return _globalLayoutState->LogicalWidthWithFormattingContextForBox(
      box, layout_integration::LogicalWidthType::MaxContent);
  }

  LayoutUnit IntegrationUtils::MinContentWidth(const ElementBox &box) const noexcept
  {
    krys_debug_assert(box.IsFlexItem());
    return _globalLayoutState->LogicalWidthWithFormattingContextForBox(
      box, layout_integration::LogicalWidthType::MinContent);
  }

  LayoutUnit IntegrationUtils::MinContentHeight(const ElementBox &box) const noexcept
  {
    krys_debug_assert(box.IsFlexItem());
    return _globalLayoutState->LogicalHeightWithFormattingContextForBox(
      box, layout_integration::LogicalHeightType::MinContent);
  }

  LayoutUnit IntegrationUtils::PreferredMinWidth(const ElementBox &box) const noexcept
  {
    krys_debug_assert(box.IsGridItem());
    return _globalLayoutState->LogicalWidthWithFormattingContextForBox(
      box, layout_integration::LogicalWidthType::PreferredMinimum);
  }

  LayoutUnit IntegrationUtils::PreferredMaxWidth(const ElementBox &box) const noexcept
  {
    krys_debug_assert(box.IsGridItem());
    return _globalLayoutState->LogicalWidthWithFormattingContextForBox(
      box, layout_integration::LogicalWidthType::PreferredMaximum);
  }

  void IntegrationUtils::LayoutWithFormattingContextForBlockInInline(
    const ElementBox &block, LayoutPoint blockLineLogicalTopLeft,
    const InlineLayoutState &inlineLayoutState) const noexcept
  {
    krys_debug_assert(block.IsBlockLevelBox());
    _globalLayoutState->LayoutWithFormattingContextForBlockInInline(block, blockLineLogicalTopLeft,
                                                                    inlineLayoutState);
  }

  BlockLayoutState::MarginState
    IntegrationUtils::ToMarginState(const RenderBlockFlow::MarginInfo &marginInfo,
                                    LayoutUnit contentOffsetAfterSelfCollapsingBlock) noexcept
  {
    return {marginInfo.CanCollapseWithChildren(),
            marginInfo.CanCollapseMarginBeforeWithChildren(),
            marginInfo.CanCollapseMarginAfterWithChildren(),
            marginInfo.QuirkContainer(),
            marginInfo.AtBeforeSideOfBlock(),
            marginInfo.AtAfterSideOfBlock(),
            marginInfo.HasMarginBeforeQuirk(),
            marginInfo.HasMarginAfterQuirk(),
            marginInfo.DeterminedMarginBeforeQuirk(),
            marginInfo.PositiveMargin(),
            marginInfo.NegativeMargin(),
            contentOffsetAfterSelfCollapsingBlock};
  }

  RenderBlockFlow::MarginInfo
    IntegrationUtils::ToMarginInfo(const BlockLayoutState::MarginState &marginState) noexcept
  {
    return {marginState.CanCollapseWithChildren,
            marginState.CanCollapseMarginBeforeWithChildren,
            marginState.CanCollapseMarginAfterWithChildren,
            marginState.QuirkContainer,
            marginState.AtBeforeSideOfBlock,
            marginState.AtAfterSideOfBlock,
            marginState.HasMarginBeforeQuirk,
            marginState.HasMarginAfterQuirk,
            marginState.DeterminedMarginBeforeQuirk,
            marginState.PositiveMargin,
            marginState.NegativeMargin};
  }

  Pair<LayoutRect, LayoutRect> IntegrationUtils::ToMarginAndBorderBoxVisualRect(
    const BoxGeometry &logicalGeometry, const LayoutSize &containerSize, WritingMode writingMode) noexcept
  {
    // In certain writing modes, IFC gets the border box position wrong;
    // but the margin box is correct, so use it to derive the border box.
    auto marginBoxLogicalRect = BoxGeometry::MarginBoxRect(logicalGeometry);
    auto containerLogicalWidth = writingMode.IsHorizontal() ? containerSize.Width() : containerSize.Height();
    auto marginBoxLogicalX = writingMode.IsInlineFlipped()
                               ? containerLogicalWidth - marginBoxLogicalRect.Right()
                               : marginBoxLogicalRect.Left();
    auto marginBoxVisualRect = writingMode.IsHorizontal()
                                 ? LayoutRect {marginBoxLogicalX, marginBoxLogicalRect.Top(),
                                               marginBoxLogicalRect.Width(), marginBoxLogicalRect.Height()}
                                 : LayoutRect {marginBoxLogicalRect.Top(), marginBoxLogicalX,
                                               marginBoxLogicalRect.Height(), marginBoxLogicalRect.Width()};

    auto marginLeft = LayoutUnit {};
    auto marginTop = LayoutUnit {};
    auto marginWidth = LayoutUnit {};
    auto marginHeight = LayoutUnit {};
    if (writingMode.IsHorizontal())
    {
      marginLeft =
        writingMode.IsInlineLeftToRight() ? logicalGeometry.MarginStart() : logicalGeometry.MarginEnd();
      marginTop =
        writingMode.IsBlockTopToBottom() ? logicalGeometry.MarginBefore() : logicalGeometry.MarginAfter();
      marginWidth = logicalGeometry.MarginStart() + logicalGeometry.MarginEnd();
      marginHeight = logicalGeometry.MarginBefore() + logicalGeometry.MarginAfter();
    }
    else
    {
      // Invert verticalLogicalMargin() and convert to unflipped coords.
      marginLeft =
        writingMode.IsLineInverted() ? logicalGeometry.MarginAfter() : logicalGeometry.MarginBefore();
      marginTop =
        writingMode.IsInlineTopToBottom() ? logicalGeometry.MarginStart() : logicalGeometry.MarginEnd();
      marginWidth = logicalGeometry.MarginBefore() + logicalGeometry.MarginAfter();
      marginHeight = logicalGeometry.MarginStart() + logicalGeometry.MarginEnd();
    }

    auto borderBoxVisualRect = marginBoxVisualRect;
    borderBoxVisualRect.Expand(-marginWidth, -marginHeight);
    borderBoxVisualRect.Move(marginLeft, marginTop);
    return {marginBoxVisualRect, borderBoxVisualRect};
  }
}
