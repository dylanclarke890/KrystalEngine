#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContentBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayLineBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentConstrainer.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItemsBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBox.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBoxBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/IntrinsicWidthHandler.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Invalidation/InlineInvalidation.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/RangeBasedLineBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/TextOnlySimpleLineBuilder.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD size_t EstimatedDisplayBoxSize(size_t inlineItemSize) noexcept
    {
      if (inlineItemSize == 1)
      {
        // Common case of blocks with only one word where we produce 2 boxes (root inline and text box)
        return 2;
      }
      // Let's try not to overwhelm the list's initial reserve.
      static constexpr size_t MaximumEstimatedDisplayBoxSize = 1'000;
      // This value represents a simple average derived from typical web page content.
      return std::min<size_t>(MaximumEstimatedDisplayBoxSize, static_cast<size_t>(inlineItemSize * 0.6));
    }

    KRYS_NODISCARD Maybe<InlineItemRange> PartialRangeForDamage(const InlineItemList &inlineItemList,
                                                                const InlineDamage &lineDamage) noexcept
    {
      auto layoutStartPosition = lineDamage.LayoutStartPosition()->InlineItemPos;
      if (layoutStartPosition.Index >= inlineItemList.size())
      {
        krys_debug_assert(false);
        return {};
      }
      auto *damagedInlineTextItem =
        DynamicDowncast<InlineTextItem>(inlineItemList[layoutStartPosition.Index]);
      if (layoutStartPosition.Offset
          && (!damagedInlineTextItem || layoutStartPosition.Offset >= damagedInlineTextItem->Length()))
      {
        krys_debug_assert(false);
        return {};
      }
      return InlineItemRange {layoutStartPosition, {inlineItemList.size(), 0}};
    }

    KRYS_NODISCARD bool MayExitFromPartialLayout(const InlineDamage &lineDamage, size_t lineIndex,
                                                 const inline_display::Boxes &newContent) noexcept
    {
      if (lineDamage.LayoutStartPosition()->LineIndex == lineIndex)
      {
        // Never stop at the damaged line. Adding trailing overflowing content could easily produce the
        // same set of display boxes for the first damaged line.
        return false;
      }
      auto trailingContentFromPreviousLayout = lineDamage.TrailingContentForLine(lineIndex);
      return trailingContentFromPreviousLayout
               ? (!newContent.empty() && *trailingContentFromPreviousLayout == newContent.back())
               : false;
    }

    KRYS_NODISCARD bool IsEmptyInlineContent(const InlineItemList &inlineItemList) noexcept
    {
      // Very common, pseudo before/after empty content.
      if (inlineItemList.size() != 1)
      {
        return false;
      }

      auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItemList[0]);
      return inlineTextItem && !inlineTextItem->Length();
    }

    inline void HandleAfterSideMargin(BlockLayoutState::MarginState &marginState,
                                      inline_display::Content &displayContent) noexcept
    {
      if (!InlineDisplayLineBuilder::HasTrailingLineWithBlockContent(displayContent.LineList))
      {
        marginState.CanCollapseMarginAfterWithChildren = false;
      }
    }

#ifndef NDEBUG
    KRYS_NODISCARD bool
      IsOkToAccessBoxGeometry(const Box &layoutBox, const ElementBox &rootBlockContainer,
                              Maybe<InlineFormattingContext::EscapeReason> escapeReason) noexcept
    {
      if (escapeReason
            == InlineFormattingContext::EscapeReason::InkOverflowNeedsInitialContiningBlockForStrokeWidth
          && Is<InitialContainingBlock>(layoutBox))
      {
        return true;
      }
      // This is the non-escape case of accessing a box's geometry information within the same formatting
      // context when computing static position for out-of-flow boxes.
      if (layoutBox.IsOutOfFlowPositioned())
      {
        return true;
      }
      auto containingBlock = [&]() -> const Box *
      {
        for (auto *ancestor = &layoutBox.Parent(); !Is<InitialContainingBlock>(*ancestor);
             ancestor = &ancestor->Parent())
        {
          if (ancestor->IsContainingBlockForInFlow())
          {
            return ancestor;
          }
        }
        return nullptr;
      };
      // This is the non-escape case of accessing a box's geometry information within the same formatting
      // context.
      return containingBlock() == &rootBlockContainer;
    }
#endif
  }

  InlineFormattingContext::InlineFormattingContext(const ElementBox &rootBlockContainer,
                                                   LayoutState &globalLayoutState,
                                                   BlockLayoutState &parentBlockLayoutState) noexcept
      : _rootBlockContainer(rootBlockContainer), _globalLayoutState(globalLayoutState),
        _floatingContext(rootBlockContainer, globalLayoutState, parentBlockLayoutState.GetPlacedFloats()),
        _inlineFormattingUtils(*this), _inlineQuirks(*this), _integrationUtils(globalLayoutState),
        _inlineContentCache(globalLayoutState.GetInlineContentCache(rootBlockContainer)),
        _inlineLayoutState(parentBlockLayoutState)
  {
    InitializeInlineLayoutState(globalLayoutState);
  }

  InlineLayoutResult InlineFormattingContext::Layout(const ConstraintsForInlineContent &constraints,
                                                     InlineDamage *lineDamage) noexcept
  {
    RebuildInlineItemListIfNeeded(lineDamage);

    if (FormattingUtils().ShouldDiscardRemainingContentInBlockDirection())
    {
      // This inline content may be completely collapsed (i.e. after clamped block container)
      ResetBoxGeometriesForDiscardedContent(
        {{}, {GetInlineContentCache().GetInlineItems().Content().size(), {}}}, {});
      return {};
    }

    if (!Root().HasInFlowChild() && !Root().HasOutOfFlowChild())
    {
      // Float only content does not support partial layout.
      krys_debug_assert(!InlineInvalidation::MayOnlyNeedPartialLayout(lineDamage));
      LayoutFloatContentOnly(constraints);
      return {{}, InlineLayoutResult::Range::Full};
    }

    auto &inlineItems = GetInlineContentCache().GetInlineItems();
    auto &inlineItemList = inlineItems.Content();
    auto needsLayoutRange = [&]() -> InlineItemRange
    {
      if (!InlineInvalidation::MayOnlyNeedPartialLayout(lineDamage))
      {
        return {{}, {inlineItemList.size(), {}}};
      }
      if (auto partialRange = PartialRangeForDamage(inlineItemList, *lineDamage))
      {
        return *partialRange;
      }
      // We should be able to produce partial range for partial layout.
      krys_debug_assert(false);
      // Let's turn this unexpected state to full layout.
      lineDamage = nullptr;
      return {{}, {inlineItemList.size(), {}}};
    }();

    if (needsLayoutRange.IsEmpty())
    {
      krys_debug_assert(false);
      return {};
    }

    auto previousLine = [&]() -> Maybe<PreviousLine>
    {
      if (!needsLayoutRange.Start)
      {
        return {};
      }
      if (!lineDamage || !lineDamage->LayoutStartPosition())
      {
        krys_debug_assert(false);
        return {};
      }
      auto lastLineIndex = lineDamage->LayoutStartPosition()->LineIndex - 1;
      // FIXME: We should be able to extract the last line information and provide it to layout as "previous
      // line" (ends in line break and inline direction).
      return PreviousLine {lastLineIndex, {}, {}, {}, {}};
    }();

    auto &inlineLayoutState = GetLayoutState();
    inlineLayoutState.SetLineCount(previousLine ? previousLine->LineIndex + 1 : 0);
    // FIXME: This needs partial support when line-clamped content has nested blocks.
    inlineLayoutState.SetLineCountWithInlineContentIncludingNestedBlocks(inlineLayoutState.LineCount());

    auto textWrapStyle = Root().Style().GetTextWrapStyle();
    if (Root().Style().GetTextWrapMode() == TextWrapMode::Wrap
        && (textWrapStyle == TextWrapStyle::Balance || textWrapStyle == TextWrapStyle::Pretty))
    {
      auto constrainer = InlineContentConstrainer {*this, inlineItemList, constraints.Horizontal()};
      auto constrainedLineWidths = constrainer.ComputeParagraphLevelConstraints(textWrapStyle);
      if (constrainedLineWidths)
      {
        inlineLayoutState.SetAvailableLineWidthOverride({krys::move(*constrainedLineWidths)});
      }
    }

    if (TextOnlySimpleLineBuilder::IsEligibleForSimplifiedTextOnlyInlineLayoutByContent(
          inlineItems, inlineLayoutState.GetPlacedFloats())
        && TextOnlySimpleLineBuilder::IsEligibleForSimplifiedInlineLayoutByStyle(Root()))
    {
      auto simplifiedLineBuilder =
        TextOnlySimpleLineBuilder {*this, Root(), constraints.Horizontal(), inlineItemList};
      return LineLayout(simplifiedLineBuilder, inlineItemList, needsLayoutRange, previousLine, constraints,
                        lineDamage);
    }
    if (RangeBasedLineBuilder::IsEligibleForRangeInlineLayout(*this, needsLayoutRange, inlineItems,
                                                              inlineLayoutState.GetPlacedFloats()))
    {
      auto rangeBasedLineBuilder = RangeBasedLineBuilder {*this, constraints.Horizontal(), inlineItems};
      return LineLayout(rangeBasedLineBuilder, inlineItemList, needsLayoutRange, previousLine, constraints,
                        lineDamage);
    }
    auto lineBuilder = LineBuilder {*this, constraints.Horizontal(), inlineItemList,
                                    GetInlineContentCache().GetTextSpacingContext()};
    return LineLayout(lineBuilder, inlineItemList, needsLayoutRange, previousLine, constraints, lineDamage);
  }

  std::pair<LayoutUnit, LayoutUnit>
    InlineFormattingContext::MinimumMaximumContentSize(InlineDamage *lineDamage) noexcept
  {
    auto &inlineContentCache = GetInlineContentCache();
    auto minimumContentSize = inlineContentCache.MinimumContentSize();
    auto maximumContentSize = inlineContentCache.MaximumContentSize();

    if (minimumContentSize && maximumContentSize)
    {
      return {LayoutUnit::FromFloatCeil(*minimumContentSize), LayoutUnit::FromFloatCeil(*maximumContentSize)};
    }

    RebuildInlineItemListIfNeeded(lineDamage);
    auto &inlineItems = inlineContentCache.GetInlineItems();

    if (!IsEmptyInlineContent(inlineItems.Content()))
    {
      auto intrinsicWidthHandler = IntrinsicWidthHandler {*this, inlineItems};

      if (!minimumContentSize)
      {
        minimumContentSize = intrinsicWidthHandler.MinimumContentSize();
      }
      if (!maximumContentSize)
      {
        maximumContentSize = intrinsicWidthHandler.MaximumContentSize();
        if (intrinsicWidthHandler.MaximumIntrinsicWidthLineContent())
        {
          inlineContentCache.SetMaximumIntrinsicWidthLineContent(
            krys::move(*intrinsicWidthHandler.MaximumIntrinsicWidthLineContent()));
        }
      }
    }
    else
    {
      minimumContentSize = minimumContentSize.value_or(0.f);
      maximumContentSize = maximumContentSize.value_or(0.f);
    }
    // FIXME: "Nominally, the smallest size a box could take that doesn't lead to overflow that could be
    // avoided by choosing a larger size. Formally, the size of the box when sized under a min-content
    // constraint" 'nominally' seems to overrule 'formally' when inline content has negative text indent. This
    // also undermines the idea of computing min/max values independently.
    minimumContentSize = std::min(*minimumContentSize, *maximumContentSize);

    inlineContentCache.SetMinimumContentSize(*minimumContentSize);
    inlineContentCache.SetMaximumContentSize(*maximumContentSize);
    return {LayoutUnit::FromFloatCeil(*minimumContentSize), LayoutUnit::FromFloatCeil(*maximumContentSize)};
  }

  LayoutUnit InlineFormattingContext::MinimumContentSize(InlineDamage *lineDamage) noexcept
  {
    auto &inlineContentCache = GetInlineContentCache();
    if (inlineContentCache.MinimumContentSize())
    {
      return LayoutUnit::FromFloatCeil(*inlineContentCache.MinimumContentSize());
    }

    RebuildInlineItemListIfNeeded(lineDamage);
    auto &inlineItems = inlineContentCache.GetInlineItems();
    auto minimumContentSize = InlineLayoutUnit {};
    if (!IsEmptyInlineContent(inlineItems.Content()))
    {
      minimumContentSize = IntrinsicWidthHandler {*this, inlineItems}.MinimumContentSize();
    }
    inlineContentCache.SetMinimumContentSize(minimumContentSize);
    return LayoutUnit::FromFloatCeil(minimumContentSize);
  }

  LayoutUnit InlineFormattingContext::MaximumContentSize(InlineDamage *lineDamage) noexcept
  {
    auto &inlineContentCache = GetInlineContentCache();
    if (inlineContentCache.MaximumContentSize())
    {
      return LayoutUnit::FromFloatCeil(*inlineContentCache.MaximumContentSize());
    }

    RebuildInlineItemListIfNeeded(lineDamage);
    auto &inlineItems = inlineContentCache.GetInlineItems();
    auto maximumContentSize = InlineLayoutUnit {};
    if (!IsEmptyInlineContent(inlineItems.Content()))
    {
      auto intrinsicWidthHandler = IntrinsicWidthHandler {*this, inlineItems};

      maximumContentSize = intrinsicWidthHandler.MaximumContentSize();
      if (intrinsicWidthHandler.MaximumIntrinsicWidthLineContent())
      {
        inlineContentCache.SetMaximumIntrinsicWidthLineContent(
          krys::move(*intrinsicWidthHandler.MaximumIntrinsicWidthLineContent()));
      }
    }
    inlineContentCache.SetMaximumContentSize(maximumContentSize);
    return LayoutUnit::FromFloatCeil(maximumContentSize);
  }

  InlineLayoutResult InlineFormattingContext::LineLayout(AbstractLineBuilder &lineBuilder,
                                                         const InlineItemList &inlineItemList,
                                                         InlineItemRange needsLayoutRange,
                                                         Maybe<PreviousLine> previousLine,
                                                         const ConstraintsForInlineContent &constraints,
                                                         const InlineDamage *lineDamage) noexcept
  {
    krys_debug_assert(!needsLayoutRange.IsEmpty());
    auto layoutResult = InlineLayoutResult {};
    auto &inlineLayoutState = GetLayoutState();
    auto &marginState = inlineLayoutState.ParentBlockLayoutState().GetMarginState();

    auto isPartialLayout = InlineInvalidation::MayOnlyNeedPartialLayout(lineDamage);
    krys_debug_assert(isPartialLayout || !previousLine);

    if (!isPartialLayout
        && (CreateDisplayContentForLineFromCachedContent(constraints, layoutResult)
            || CreateDisplayContentForEmptyInlineContent(constraints, inlineItemList, layoutResult)))
    {
      layoutResult.ContentRange = InlineLayoutResult::Range::Full;
      HandleAfterSideMargin(marginState, layoutResult.DisplayContent);
      return layoutResult;
    }

    if (!needsLayoutRange.Start)
    {
      layoutResult.DisplayContent.BoxList.reserve(EstimatedDisplayBoxSize(inlineItemList.size()));
    }

    auto &floatingContext = GetFloatingContext();
    auto lineLogicalTop = constraints.LogicalTop().ToFloat();
    auto previousLineEnd = Maybe<InlineItemPosition> {};
    auto leadingInlineItemPosition = needsLayoutRange.Start;
    auto isFirstFormattedLineCandidate = !previousLine || !previousLine->LineIndex;
    while (true)
    {
      auto lineInitialRect = InlineRect {lineLogicalTop, constraints.Horizontal().LogicalLeft.ToFloat(),
                                         constraints.Horizontal().LogicalWidth.ToFloat(),
                                         FormattingUtils().InitialLineHeight(!previousLine.has_value())};
      auto lineInput = LineInput {{leadingInlineItemPosition, needsLayoutRange.End}, lineInitialRect};
      auto lineIndex = previousLine ? (previousLine->LineIndex + 1) : size_t {0};

      auto lineLayoutResult =
        lineBuilder.LayoutInlineContent(lineInput, previousLine, isFirstFormattedLineCandidate);
      auto lineBox = LineBoxBuilder {*this, lineLayoutResult}.Build(lineIndex);
      inlineLayoutState.SetLineCount(inlineLayoutState.LineCount() + (lineBox.HasContent() ? 1 : 0));
      auto lineLogicalRect = CreateDisplayContentForInlineContent(lineBox, lineLayoutResult, constraints,
                                                                  layoutResult.DisplayContent);
      UpdateBoxGeometryForPlacedFloats(lineLayoutResult.Floats.PlacedFloats);
      UpdateLayoutStateWithLineLayoutResult(lineLayoutResult, lineLogicalRect, floatingContext);

      auto lineContentEnd = lineLayoutResult.ItemRange.End;
      leadingInlineItemPosition = InlineFormattingUtils::LeadingInlineItemPositionForNextLine(
        lineContentEnd, previousLineEnd,
        !lineLayoutResult.Floats.HasIntrusiveFloat.IsEmpty() || !lineLayoutResult.Floats.PlacedFloats.empty(),
        needsLayoutRange.End);

      auto isEndOfContent =
        leadingInlineItemPosition == needsLayoutRange.End && lineLayoutResult.Floats.SuspendedFloats.empty();
      if (isEndOfContent)
      {
        layoutResult.ContentRange =
          !isPartialLayout ? InlineLayoutResult::Range::Full : InlineLayoutResult::Range::FullFromDamage;
        break;
      }
      if (isPartialLayout
          && MayExitFromPartialLayout(*lineDamage, lineIndex, layoutResult.DisplayContent.BoxList))
      {
        layoutResult.ContentRange = InlineLayoutResult::Range::PartialFromDamage;
        break;
      }

      if (FormattingUtils().ShouldDiscardRemainingContentInBlockDirection())
      {
        ResetBoxGeometriesForDiscardedContent({leadingInlineItemPosition, needsLayoutRange.End},
                                              lineLayoutResult.Floats.SuspendedFloats);
        layoutResult.ContentRange =
          !isPartialLayout ? InlineLayoutResult::Range::Full : InlineLayoutResult::Range::FullFromDamage;
        layoutResult.DidDiscardContent = true;
        break;
      }

      previousLine = PreviousLine {lineIndex, lineLayoutResult.Content.TrailingOverflowingContentWidth,
                                   lineLayoutResult.EndsWithLineBreak(),
                                   lineLayoutResult.BidiDirectionality.InlineBaseDirection,
                                   krys::move(lineLayoutResult.Floats.SuspendedFloats)};
      previousLineEnd = lineContentEnd;
      isFirstFormattedLineCandidate &= !lineLayoutResult.HasContentfulInFlowContent();
      lineLogicalTop = FormattingUtils().LogicalTopForNextLine(lineLayoutResult, lineLogicalRect,
                                                               floatingContext, marginState);
      marginState.ContentOffsetAfterSelfCollapsingBlock = LayoutUnit {};
    }
    InlineDisplayLineBuilder::AddLegacyLineClampTrailingLinkBoxIfApplicable(*this, inlineLayoutState,
                                                                            layoutResult.DisplayContent);
    HandleAfterSideMargin(marginState, layoutResult.DisplayContent);

    return layoutResult;
  }

  void
    InlineFormattingContext::LayoutFloatContentOnly(const ConstraintsForInlineContent &constraints) noexcept
  {
    krys_debug_assert(!Root().HasInFlowChild());

    auto &inlineContentCache = GetInlineContentCache();
    auto &floatingContext = GetFloatingContext();
    auto &placedFloats = GetLayoutState().GetPlacedFloats();

    InlineItemsBuilder {inlineContentCache, Root()}.Build({});

    for (auto &inlineItem : inlineContentCache.GetInlineItems().Content())
    {
      if (inlineItem.IsFloat())
      {
        auto &floatBox = inlineItem.LayoutBox();

        GetIntegrationUtils().LayoutWithFormattingContextForBox(Downcast<ElementBox>(floatBox));

        auto &floatBoxGeometry = GeometryForBox(floatBox);
        auto staticPosition = LayoutPoint {constraints.Horizontal().LogicalLeft, constraints.LogicalTop()};
        staticPosition.Move(floatBoxGeometry.MarginStart(), floatBoxGeometry.MarginBefore());
        floatBoxGeometry.SetTopLeft(staticPosition);

        auto floatBoxTopLeft =
          floatingContext.PositionForFloat(floatBox, floatBoxGeometry, constraints.Horizontal());
        floatBoxGeometry.SetTopLeft(floatBoxTopLeft);
        placedFloats.Add(floatingContext.MakeFloatItem(floatBox, floatBoxGeometry));
        continue;
      }
      krys_debug_assert(false);
    }
  }

  void InlineFormattingContext::UpdateLayoutStateWithLineLayoutResult(
    const LineLayoutResult &lineLayoutResult, const InlineRect &lineLogicalRect,
    const FloatingContext &floatingContext) noexcept
  {
    auto &layoutState = GetLayoutState();
    if (auto firstLineGap = lineLayoutResult.Geometry.InitialLetterClearGap)
    {
      krys_debug_assert(!layoutState.ClearGapBeforeFirstLine());
      layoutState.SetClearGapBeforeFirstLine(*firstLineGap);
    }

    if (lineLayoutResult.FirstLast.IsLastLineWithInlineContent)
    {
      auto logicalTopCandidate =
        FormattingUtils().LogicalTopForNextLine(lineLayoutResult, lineLogicalRect, floatingContext,
                                                layoutState.ParentBlockLayoutState().GetMarginState());
      layoutState.SetClearGapAfterLastLine(std::max(0.f, logicalTopCandidate - lineLogicalRect.Bottom()));
    }

    if (lineLayoutResult.EndsWithHyphen())
    {
      layoutState.IncrementSuccessiveHyphenatedLineCount();
    }
    else
    {
      layoutState.ResetSuccessiveHyphenatedLineCount();
    }
    layoutState.SetFirstLineStartTrimForInitialLetter(lineLayoutResult.FirstLineStartTrim);
  }

  void InlineFormattingContext::UpdateBoxGeometryForPlacedFloats(
    const LineLayoutResult::PlacedFloatList &placedFloats) noexcept
  {
    for (auto &floatItem : placedFloats)
    {
      if (!floatItem.LayoutBox())
      {
        krys_debug_assert(false);
        // We should not be placing intrusive floats coming from parent BFC.
        continue;
      }
      auto &boxGeometry = GeometryForBox(*floatItem.LayoutBox());
      auto usedGeometry = floatItem.GetBoxGeometry();
      boxGeometry.SetTopLeft(BoxGeometry::BorderBoxTopLeft(usedGeometry));
      // Adopt trimmed inline direction margin.
      boxGeometry.SetHorizontalMargin(usedGeometry.HorizontalMargin());
    }
  }

  InlineRect InlineFormattingContext::CreateDisplayContentForInlineContent(
    const LineBox &lineBox, const LineLayoutResult &lineLayoutResult,
    const ConstraintsForInlineContent &constraints, inline_display::Content &displayContent) noexcept
  {
    auto &inlineLayoutState = GetLayoutState();
    auto lineClamp = inlineLayoutState.ParentBlockLayoutState().GetLineClamp();
    // Eligible lines from nested block containers are already included (see
    // LayoutWithFormattingContextForBlockInInline).
    auto numberOfLinesWithInlineContent = inlineLayoutState.LineCountWithInlineContentIncludingNestedBlocks()
                                          + (lineLayoutResult.HasContentfulInlineContent() ? 1 : 0);
    auto numberOfVisibleLinesAllowed = lineClamp ? Maybe<size_t> {lineClamp->MaximumLines} : null;

    auto lineIsFullyTruncatedInBlockDirection =
      numberOfVisibleLinesAllowed ? numberOfLinesWithInlineContent > *numberOfVisibleLinesAllowed : false;
    auto displayLine = InlineDisplayLineBuilder {*this, constraints}.Build(
      lineLayoutResult, lineBox, lineIsFullyTruncatedInBlockDirection);
    auto boxes =
      InlineDisplayContentBuilder {*this, constraints, lineBox, displayLine}.Build(lineLayoutResult);
    displayLine.SetBoxCount(boxes.size());

    auto addTrailingEllipsisIfApplicable = [&]
    {
      if (lineLayoutResult.IsBlockContent())
      {
        // When a block line is clamped, its content gets clamped and not this line itself.
        return;
      }
      auto isLegacyLineClamp = lineClamp && lineClamp->IsLegacy;
      auto truncationPolicy = InlineFormattingUtils::GetLineEndingTruncationPolicy(
        Root().Style(), numberOfLinesWithInlineContent, numberOfVisibleLinesAllowed, lineBox.HasContent());
      InlineDisplayLineBuilder::ApplyEllipsisIfNeeded(truncationPolicy, displayLine, boxes,
                                                      isLegacyLineClamp);
      auto lineHasLegacyLineClamp =
        isLegacyLineClamp && displayLine.HasEllipsis()
        && truncationPolicy == LineEndingTruncationPolicy::WhenContentOverflowsInBlockDirection;
      if (lineHasLegacyLineClamp)
      {
        inlineLayoutState.SetLegacyClampedLineIndex(lineBox.LineIndex());
      }
    };
    addTrailingEllipsisIfApplicable();

    displayContent.BoxList.insert(displayContent.BoxList.end(), std::make_move_iterator(boxes.begin()),
                                  std::make_move_iterator(boxes.end()));
    displayContent.LineList.push_back(displayLine);
    inlineLayoutState.SetLineCountWithInlineContentIncludingNestedBlocks(numberOfLinesWithInlineContent);
    return InlineFormattingUtils::FlipVisualRectToLogicalForWritingMode(
      displayContent.LineList.back().LineBoxRect(), Root().GetWritingMode());
  }

  void InlineFormattingContext::ResetBoxGeometriesForDiscardedContent(
    const InlineItemRange &discardedRange,
    const LineLayoutResult::SuspendedFloatList &suspendedFloats) noexcept
  {
    if (discardedRange.IsEmpty() && suspendedFloats.empty())
    {
      return;
    }

    auto &inlineItemList = GetInlineContentCache().GetInlineItems().Content();
    for (auto index = discardedRange.StartIndex(); index < discardedRange.EndIndex(); ++index)
    {
      auto &inlineItem = inlineItemList[index];
      auto hasBoxGeometry = inlineItem.IsAtomicInlineBox() || inlineItem.IsFloat()
                            || inlineItem.IsHardLineBreak() || inlineItem.IsInlineBoxStart()
                            || inlineItem.IsOpaque();
      if (!hasBoxGeometry)
      {
        continue;
      }
      GeometryForBox(inlineItem.LayoutBox()).Reset();
    }

    for (auto *floatBox : suspendedFloats)
    {
      GeometryForBox(*floatBox).Reset();
    }
  }

  bool InlineFormattingContext::CreateDisplayContentForLineFromCachedContent(
    const ConstraintsForInlineContent &constraints, InlineLayoutResult &layoutResult) noexcept
  {
    auto &inlineContentCache = GetInlineContentCache();

    if (!inlineContentCache.MaximumIntrinsicWidthLineContent() || !inlineContentCache.MaximumContentSize())
    {
      return false;
    }

    auto horizontalAvailableSpace = constraints.Horizontal().LogicalWidth;
    if (*inlineContentCache.MaximumContentSize() > horizontalAvailableSpace.ToFloat())
    {
      inlineContentCache.ClearMaximumIntrinsicWidthLineContent();
      return false;
    }
    if (!GetLayoutState().GetPlacedFloats().IsEmpty())
    {
      inlineContentCache.ClearMaximumIntrinsicWidthLineContent();
      return false;
    }

    auto &lineContent = *inlineContentCache.MaximumIntrinsicWidthLineContent();
    auto restoreTrimmedTrailingWhitespaceIfApplicable = [&]() -> Maybe<bool>
    {
      // Special 'line-break: after-white-space' behavior where min/max width trims trailing whitespace, while
      // layout should preserve _overflowing_ trailing whitespace.
      if (Root().Style().GetLineBreak() != LineBreak::AfterWhiteSpace
          || !lineContent.TrimmedTrailingWhitespaceWidth)
      {
        return null;
      }
      if (LayoutUnit::FromFloatCeil(lineContent.Content.LogicalWidth).ToFloat() + LayoutUnit::Epsilon()
          <= horizontalAvailableSpace.ToFloat())
      {
        return null;
      }
      if (!Line::RestoreTrimmedTrailingWhitespace(lineContent.TrimmedTrailingWhitespaceWidth,
                                                  lineContent.Runs, lineContent.ItemRange,
                                                  inlineContentCache.GetInlineItems().Content()))
      {
        krys_debug_assert(false);
        return false;
      }
      lineContent.Content.LogicalWidth += lineContent.TrimmedTrailingWhitespaceWidth;
      lineContent.Content.LogicalRightIncludingNegativeMargin += lineContent.TrimmedTrailingWhitespaceWidth;
      lineContent.TrimmedTrailingWhitespaceWidth = {};
      return true;
    };
    auto successfullyTrimmed = restoreTrimmedTrailingWhitespaceIfApplicable();
    if (successfullyTrimmed && !*successfullyTrimmed)
    {
      inlineContentCache.ClearMaximumIntrinsicWidthLineContent();
      return false;
    }

    lineContent.Geometry.LogicalTopLeft = {constraints.Horizontal().LogicalLeft.ToFloat(),
                                           constraints.LogicalTop().ToFloat()};
    lineContent.Geometry.LogicalWidth = constraints.Horizontal().LogicalWidth.ToFloat();
    lineContent.Content.LogicalLeft = InlineFormattingUtils::HorizontalAlignmentOffset(
      Root().Style(), lineContent.Content.LogicalWidth, lineContent.Geometry.LogicalWidth,
      lineContent.Hanging.LogicalWidth, true);
    auto lineBox = LineBoxBuilder {*this, lineContent}.Build({});
    (void)CreateDisplayContentForInlineContent(lineBox, lineContent, constraints,
                                               layoutResult.DisplayContent);
    return true;
  }

  bool InlineFormattingContext::CreateDisplayContentForEmptyInlineContent(
    const ConstraintsForInlineContent &constraints, const InlineItemList &inlineItemList,
    InlineLayoutResult &layoutResult) noexcept
  {
    if (!IsEmptyInlineContent(inlineItemList))
    {
      return false;
    }

    auto emptyLineBreakingResult = LineLayoutResult {};
    emptyLineBreakingResult.Geometry.LogicalTopLeft = {constraints.Horizontal().LogicalLeft.ToFloat(),
                                                       constraints.LogicalTop().ToFloat()};
    emptyLineBreakingResult.Geometry.LogicalWidth = constraints.Horizontal().LogicalWidth.ToFloat();
    auto lineBox = LineBoxBuilder {*this, emptyLineBreakingResult}.Build({});
    (void)CreateDisplayContentForInlineContent(lineBox, emptyLineBreakingResult, constraints,
                                               layoutResult.DisplayContent);
    return true;
  }

  void InlineFormattingContext::InitializeInlineLayoutState(const LayoutState &globalLayoutState) noexcept
  {
    auto &inlineLayoutState = GetLayoutState();

    if (auto limitLinesValue = Root().Style().GetHyphenateLimitLines().TryValue())
    {
      inlineLayoutState.SetHyphenationLimitLines(static_cast<size_t>(limitLinesValue->Value));
    }
    // FIXME: Remove when IFC takes care of running layout on inline-blocks.
    inlineLayoutState.SetShouldNotSynthesizeInlineBlockBaseline();
    if (globalLayoutState.InStandardsMode())
    {
      inlineLayoutState.SetInStandardsMode();
    }
    if (globalLayoutState.IsTextShapingAcrossInlineBoxesEnabled())
    {
      inlineLayoutState.SetShouldShapeTextAcrossInlineBoxes();
    }
  }

  const BoxGeometry &InlineFormattingContext::GeometryForBox(const Box &layoutBox,
                                                             Maybe<EscapeReason> escapeReason) const noexcept
  {
#ifndef NDEBUG
    krys_debug_assert(IsOkToAccessBoxGeometry(layoutBox, Root(), escapeReason));
#else
    (void)escapeReason;
#endif
    return _globalLayoutState.GeometryForBox(layoutBox);
  }

  BoxGeometry &InlineFormattingContext::GeometryForBox(const Box &layoutBox) noexcept
  {
#ifndef NDEBUG
    krys_debug_assert(IsOkToAccessBoxGeometry(layoutBox, Root(), null));
#endif
    return _globalLayoutState.EnsureGeometryForBox(layoutBox);
  }

  void InlineFormattingContext::RebuildInlineItemListIfNeeded(InlineDamage *lineDamage) noexcept
  {
    auto &inlineContentCache = GetInlineContentCache();
    auto inlineItemListNeedsUpdate =
      inlineContentCache.GetInlineItems().IsEmpty() || (lineDamage && lineDamage->IsInlineItemListDirty());
    if (!inlineItemListNeedsUpdate)
    {
      return;
    }

    auto startPositionForInlineItemsBuilding = [&]() -> InlineItemPosition
    {
      if (!lineDamage)
      {
        krys_debug_assert(inlineContentCache.GetInlineItems().IsEmpty());
        return {};
      }
      if (auto startPosition = lineDamage->LayoutStartPosition())
      {
        if (lineDamage->Reasons().Contains(InlineDamage::Reason::Pagination))
        {
          // FIXME: We don't support partial rebuild with certain types of content. Let's just re-collect
          // inline items.
          return {};
        }
        return startPosition->InlineItemPos;
      }
      // Unsupported damage. Need to run full build/layout.
      return {};
    };
    InlineItemsBuilder {inlineContentCache, Root()}.Build(startPositionForInlineItemsBuilding());
    if (lineDamage)
    {
      lineDamage->SetInlineItemListClean();
    }
    inlineContentCache.ClearMaximumIntrinsicWidthLineContent();
  }
}
