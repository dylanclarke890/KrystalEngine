#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayLineBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBox.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

#include <ranges>

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD InlineRect MapLineRectLogicalToVisual(const InlineRect &lineLogicalRect,
                                                         const LayoutSize &formattingRootBorderBoxSize,
                                                         WritingMode writingMode) noexcept
    {
      if (writingMode.IsHorizontal())
      {
        return lineLogicalRect;
      }
      if (writingMode.IsLogicalLeftLineLeft())
      {
        return {lineLogicalRect.Left(), lineLogicalRect.Top(), lineLogicalRect.Height(),
                lineLogicalRect.Width()};
      }

      return {formattingRootBorderBoxSize.Height().ToFloat() - lineLogicalRect.Right(), lineLogicalRect.Top(),
              lineLogicalRect.Height(), lineLogicalRect.Width()};
    }

    KRYS_NODISCARD float TruncateTextContentWithMismatchingDirection(inline_display::Box &displayBox,
                                                                     float contentWidth,
                                                                     float availableWidthForContent,
                                                                     bool canFullyTruncate) noexcept
    {
      // While truncation normally starts at the end of the content and progress backwards, with mismatching
      // direction we take a different approach and truncate content the other way around (i.e. ellipsis
      // follows inline direction truncating the beginning of the content). <div dir=rtl>some long
      // content</div>
      // [...ng content]
      auto &inlineTextBox = Downcast<InlineTextBox>(displayBox.LayoutBox());
      auto &textContent = displayBox.GetText();

      auto availableWidthForTruncatedContent = contentWidth - availableWidthForContent;
      auto truncatedSide =
        TextUtil::BreakWord(inlineTextBox, textContent.Start(), textContent.Length(), contentWidth,
                            availableWidthForTruncatedContent, {}, displayBox.Style().GetFontCascade());

      krys_debug_assert(truncatedSide.Length < textContent.Length());
      auto visibleLength = textContent.Length() - truncatedSide.Length;
      auto visibleWidth = contentWidth - truncatedSide.LogicalWidth;
      // TextUtil::BreakWord never returns overflowing content (left side) which means the right side is
      // normally wider (by one character) than what we need.
      auto visibleContentOverflows = truncatedSide.LogicalWidth < availableWidthForTruncatedContent;
      if (!visibleContentOverflows)
      {
        textContent.SetPartiallyVisibleContentLength(visibleLength);
        return visibleWidth;
      }

      auto &content = inlineTextBox.Content();
      auto codePointLengthAt = [&](size_t position)
      {
        return text::DecodeUTF8(Span<const char8>(content.data() + position, content.size() - position))
          .Length;
      };
      auto firstVisibleCodePointLength = codePointLengthAt(textContent.End() - visibleLength);
      auto wouldFullyTruncate = visibleLength <= firstVisibleCodePointLength;
      if (wouldFullyTruncate && canFullyTruncate)
      {
        displayBox.SetIsFullyTruncated();
        return {};
      }

      // Drop the first visible character, or keep the last one when nothing else would be left.
      if (!wouldFullyTruncate)
      {
        visibleLength -= firstVisibleCodePointLength;
      }
      else
      {
        auto lastCodePointStart = textContent.End() - 1;
        while (lastCodePointStart > textContent.Start() && text::IsTrailingUTF8(content[lastCodePointStart]))
        {
          --lastCodePointStart;
        }
        visibleLength = textContent.End() - lastCodePointStart;
      }
      textContent.SetPartiallyVisibleContentLength(visibleLength);
      auto visibleStartPosition = textContent.End() - visibleLength;
      return TextUtil::Width(
        inlineTextBox, displayBox.Style().GetFontCascade(), static_cast<uint32>(visibleStartPosition),
        static_cast<uint32>(textContent.End()), {}, TextUtil::UseTrailingWhitespaceMeasuringOptimization::No);
    }

    KRYS_NODISCARD float Truncate(inline_display::Box &displayBox, float contentWidth,
                                  float availableWidthForContent, bool canFullyTruncate) noexcept
    {
      if (displayBox.IsText())
      {
        if (!availableWidthForContent && canFullyTruncate)
        {
          displayBox.SetIsFullyTruncated();
          return {};
        }
        auto contentDirection = displayBox.BidiLevel() % 2 ? TextDirection::RTL : TextDirection::LTR;
        if (displayBox.LayoutBox().Parent().Style().GetWritingMode().BidiDirection() != contentDirection)
        {
          return TruncateTextContentWithMismatchingDirection(displayBox, contentWidth,
                                                             availableWidthForContent, canFullyTruncate);
        }

        auto &inlineTextBox = Downcast<InlineTextBox>(displayBox.LayoutBox());
        auto &textContent = displayBox.GetText();
        auto visibleSide =
          TextUtil::BreakWord(inlineTextBox, textContent.Start(), textContent.Length(), contentWidth,
                              availableWidthForContent, {}, displayBox.Style().GetFontCascade());
        if (visibleSide.Length)
        {
          textContent.SetPartiallyVisibleContentLength(visibleSide.Length);
          return visibleSide.LogicalWidth;
        }
        if (canFullyTruncate)
        {
          displayBox.SetIsFullyTruncated();
          return {};
        }
        auto firstCharacterLength = TextUtil::FirstUserPerceivedCharacterLength(
          inlineTextBox, textContent.Start(), textContent.Length());
        auto firstCharacterWidth = TextUtil::Width(
          inlineTextBox, displayBox.Style().GetFontCascade(), static_cast<uint32>(textContent.Start()),
          static_cast<uint32>(textContent.Start() + firstCharacterLength), {},
          TextUtil::UseTrailingWhitespaceMeasuringOptimization::No);
        textContent.SetPartiallyVisibleContentLength(firstCharacterLength);
        return firstCharacterWidth;
      }

      if (canFullyTruncate)
      {
        // Atomic inline level boxes are never partially truncated.
        displayBox.SetIsFullyTruncated();
        return {};
      }
      return contentWidth;
    }

    KRYS_NODISCARD float
      TruncateOverflowingDisplayBoxes(inline_display::Boxes &boxes, size_t startIndex, size_t endIndex,
                                      float lineBoxVisualLeft, float lineBoxVisualRight, float ellipsisWidth,
                                      const RenderStyle &rootStyle,
                                      LineEndingTruncationPolicy lineEndingTruncationPolicy) noexcept
    {
      krys_debug_assert(endIndex && startIndex <= endIndex);
      // We gotta truncate some runs.
      auto isHorizontal = rootStyle.GetWritingMode().IsHorizontal();
      auto left = [&](const inline_display::Box &displayBox)
      {
        return isHorizontal ? displayBox.Left() : displayBox.Top();
      };
      auto right = [&](const inline_display::Box &displayBox)
      {
        return isHorizontal ? displayBox.Right() : displayBox.Bottom();
      };
      auto width = [&](const inline_display::Box &displayBox)
      {
        return isHorizontal ? displayBox.Width() : displayBox.Height();
      };
      // The logically first character or atomic inline-level element on a line must be clipped rather than
      // ellipsed.
      auto isFirstContentRun = true;
      if (rootStyle.GetWritingMode().IsBidiLTR())
      {
        auto visualRightForContentEnd = std::max(0.f, lineBoxVisualRight - ellipsisWidth);
        if (visualRightForContentEnd)
        {
          visualRightForContentEnd += LayoutUnit::Epsilon();
        }
        auto truncateRight = Maybe<float> {};
        for (auto index = startIndex; index <= endIndex; ++index)
        {
          auto &displayBox = boxes[index];
          if (displayBox.IsInlineBox())
          {
            continue;
          }
          if (right(displayBox) > visualRightForContentEnd)
          {
            auto visibleWidth =
              Truncate(displayBox, width(displayBox),
                       std::max(0.f, visualRightForContentEnd - left(displayBox)), !isFirstContentRun);
            auto truncatePosition = left(displayBox) + visibleWidth;
            truncateRight = truncateRight.value_or(truncatePosition);
          }
          isFirstContentRun = false;
        }
        krys_debug_assert(lineEndingTruncationPolicy
                            != LineEndingTruncationPolicy::WhenContentOverflowsInInlineDirection
                          || truncateRight.has_value() || right(boxes.back()) == visualRightForContentEnd
                          || boxes.back().IsInlineBox());
        return truncateRight.value_or(right(boxes.back()));
      }

      auto truncateLeft = Maybe<float> {};
      auto visualLeftForContentEnd = std::max(0.f, lineBoxVisualLeft + ellipsisWidth);
      if (visualLeftForContentEnd)
      {
        visualLeftForContentEnd -= LayoutUnit::Epsilon();
      }
      for (size_t index = endIndex + 1; index-- > startIndex;)
      {
        auto &displayBox = boxes[index];
        if (displayBox.IsInlineBox())
        {
          continue;
        }
        if (left(displayBox) < visualLeftForContentEnd)
        {
          auto visibleWidth =
            Truncate(displayBox, width(displayBox),
                     std::max(0.f, right(displayBox) - visualLeftForContentEnd), !isFirstContentRun);
          auto truncatePosition = right(displayBox) - visibleWidth;
          truncateLeft = truncateLeft.value_or(truncatePosition);
        }
        isFirstContentRun = false;
      }

      krys_debug_assert(lineEndingTruncationPolicy
                          != LineEndingTruncationPolicy::WhenContentOverflowsInInlineDirection
                        || truncateLeft.has_value() || left(boxes.front()) == visualLeftForContentEnd
                        || boxes.front().IsInlineBox());
      return truncateLeft.value_or(left(boxes.front())) - ellipsisWidth;
    }

    KRYS_NODISCARD Maybe<FloatRect> TrailingEllipsisVisualRectAfterTruncation(
      LineEndingTruncationPolicy lineEndingTruncationPolicy, const StringAtom &ellipsisText,
      const inline_display::Line &displayLine, inline_display::Boxes &displayBoxes) noexcept
    {
      krys_debug_assert(lineEndingTruncationPolicy != LineEndingTruncationPolicy::NoTruncation);
      if (displayBoxes.empty())
      {
        return null;
      }

      auto needsEllipsis = [&]
      {
        if (lineEndingTruncationPolicy == LineEndingTruncationPolicy::WhenContentOverflowsInInlineDirection)
        {
          return displayLine.ContentLogicalWidth()
                 && displayLine.ContentLogicalWidth() > displayLine.LineBoxLogicalRect().Width();
        }
        krys_debug_assert(lineEndingTruncationPolicy
                          == LineEndingTruncationPolicy::WhenContentOverflowsInBlockDirection);
        // We consider even the last line as overflow in block direction due to the propagated nature of
        // line-clamp where clamping is shared across sibling IFCs. If this IFC has the last formatted line
        // (in which case the last line needs no ellipsis) the parent BFC reissues a layout with no clamping.
        return true;
      };
      if (!needsEllipsis())
      {
        return null;
      }

      krys_debug_assert(displayBoxes[0].IsRootInlineBox());
      auto &rootInlineBox = displayBoxes[0];
      auto &rootStyle = rootInlineBox.Style();
      auto ellipsisWidth = std::max(0.f, rootStyle.GetFontCascade().Width(ellipsisText.View()));

      auto contentNeedsTruncation = [&]
      {
        switch (lineEndingTruncationPolicy)
        {
          case LineEndingTruncationPolicy::WhenContentOverflowsInInlineDirection:
            krys_debug_assert(displayLine.ContentLogicalWidth() > displayLine.LineBoxLogicalRect().Width());
            return true;
          case LineEndingTruncationPolicy::WhenContentOverflowsInBlockDirection:
            return displayLine.ContentLogicalLeft() + displayLine.ContentLogicalWidth() + ellipsisWidth
                   > displayLine.LineBoxLogicalRect().MaxX();
          default: krys_debug_assert(false); return false;
        }
      };

      auto ellipsisStart = 0.f;
      if (!contentNeedsTruncation())
      {
        // The content does not overflow the line box. The ellipsis is supposed to be either visually trailing
        // or leading depending on the inline direction.
        if (displayBoxes.size() > 1)
        {
          ellipsisStart = rootStyle.GetWritingMode().IsBidiLTR() ? displayBoxes.back().Right()
                                                                 : displayBoxes[1].Left() - ellipsisWidth;
        }
        else
        {
          // All we have is the root inline box.
          ellipsisStart = displayBoxes.front().Left();
        }
      }
      else
      {
        auto lineBoxVisualLeft =
          rootStyle.GetWritingMode().IsHorizontal() ? displayLine.Left() : displayLine.Top();
        auto lineBoxVisualRight =
          std::max(rootStyle.GetWritingMode().IsHorizontal() ? displayLine.Right() : displayLine.Bottom(),
                   lineBoxVisualLeft);
        ellipsisStart = TruncateOverflowingDisplayBoxes(displayBoxes, 0, displayBoxes.size() - 1,
                                                        lineBoxVisualLeft, lineBoxVisualRight, ellipsisWidth,
                                                        rootStyle, lineEndingTruncationPolicy);
      }

      if (rootStyle.GetWritingMode().IsHorizontal())
      {
        return FloatRect {ellipsisStart, rootInlineBox.Top(), ellipsisWidth, rootInlineBox.Height()};
      }
      return FloatRect {rootInlineBox.Left(), ellipsisStart, rootInlineBox.Width(), ellipsisWidth};
    }

    KRYS_NODISCARD bool IsEligibleForLinkBoxLineClamp(const inline_display::Boxes &displayBoxes) noexcept
    {
      if (displayBoxes.size() < 3)
      {
        // We need at least 3 display boxes to generate content with link ([root inline box][inline
        // box][content])
        return false;
      }
      auto writingMode = displayBoxes[0].LayoutBox().GetWritingMode();
      if (writingMode.IsBidiRTL() || writingMode.IsVertical())
      {
        return false;
      }
      auto &linkCandidateBox = displayBoxes[displayBoxes.size() - 2];
      if (!linkCandidateBox.IsNonRootInlineBox() || !linkCandidateBox.IsFirstForLayoutBox())
      {
        // Link spanning multiple lines looks odd with line-clamp.
        return false;
      }
      return linkCandidateBox.LayoutBox().Style().IsLink();
    }

    constexpr int32 LegacyMatchingLinkBoxOffset = 3;

    void MakeRoomForLinkBoxOnClampedLineIfNeeded(inline_display::Line &clampedLine,
                                                 inline_display::Boxes &displayBoxes,
                                                 size_t insertionPosition, float linkContentWidth) noexcept
    {
      krys_debug_assert(insertionPosition);
      auto ellipsisBoxRect = clampedLine.GetEllipsis()->VisualRect;
      if (ellipsisBoxRect.MaxX() + linkContentWidth <= clampedLine.Right())
      {
        return;
      }
      auto &rootStyle = displayBoxes[0].LayoutBox().Style();
      auto startIndex = [&]() -> size_t
      {
        krys_debug_assert(insertionPosition);
        for (size_t index = insertionPosition - 1; index--;)
        {
          if (displayBoxes[index].IsRootInlineBox())
          {
            return index + 1;
          }
          return index;
        }
        krys_debug_assert(false);
        return 0;
      };
      auto endIndex = insertionPosition - 1;
      auto ellispsisPosition = clampedLine.Right() - linkContentWidth - LegacyMatchingLinkBoxOffset;
      auto ellipsisStart = TruncateOverflowingDisplayBoxes(
        displayBoxes, startIndex(), endIndex, clampedLine.Left(), ellispsisPosition, ellipsisBoxRect.Width(),
        rootStyle, LineEndingTruncationPolicy::WhenContentOverflowsInBlockDirection);
      ellipsisBoxRect.SetX(ellipsisStart);
      clampedLine.SetEllipsis({inline_display::Line::Ellipsis::Type::Block, ellipsisBoxRect,
                               TextUtil::EllipsisTextInInlineDirection(clampedLine.IsHorizontal())});
    }

    void MoveDisplayBoxToClampedLine(inline_display::Lines &displayLines, size_t clampedLineIndex,
                                     inline_display::Box &displayBox, float horizontalOffset) noexcept
    {
      auto &clampedLine = displayLines[clampedLineIndex];
      displayBox.SetLeft(clampedLine.GetEllipsis()->VisualRect.MaxX() + horizontalOffset
                         + LegacyMatchingLinkBoxOffset);
      // Assume baseline alignment here.
      displayBox.MoveVertically((clampedLine.Top() + clampedLine.Baseline())
                                - (displayLines.back().Top() + displayLines.back().Baseline()));

      auto &originalLine = displayLines[displayBox.LineIndex()];
      originalLine.SetBoxCount(originalLine.BoxCount() - 1);
      displayBox.MoveToLine(static_cast<uint32>(clampedLineIndex));
      clampedLine.SetBoxCount(clampedLine.BoxCount() + 1);
    }
  }

  InlineDisplayLineBuilder::InlineDisplayLineBuilder(InlineFormattingContext &inlineFormattingContext,
                                                     const ConstraintsForInlineContent &constraints) noexcept
      : _inlineFormattingContext(inlineFormattingContext), _constraints(constraints)
  {
  }

  const Box &InlineDisplayLineBuilder::Root() const noexcept
  {
    return GetFormattingContext().Root();
  }

  InlineDisplayLineBuilder::EnclosingLineGeometry
    InlineDisplayLineBuilder::CollectEnclosingLineGeometry(const LineLayoutResult &lineLayoutResult,
                                                           const LineBox &lineBox,
                                                           const InlineRect &lineBoxRect) const noexcept
  {
    auto &rootInlineBox = lineBox.RootInlineBox();
    auto enclosingTop = Maybe<InlineLayoutUnit> {};
    auto enclosingBottom = Maybe<InlineLayoutUnit> {};
    if (lineBox.HasContent() && rootInlineBox.HasContent())
    {
      enclosingTop =
        lineBoxRect.Top() + rootInlineBox.LogicalTop() - rootInlineBox.TextEmphasisAbove().value_or(0.f);
      enclosingBottom =
        lineBoxRect.Top() + rootInlineBox.LogicalBottom() + rootInlineBox.TextEmphasisBelow().value_or(0.f);
    }
    auto contentOverflowRect = [&]() -> InlineRect
    {
      auto rect = lineBoxRect;
      auto rootInlineBoxWidth = lineBox.LogicalRectForRootInlineBox().Width();
      auto isLeftToRightDirection = Root().GetWritingMode().IsBidiLTR();
      if (lineLayoutResult.Hanging.ShouldContributeToScrollableOverflow)
      {
        rect.ExpandHorizontally(lineLayoutResult.Hanging.LogicalWidth);
      }
      else if (!isLeftToRightDirection)
      {
        // This is to balance hanging RTL trailing content. See LineBoxBuilder::Build.
        rootInlineBoxWidth -= lineLayoutResult.Hanging.LogicalWidth;
      }
      auto rootInlineBoxHorizontalOverflow = rootInlineBoxWidth - rect.Width();
      if (rootInlineBoxHorizontalOverflow > 0)
      {
        if (isLeftToRightDirection)
        {
          rect.ShiftRightBy(rootInlineBoxHorizontalOverflow);
        }
        else
        {
          rect.ShiftLeftBy(-rootInlineBoxHorizontalOverflow);
        }
      }
      return rect;
    }();
    for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
    {
      if (!inlineLevelBox.IsAtomicInlineBox() && !inlineLevelBox.IsInlineBox()
          && !inlineLevelBox.IsLineBreakBox())
      {
        continue;
      }

      auto &layoutBox = inlineLevelBox.LayoutBox();
      auto borderBox = InlineRect {};

      if (inlineLevelBox.IsAtomicInlineBox())
      {
        borderBox = lineBox.LogicalBorderBoxForAtomicInlineBox(
          layoutBox, GetFormattingContext().GeometryForBox(layoutBox));
        borderBox.MoveBy(lineBoxRect.TopLeft());
      }
      else if (inlineLevelBox.IsInlineBox())
      {
        auto &boxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
        // In standards mode, inline boxes always start with an imaginary strut.
        auto isContentful = GetFormattingContext().GetLayoutState().InStandardsMode()
                            || inlineLevelBox.HasContent() || boxGeometry.HorizontalBorderAndPadding();
        if (!isContentful)
        {
          continue;
        }
        borderBox = lineBox.LogicalBorderBoxForInlineBox(layoutBox, boxGeometry);
        borderBox.MoveBy(lineBoxRect.TopLeft());
        // Collect scrollable overflow from inline boxes. All other inline level boxes (e.g atomic inline
        // level boxes) stretch the line.
        if (lineBox.HasContent())
        {
          // Empty lines (e.g. continuation pre/post blocks) don't expect scrollbar overflow.
          contentOverflowRect.ExpandVerticallyToContain(borderBox);
        }
      }
      else if (inlineLevelBox.IsLineBreakBox())
      {
        borderBox =
          lineBox.LogicalBorderBoxForInlineBox(layoutBox, GetFormattingContext().GeometryForBox(layoutBox));
        borderBox.MoveBy(lineBoxRect.TopLeft());
      }
      else
      {
        krys_debug_assert(false);
      }

      auto adjustedBorderBoxTop = borderBox.Top() - inlineLevelBox.TextEmphasisAbove().value_or(0.f);
      auto adjustedBorderBoxBottom = borderBox.Bottom() + inlineLevelBox.TextEmphasisBelow().value_or(0.f);
      enclosingTop = std::min(enclosingTop.value_or(adjustedBorderBoxTop), adjustedBorderBoxTop);
      enclosingBottom = std::max(enclosingBottom.value_or(adjustedBorderBoxBottom), adjustedBorderBoxBottom);
    }
    return {{enclosingTop.value_or(lineBoxRect.Top()), enclosingBottom.value_or(lineBoxRect.Bottom())},
            contentOverflowRect};
  }

  inline_display::Line
    InlineDisplayLineBuilder::Build(const LineLayoutResult &lineLayoutResult, const LineBox &lineBox,
                                    bool lineIsFullyTruncatedInBlockDirection) const noexcept
  {
    auto &constraints = GetConstraints();
    auto isLeftToRightDirection =
      lineLayoutResult.BidiDirectionality.InlineBaseDirection == TextDirection::LTR;

    InlineRect lineBoxLogicalRect = lineBox.LogicalRect();
    auto lineBoxLineLeftEdge =
      isLeftToRightDirection ? lineBoxLogicalRect.Left()
                             : (constraints.VisualLeft() + constraints.Horizontal().LogicalRight()).ToFloat()
                                 - lineBoxLogicalRect.Right();
    lineBoxLogicalRect.SetLeft(lineBoxLineLeftEdge);
    auto enclosingLineGeometry = CollectEnclosingLineGeometry(lineLayoutResult, lineBox, lineBoxLogicalRect);

    auto &rootInlineBox = lineBox.RootInlineBox();
    auto rootInlineBoxRect = lineBox.LogicalRectForRootInlineBox();
    // Note that with hanging content lineLayoutResult.Content.LogicalRightIncludingNegativeMargin is not the
    // same as rootLineBoxRect.Right().
    auto contentLineLeftEdge =
      isLeftToRightDirection
        ? rootInlineBoxRect.Left()
        : lineBoxLogicalRect.Width() - lineLayoutResult.Content.LogicalRightIncludingNegativeMargin;

    auto hasInflowContent = [&]
    {
      if (lineLayoutResult.HasContentfulInFlowContent())
      {
        return true;
      }
      for (auto &run : lineLayoutResult.Runs)
      {
        if (!run.IsOpaque())
        {
          return true;
        }
      }
      return false;
    };
    auto writingMode = Root().GetWritingMode();
    return inline_display::Line {
      hasInflowContent(),
      lineBox.HasContent(),
      lineLayoutResult.IsBlockContent(),
      lineBoxLogicalRect,
      MapLineRectLogicalToVisual(lineBoxLogicalRect, constraints.FormattingRootBorderBoxSize(), writingMode),
      MapLineRectLogicalToVisual(enclosingLineGeometry.ContentOverflowRect,
                                 constraints.FormattingRootBorderBoxSize(), writingMode),
      enclosingLineGeometry.EnclosingTopAndBottom,
      rootInlineBox.LogicalTop() + rootInlineBox.Ascent(),
      lineBox.BaselineType(),
      rootInlineBoxRect.Left(),
      contentLineLeftEdge,
      rootInlineBox.LogicalWidth(),
      isLeftToRightDirection,
      rootInlineBox.LayoutBox().GetWritingMode().IsHorizontal(),
      lineIsFullyTruncatedInBlockDirection};
  }

  void InlineDisplayLineBuilder::AddLegacyLineClampTrailingLinkBoxIfApplicable(
    const InlineFormattingContext &inlineFormattingContext, const InlineLayoutState &inlineLayoutState,
    inline_display::Content &displayContent) noexcept
  {
    // This is link-box type of line clamping (-webkit-line-clamp) where we check if the inline content ends
    // in a link and move such link content next to the clamped line's trailing ellispsis. It is meant to
    // produce the following rendering
    //
    // first line
    // second line is clamped... more info
    //
    // where "more info" is a link and it comes from the end of the inline content (normally invisible due to
    // block direction clamping). It supports horizontal, left-to-right content only where the link inline box
    // has (non-split) text content and when the link content ('more info') fits the clamped line.
    auto clampedLineIndex = inlineLayoutState.LegacyClampedLineIndex();
    if (!clampedLineIndex)
    {
      return;
    }
    auto &displayLines = displayContent.LineList;
    if (displayLines.empty() || *clampedLineIndex >= displayLines.size()
        || !displayLines[*clampedLineIndex].HasEllipsis())
    {
      krys_debug_assert(false);
      return;
    }
    if (*clampedLineIndex == displayLines.size() - 1)
    {
      // No need to link-box line clamp the last line.
      return;
    }

    if (!IsEligibleForLinkBoxLineClamp(displayContent.BoxList))
    {
      return;
    }

    auto &displayBoxes = displayContent.BoxList;
    auto insertionPosition = [&]() -> Maybe<size_t>
    {
      for (size_t boxIndex = 0; boxIndex < displayBoxes.size(); ++boxIndex)
      {
        if (displayBoxes[boxIndex].LineIndex() > *clampedLineIndex)
        {
          return boxIndex;
        }
      }
      return null;
    }();

    if (!insertionPosition || !*insertionPosition || *insertionPosition >= displayBoxes.size() - 1)
    {
      // Unexpected cases where the insertion point is at the leading/trailing box. They both indicate
      // incorrect line-clamp position and would produce incorrect rendering.
      krys_debug_assert(false);
      return;
    }
    auto &clampedLine = displayLines[*clampedLineIndex];
    auto linkContentWidth = displayBoxes[displayBoxes.size() - 2].VisualRectIgnoringBlockDirection().Width();
    if (linkContentWidth >= clampedLine.LineBoxWidth())
    {
      // Not enough space for "more info" content.
      return;
    }
    auto linkContentDisplayBox = krys::move(displayBoxes.back());
    displayBoxes.pop_back();
    auto linkInlineBoxDisplayBox = krys::move(displayBoxes.back());
    displayBoxes.pop_back();

    auto handleTrailingLineBreakIfApplicable = [&]
    {
      // "more info" gets inserted after the trailing run on the clamped line unless the trailing content is
      // forced line break.
      auto &trailingDisplayBox = displayBoxes[*insertionPosition - 1];
      if (!trailingDisplayBox.IsLineBreak())
      {
        return;
      }
      // Move trailing line break after the link box.
      trailingDisplayBox.MoveHorizontally(linkContentWidth);
      --*insertionPosition;
    };
    handleTrailingLineBreakIfApplicable();

    MakeRoomForLinkBoxOnClampedLineIfNeeded(clampedLine, displayBoxes, *insertionPosition, linkContentWidth);

    // link box content
    MoveDisplayBoxToClampedLine(displayLines, *clampedLineIndex, linkContentDisplayBox,
                                inlineFormattingContext.GeometryForBox(linkInlineBoxDisplayBox.LayoutBox())
                                  .MarginBorderAndPaddingStart()
                                  .ToFloat());
    displayBoxes.insert(displayBoxes.begin() + static_cast<ptrdiff_t>(*insertionPosition),
                        linkContentDisplayBox);
    // Link inline box
    MoveDisplayBoxToClampedLine(displayLines, *clampedLineIndex, linkInlineBoxDisplayBox, 0.f);
    displayBoxes.insert(displayBoxes.begin() + static_cast<ptrdiff_t>(*insertionPosition),
                        linkInlineBoxDisplayBox);

    clampedLine.SetHasContentAfterEllipsisBox();
  }

  void InlineDisplayLineBuilder::ApplyEllipsisIfNeeded(LineEndingTruncationPolicy truncationPolicy,
                                                       inline_display::Line &displayLine,
                                                       inline_display::Boxes &displayBoxes,
                                                       bool isLegacyLineClamp) noexcept
  {
    if (truncationPolicy == LineEndingTruncationPolicy::NoTruncation || !displayBoxes.size())
    {
      return;
    }

    auto ellipsisText = [&]() -> StringAtom
    {
      if (truncationPolicy == LineEndingTruncationPolicy::WhenContentOverflowsInInlineDirection
          || isLegacyLineClamp)
      {
        // Legacy line clamp always uses ...
        return TextUtil::EllipsisTextInInlineDirection(displayLine.IsHorizontal());
      }
      return displayBoxes[0].LayoutBox().Style().GetBlockEllipsis().SwitchOn(
        [&](const css::keywords::None &) -> StringAtom { return StringAtom::Null(); },
        [&](const css::keywords::Auto &) -> StringAtom
        { return TextUtil::EllipsisTextInInlineDirection(displayLine.IsHorizontal()); },
        [&](const StringAtom &string) -> StringAtom { return string; });
    }();

    if (ellipsisText == StringAtom::Null())
    {
      return;
    }

    if (auto ellipsisRect = TrailingEllipsisVisualRectAfterTruncation(truncationPolicy, ellipsisText,
                                                                      displayLine, displayBoxes))
    {
      displayLine.SetEllipsis(
        {truncationPolicy == LineEndingTruncationPolicy::WhenContentOverflowsInInlineDirection
           ? inline_display::Line::Ellipsis::Type::Inline
           : inline_display::Line::Ellipsis::Type::Block,
         *ellipsisRect, ellipsisText});
    }
  }

  bool InlineDisplayLineBuilder::HasTrailingLineWithBlockContent(
    const inline_display::Lines &displayLines) noexcept
  {
    for (auto &line : displayLines | std::views::reverse)
    {
      if (!line.HasContentfulInFlowBox())
      {
        continue;
      }
      return line.HasBlockLevelBox();
    }
    return false;
  }
}
