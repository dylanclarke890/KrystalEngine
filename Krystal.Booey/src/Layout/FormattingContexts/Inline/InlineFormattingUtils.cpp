#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLevelBox.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Ruby/RubyFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include <cmath>
#include <ranges>

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD inline bool EndsWithSoftWrapOpportunity(const InlineTextItem &previousInlineTextItem,
                                                           const InlineTextItem &nextInlineTextItem) noexcept
    {
      krys_debug_assert(!nextInlineTextItem.IsWhitespace());
      // We are at the position after a whitespace.
      if (previousInlineTextItem.IsWhitespace())
      {
        return true;
      }
      // When both these non-whitespace runs belong to the same layout box with the same bidi level, it's
      // guaranteed that they are split at a soft breaking opportunity. See
      // InlineItemsBuilder::moveToNextBreakablePosition.
      if (&previousInlineTextItem.GetInlineTextBox() == &nextInlineTextItem.GetInlineTextBox())
      {
        if (previousInlineTextItem.BidiLevel() == nextInlineTextItem.BidiLevel())
        {
          return true;
        }
        // The bidi boundary may or may not be the reason for splitting the inline text box content.
        // FIXME: We could add a "reason flag" to InlineTextItem to tell why the split happened.
        auto &style = previousInlineTextItem.Style();
        auto lineBreakIteratorFactory = CachedLineBreakIteratorFactory {
          previousInlineTextItem.GetInlineTextBox().Content(), style.ComputedLocale().GetPlatform(),
          TextUtil::LineBreakIteratorMode(style.GetLineBreak()),
          TextUtil::GetContentAnalysis(style.GetWordBreak())};
        auto softWrapOpportunityCandidate = nextInlineTextItem.Start();
        return TextUtil::FindNextBreakablePosition(lineBreakIteratorFactory, softWrapOpportunityCandidate,
                                                   style)
               == softWrapOpportunityCandidate;
      }
      return TextUtil::MayBreakInBetween(previousInlineTextItem, nextInlineTextItem);
    }

    KRYS_NODISCARD inline const ElementBox &NearestCommonAncestor(const Box &first, const Box &second,
                                                                  const ElementBox &rootBox) noexcept
    {
      auto &firstParent = first.Parent();
      auto &secondParent = second.Parent();
      // Cover a few common cases first.
      // 'some content'
      if (&firstParent == &secondParent)
      {
        return firstParent;
      }
      // some<span>content</span>
      if (&secondParent != &rootBox && &secondParent.Parent() == &firstParent)
      {
        return firstParent;
      }
      // <span>some</span>content
      if (&firstParent != &rootBox && &firstParent.Parent() == &secondParent)
      {
        return secondParent;
      }
      // <span>some</span><span>content</span>
      if (&firstParent != &rootBox && &secondParent != &rootBox
          && &firstParent.Parent() == &secondParent.Parent())
      {
        return firstParent.Parent();
      }

      HashSet<const ElementBox *> descendantsSet;
      for (auto *descendant = &firstParent; descendant != &rootBox; descendant = &descendant->Parent())
      {
        descendantsSet.insert(descendant);
      }
      for (auto *descendant = &secondParent; descendant != &rootBox; descendant = &descendant->Parent())
      {
        if (!descendantsSet.insert(descendant).second)
        {
          return *descendant;
        }
      }
      return rootBox;
    }
  }

  InlineFormattingUtils::InlineFormattingUtils(
    const InlineFormattingContext &inlineFormattingContext) noexcept
      : _inlineFormattingContext(inlineFormattingContext)
  {
  }

  InlineLayoutUnit InlineFormattingUtils::LogicalTopForNextLine(
    const LineLayoutResult &lineLayoutResult, const InlineRect &lineLogicalRect,
    const FloatingContext &floatingContext, const BlockLayoutState::MarginState &marginState) const noexcept
  {
    auto didManageToPlaceInlineContentOrFloat = !lineLayoutResult.ItemRange.IsEmpty();
    if (didManageToPlaceInlineContentOrFloat)
    {
      auto logicalTopCandidateByContent = [&]
      {
        // Normally the next line's logical top is the previous line's logical bottom, but when the line ends
        // with the clear property set, the next line needs to clear the existing floats.
        if (!lineLayoutResult.HasContentfulInlineContent())
        {
          return lineLogicalRect.Bottom();
        }
        auto &lastRunLayoutBox = lineLayoutResult.Runs.back().LayoutBox();
        if (!lastRunLayoutBox.HasFloatClear() || lastRunLayoutBox.IsOutOfFlowPositioned())
        {
          return lineLogicalRect.Bottom();
        }
        auto blockAxisPositionWithClearance = floatingContext.GetBlockAxisPositionWithClearance(
          lastRunLayoutBox, GetFormattingContext().GeometryForBox(lastRunLayoutBox));
        if (!blockAxisPositionWithClearance)
        {
          return lineLogicalRect.Bottom();
        }
        return std::max(lineLogicalRect.Bottom(), blockAxisPositionWithClearance->Position.ToFloat());
      };
      return logicalTopCandidateByContent() + marginState.ContentOffsetAfterSelfCollapsingBlock.ToFloat();
    }

    auto intrusiveFloatBottom = [&]() -> Maybe<InlineLayoutUnit>
    {
      // Floats must have prevented us placing any content on the line.
      // Move next line below the intrusive float(s).
      krys_debug_assert(!lineLayoutResult.HasContentfulInlineContent()
                        || lineLayoutResult.Runs[0].IsLineSpanningInlineBoxStart());
      auto nextLineLogicalTop = [&]() -> LayoutUnit
      {
        if (auto nextLineLogicalTopCandidate = lineLayoutResult.HintForNextLineTopToAvoidIntrusiveFloat)
        {
          return LayoutUnit {*nextLineLogicalTopCandidate};
        }
        // We have to have a hint when intrusive floats prevented any inline content placement.
        krys_debug_assert(false);
        return LayoutUnit {lineLogicalRect.Top()
                           + GetFormattingContext().Root().Style().ComputedLineHeight()};
      };
      auto floatConstraints = floatingContext.GetConstraints(
        ToLayoutUnit(lineLogicalRect.Top()), nextLineLogicalTop(), FloatingContext::MayBeAboveLastFloat::Yes);
      if (floatConstraints.Start && floatConstraints.End)
      {
        // In case of left and right constraints, we need to pick the one that's closer to the current line.
        return std::min(floatConstraints.Start->Y, floatConstraints.End->Y).ToFloat();
      }
      if (floatConstraints.Start)
      {
        return floatConstraints.Start->Y.ToFloat();
      }
      if (floatConstraints.End)
      {
        return floatConstraints.End->Y.ToFloat();
      }
      // If we didn't manage to place a content on this vertical position due to intrusive floats, we have to
      // have at least one float here.
      krys_debug_assert(false);
      return {};
    };
    if (auto firstAvailableVerticalPosition = intrusiveFloatBottom();
        firstAvailableVerticalPosition && *firstAvailableVerticalPosition > lineLogicalRect.Top())
    {
      return *firstAvailableVerticalPosition;
    }
    // Do not get stuck on the same vertical position even when we find ourselves in this unexpected state.
    return std::ceil(std::nextafter(lineLogicalRect.Bottom(), std::numeric_limits<float>::max()));
  }

  bool
    InlineFormattingUtils::InlineLevelBoxAffectsLineBox(const InlineLevelBox &inlineLevelBox) const noexcept
  {
    if (!inlineLevelBox.MayStretchLineBox())
    {
      return false;
    }

    if (inlineLevelBox.IsLineBreakBox())
    {
      return false;
    }
    if (inlineLevelBox.IsListMarker())
    {
      // This does not match other browser engines. see webkit.org/b/256390.
      return true;
    }
    if (inlineLevelBox.IsInlineBox())
    {
      return GetFormattingContext().GetLayoutState().InStandardsMode()
               ? true
               : GetFormattingContext().Quirks().InlineBoxAffectsLineBox(inlineLevelBox);
    }
    if (inlineLevelBox.IsAtomicInlineBox())
    {
      return !inlineLevelBox.LayoutBox().IsRubyAnnotationBox();
    }
    return false;
  }

  InlineRect InlineFormattingUtils::FlipVisualRectToLogicalForWritingMode(const InlineRect &visualRect,
                                                                          WritingMode writingMode) noexcept
  {
    switch (writingMode.BlockDirection())
    {
      case FlowDirection::TopToBottom:
      case FlowDirection::BottomToTop: return visualRect;
      case FlowDirection::LeftToRight:
      case FlowDirection::RightToLeft:
        // FIXME: While vertical-lr and vertical-rl modes do differ in the ordering direction of line boxes
        // in a block container (see: https://drafts.csswg.org/css-writing-modes/#block-flow)
        // we ignore it for now as RenderBlock takes care of it for us.
        return InlineRect {visualRect.Left(), visualRect.Top(), visualRect.Height(), visualRect.Width()};
    }
    krys_debug_assert(false);
    return visualRect;
  }

  InlineLayoutUnit
    InlineFormattingUtils::ComputedTextIndent(IsIntrinsicWidthMode isIntrinsicWidthMode,
                                              IsFirstFormattedLine isFirstFormattedLine,
                                              Maybe<LineEndsWithLineBreak> previousLineEndsWithLineBreak,
                                              InlineLayoutUnit availableWidth) const noexcept
  {
    auto &root = GetFormattingContext().Root();

    // text-indent property specifies the indentation applied to lines of inline content in a block.
    // The indent is treated as a margin applied to the start edge of the line box.
    // The first formatted line of an element is always indented. For example, the first line of an anonymous
    // block box is only affected if it is the first child of its parent element. If 'each-line' is specified,
    // indentation also applies to all lines where the previous line ends with a hard break. [Integration]
    // root()->parent() would normally produce a valid layout box.
    auto shouldIndent = false;
    auto &textIndent = root.Style().GetTextIndent();
    if (textIndent.EachLine.has_value())
    {
      shouldIndent =
        isFirstFormattedLine == IsFirstFormattedLine::Yes
        || (previousLineEndsWithLineBreak && *previousLineEndsWithLineBreak == LineEndsWithLineBreak::Yes);
    }
    else if (root.IsAnonymousTextIndentCandidateForIntegration() || !root.IsAnonymous()
             || (!root.IsInlineIntegrationRoot() && root.Parent().FirstInFlowChild() == &root))
    {
      shouldIndent = isFirstFormattedLine == IsFirstFormattedLine::Yes;
    }

    // Specifying 'hanging' inverts whether the line should be indented or not.
    if (textIndent.Hanging.has_value())
    {
      shouldIndent = !shouldIndent;
    }

    if (!shouldIndent)
    {
      return {};
    }

    auto &textIndentLength = textIndent.Length;
    if (textIndentLength.IsKnownZero())
    {
      return {};
    }
    if (isIntrinsicWidthMode == IsIntrinsicWidthMode::Yes && textIndentLength.IsPercent())
    {
      // Percentages must be treated as 0 for the purpose of calculating intrinsic size contributions.
      // https://drafts.csswg.org/css-text/#text-indent-property
      return {};
    }
    return style::Evaluate<InlineLayoutUnit>(textIndentLength, availableWidth,
                                             root.Style().UsedZoomForLength());
  }

  InlineLayoutUnit InlineFormattingUtils::InitialLineHeight(bool isFirstLine) const noexcept
  {
    if (GetFormattingContext().GetLayoutState().InStandardsMode())
    {
      return isFirstLine ? GetFormattingContext().Root().FirstLineStyle().ComputedLineHeight()
                         : GetFormattingContext().Root().Style().ComputedLineHeight();
    }
    return GetFormattingContext().Quirks().InitialLineHeight();
  }

  FloatingContext::Constraints
    InlineFormattingUtils::FloatConstraintsForLine(InlineLayoutUnit lineLogicalTop,
                                                   InlineLayoutUnit contentLogicalHeight,
                                                   const FloatingContext &floatingContext) const noexcept
  {
    auto logicalTopCandidate = LayoutUnit {lineLogicalTop};
    auto logicalBottomCandidate = LayoutUnit {lineLogicalTop + contentLogicalHeight};
    if (logicalTopCandidate.MightBeSaturated() || logicalBottomCandidate.MightBeSaturated())
    {
      return {};
    }
    // Check for intruding floats and adjust logical left/available width for this line accordingly.
    return floatingContext.GetConstraints(logicalTopCandidate, logicalBottomCandidate,
                                          FloatingContext::MayBeAboveLastFloat::Yes);
  }

  InlineLayoutUnit InlineFormattingUtils::HorizontalAlignmentOffset(
    const RenderStyle &rootStyle, InlineLayoutUnit contentLogicalRight, InlineLayoutUnit lineLogicalWidth,
    InlineLayoutUnit hangingTrailingWidth, bool isLastLineOrLineEndsWithForcedLineBreak,
    Maybe<TextDirection> inlineBaseDirectionOverride) noexcept
  {
    // Depending on the line's alignment/justification, the hanging glyph can be placed outside the line box.
    if (hangingTrailingWidth)
    {
      // If white-space is set to pre-wrap, the UA must (unconditionally) hang this sequence, unless the
      // sequence is followed by a forced line break, in which case it must conditionally hang the sequence is
      // instead. Note that end of last line in a paragraph is considered a forced break.
      auto isConditionalHanging = isLastLineOrLineEndsWithForcedLineBreak;
      // In some cases, a glyph at the end of a line can conditionally hang: it hangs only if it does not
      // otherwise fit in the line prior to justification.
      if (isConditionalHanging)
      {
        // FIXME: Conditional hanging needs partial overflow trimming at glyph boundary, one by one until they
        // fit.
        contentLogicalRight = std::min(contentLogicalRight, lineLogicalWidth);
      }
      else
      {
        contentLogicalRight -= hangingTrailingWidth;
      }
    }

    auto horizontalAvailableSpace = lineLogicalWidth - contentLogicalRight;

    if (horizontalAvailableSpace <= 0)
    {
      return {};
    }

    auto isLeftToRightDirection =
      inlineBaseDirectionOverride.value_or(rootStyle.GetWritingMode().BidiDirection()) == TextDirection::LTR;

    auto computedHorizontalAlignment = [&]
    {
      auto textAlign = rootStyle.GetTextAlign();
      if (!isLastLineOrLineEndsWithForcedLineBreak)
      {
        return textAlign;
      }
      // The last line before a forced break or the end of the block is aligned according to text-align-last.
      switch (rootStyle.GetTextAlignLast())
      {
        case style::TextAlignLast::Auto:
          if (textAlign == style::TextAlign::Justify)
          {
            return style::TextAlign::Start;
          }
          return textAlign;
        case style::TextAlignLast::Start:   return style::TextAlign::Start;
        case style::TextAlignLast::End:     return style::TextAlign::End;
        case style::TextAlignLast::Left:    return style::TextAlign::Left;
        case style::TextAlignLast::Right:   return style::TextAlign::Right;
        case style::TextAlignLast::Center:  return style::TextAlign::Center;
        case style::TextAlignLast::Justify: return style::TextAlign::Justify;
        default:                            krys_debug_assert(false); return style::TextAlign::Start;
      }
    };

    switch (computedHorizontalAlignment())
    {
      case style::TextAlign::Left:
      case style::TextAlign::WebKitLeft:
        if (!isLeftToRightDirection)
        {
          return horizontalAvailableSpace;
        }
        [[fallthrough]];
      case style::TextAlign::Start: return {};
      case style::TextAlign::Right:
      case style::TextAlign::WebKitRight:
        if (!isLeftToRightDirection)
        {
          return {};
        }
        [[fallthrough]];
      case style::TextAlign::End:          return horizontalAvailableSpace;
      case style::TextAlign::Center:
      case style::TextAlign::WebKitCenter: return horizontalAvailableSpace / 2;
      case style::TextAlign::Justify:
        // Style::TextAlign::Justify is a run alignment (and we only do inline box alignment here)
        return {};
      default: krys_debug_assert(false); return {};
    }
  }

  InlineItemPosition InlineFormattingUtils::LeadingInlineItemPositionForNextLine(
    InlineItemPosition lineContentEnd, Maybe<InlineItemPosition> previousLineContentEnd,
    bool lineHasIntrusiveOrNewlyPlacedFloat, InlineItemPosition layoutRangeEnd) noexcept
  {
    if (!previousLineContentEnd)
    {
      return lineContentEnd;
    }
    if (previousLineContentEnd->Index < lineContentEnd.Index
        || (previousLineContentEnd->Index == lineContentEnd.Index
            && previousLineContentEnd->Offset < lineContentEnd.Offset))
    {
      // Either full or partial advancing.
      return lineContentEnd;
    }
    if (lineContentEnd == *previousLineContentEnd && lineHasIntrusiveOrNewlyPlacedFloat)
    {
      // Couldn't manage to put any content on line due to floats.
      return lineContentEnd;
    }
    if (lineContentEnd == layoutRangeEnd)
    {
      // End of content.
      return layoutRangeEnd;
    }
    // This looks like a partial content and we are stuck. Let's force-move over to the next inline item.
    // We certainly lose some content, but we would be busy looping otherwise.
    krys_debug_assert(false);
    return {std::min(lineContentEnd.Index + 1, layoutRangeEnd.Index), {}};
  }

  InlineLayoutUnit InlineFormattingUtils::InlineItemWidth(const InlineItem &inlineItem,
                                                          InlineLayoutUnit contentLogicalLeft,
                                                          bool useFirstLineStyle) const noexcept
  {
    krys_debug_assert(inlineItem.LayoutBox().IsInlineLevelBox() || inlineItem.IsBlock());
    if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
    {
      if (auto contentWidth = inlineTextItem->Width())
      {
        return *contentWidth;
      }
      auto &fontCascade = useFirstLineStyle ? inlineTextItem->FirstLineStyle().GetFontCascade()
                                            : inlineTextItem->Style().GetFontCascade();
      if (!inlineTextItem->IsWhitespace() || InlineTextItem::ShouldPreserveSpacesAndTabs(*inlineTextItem))
      {
        return TextUtil::Width(*inlineTextItem, fontCascade, contentLogicalLeft);
      }
      return TextUtil::Width(*inlineTextItem, fontCascade, inlineTextItem->Start(),
                             inlineTextItem->Start() + 1, contentLogicalLeft);
    }

    if (inlineItem.IsLineBreak() || inlineItem.IsWordBreakOpportunity())
    {
      return {};
    }

    auto &layoutBox = inlineItem.LayoutBox();
    auto &boxGeometry = GetFormattingContext().GeometryForBox(layoutBox);

    if (layoutBox.IsReplacedBox())
    {
      return boxGeometry.MarginBoxWidth().ToFloat();
    }

    if (inlineItem.IsInlineBoxStart())
    {
      return (boxGeometry.MarginStart() + boxGeometry.BorderStart() + boxGeometry.PaddingStart()).ToFloat();
    }

    if (inlineItem.IsInlineBoxEnd())
    {
      return (boxGeometry.MarginEnd() + boxGeometry.BorderEnd() + boxGeometry.PaddingEnd()).ToFloat();
    }

    if (inlineItem.IsOpaque())
    {
      return {};
    }

    if (inlineItem.IsBlock())
    {
      return boxGeometry.MarginBoxWidth().ToFloat();
    }

    // Non-replaced inline box (e.g. inline-block)
    return boxGeometry.MarginBoxWidth().ToFloat();
  }

  bool InlineFormattingUtils::IsAtSoftWrapOpportunity(const InlineItem &previous,
                                                      const InlineItem &next) const noexcept
  {
    // FIXME: Transition no-wrapping logic from InlineContentBreaker to here where we compute the soft wrap
    // opportunity indexes. "is at" simple means that there's a soft wrap opportunity right after the
    // [previous]. [text][ ][text][inline box start]... (<div>text content<span>..</div>) soft wrap indexes: 0
    // and 1 definitely, 2 depends on the content after the [inline box start].

    // https://www.w3.org/TR/css-text-4/#line-break-details
    // Figure out if the new incoming content puts the uncommitted content on a soft wrap opportunity.
    // e.g. [inline box start][prior_continuous_content][inline box end]
    // (<span>prior_continuous_content</span>) An incoming <img> box would enable us to commit the
    // "<span>prior_continuous_content</span>" content but an incoming text content would not necessarily.
    krys_debug_assert(previous.IsText() || previous.IsAtomicInlineBox()
                      || previous.LayoutBox().IsRubyInlineBox());
    krys_debug_assert(next.IsText() || next.IsAtomicInlineBox() || next.LayoutBox().IsRubyInlineBox());

    auto mayWrapPrevious = TextUtil::IsWrappingAllowed(previous.LayoutBox().Parent().Style());
    auto mayWrapNext = TextUtil::IsWrappingAllowed(next.LayoutBox().Parent().Style());
    if (&previous.LayoutBox().Parent() == &next.LayoutBox().Parent() && !mayWrapPrevious && !mayWrapNext)
    {
      return false;
    }

    if (Is<InlineTextItem>(previous) && Is<InlineTextItem>(next))
    {
      auto &previousInlineTextItem = Downcast<InlineTextItem>(previous);
      auto &nextInlineTextItem = Downcast<InlineTextItem>(next);
      if (previousInlineTextItem.IsWhitespace() || nextInlineTextItem.IsWhitespace())
      {
        // For soft wrap opportunities created by characters that disappear at the line break (e.g. U+0020
        // SPACE), properties on the box directly containing that character control the line breaking at that
        // opportunity.
        // "<nowrap> </nowrap>after"
        if (previousInlineTextItem.IsWhitespace())
        {
          return mayWrapPrevious;
        }

        // "<span>before</span><nowrap> </nowrap>"
        if (!mayWrapNext)
        {
          return false;
        }
        // 'white-space: break-spaces' and '-webkit-line-break: after-white-space': line breaking opportunity
        // exists after every preserved white space character, but not before.
        auto &style = nextInlineTextItem.Style();
        return style.GetWhiteSpaceCollapse() != WhiteSpaceCollapse::BreakSpaces
               && style.GetLineBreak() != LineBreak::AfterWhiteSpace;
      }
      if (previous.Style().GetLineBreak() == LineBreak::Anywhere
          || next.Style().GetLineBreak() == LineBreak::Anywhere)
      {
        // which elements’ line-break, word-break, and overflow-wrap properties control the determination of
        // soft wrap opportunities at such boundaries is undefined in this level. There is a soft wrap
        // opportunity around every typographic character unit, including around any punctuation character or
        // preserved white spaces, or in the middle of words.
        return true;
      }
      // Both previous and next items are non-whitespace text.
      // [text][text] : is a continuous content.
      // [text-][text] : after [hyphen] position is a soft wrap opportunity.
      auto previousAndNextHaveSameParent =
        &previousInlineTextItem.LayoutBox().Parent() == &nextInlineTextItem.LayoutBox().Parent();
      if (previousAndNextHaveSameParent && !TextUtil::IsWrappingAllowed(previousInlineTextItem.Style()))
      {
        return false;
      }
      // For soft wrap opportunities defined by the boundary between two characters, the white-space property
      // on the nearest common ancestor of the two characters controls breaking.
      if (!EndsWithSoftWrapOpportunity(previousInlineTextItem, nextInlineTextItem))
      {
        return false;
      }
      return TextUtil::IsWrappingAllowed(NearestCommonAncestor(previousInlineTextItem.LayoutBox(),
                                                               nextInlineTextItem.LayoutBox(),
                                                               GetFormattingContext().Root())
                                           .Style());
    }
    if (previous.LayoutBox().IsListMarkerBox())
    {
      auto &listMarkerBox = Downcast<ElementBox>(previous.LayoutBox());
      return !listMarkerBox.IsListMarkerOutside();
    }
    if (next.LayoutBox().IsListMarkerBox())
    {
      // FIXME: SHould this ever be the case?
      return true;
    }
    if (previous.IsAtomicInlineBox() || next.IsAtomicInlineBox())
    {
      // [text][inline box start][inline box end][inline box] (text<span></span><img>) : there's a soft wrap
      // opportunity between the [text] and [img]. The line breaking behavior of a replaced element or other
      // atomic inline is equivalent to an ideographic character.
      return true;
    }

    krys_debug_assert(false);
    return true;
  }

  size_t InlineFormattingUtils::NextWrapOpportunity(size_t startIndex, const InlineItemRange &layoutRange,
                                                    Span<const InlineItem> inlineItemList) const noexcept
  {
    // 1. Find the start candidate by skipping leading non-content items e.g "<span><span>start". Opportunity
    // is after "<span><span>".
    // 2. Find the end candidate by skipping non-content items inbetween e.g. "<span><span>start</span>end".
    // Opportunity is after "</span>".
    // 3. Check if there's a soft wrap opportunity between the 2 candidate inline items and repeat.
    // 4. Any force line break/explicit wrap content inbetween is considered as wrap opportunity.

    // [ex-][inline box start][inline box end][float][ample] (ex-<span></span><div
    // style="float:left"></div>ample). Wrap index is at [ex-]. [ex][inline box start][amp-][inline box
    // start][le] (ex<span>amp-<span>ample). Wrap index is at [amp-]. [ex-][inline box start][line
    // break][ample] (ex-<span><br>ample). Wrap index is after [br].
    auto previousInlineItemIndex = Maybe<size_t> {};
    for (auto index = startIndex; index < layoutRange.EndIndex(); ++index)
    {
      auto &currentItem = inlineItemList[index];
      if (currentItem.IsLineBreak() || currentItem.IsWordBreakOpportunity())
      {
        // We always stop at explicit wrapping opportunities e.g. <br>. However the wrap position may be at
        // later position. e.g. <span><span><br></span></span> <- wrap position is after the second </span>
        // but in case of <span><br><span></span></span> <- wrap position is right after <br>.
        for (++index; index < layoutRange.EndIndex() && inlineItemList[index].IsInlineBoxEnd(); ++index)
        {
        }
        return index;
      }
      auto isInlineBox = currentItem.IsInlineBoxStart() || currentItem.IsInlineBoxEnd();
      if (isInlineBox)
      {
        // Need to see what comes next to decide.
        continue;
      }
      if (currentItem.IsOpaque())
      {
        // This item is invisible to line breaking. Need to pretend it's not here.
        continue;
      }
      if (currentItem.IsBlock())
      {
        // Let's break before and after a block level box.
        auto wrappingPosition = index == startIndex ? std::min(index + 1, layoutRange.EndIndex()) : index;
        return wrappingPosition;
      }
      krys_debug_assert(currentItem.IsText() || currentItem.IsAtomicInlineBox() || currentItem.IsFloat()
                        || currentItem.LayoutBox().IsRubyInlineBox());
      if (currentItem.IsFloat())
      {
        // While floats are not part of the inline content and they are not supposed to introduce soft wrap
        // opportunities, e.g. [text][float box][float box][text][float box][text] is essentially just
        // [text][text][text] figuring out whether a float (or set of floats) should stay on the line or not
        // (and handle potentially out of order inline items) brings in unnecessary complexity. For now let's
        // always treat a float as a soft wrap opportunity.
        auto wrappingPosition = index == startIndex ? std::min(index + 1, layoutRange.EndIndex()) : index;
        return wrappingPosition;
      }
      if (!previousInlineItemIndex)
      {
        previousInlineItemIndex = index;
        continue;
      }
      // At this point previous and current items are not necessarily adjacent items e.g
      // "previous<span>current</span>"
      auto &previousItem = inlineItemList[*previousInlineItemIndex];
      if (IsAtSoftWrapOpportunity(previousItem, currentItem))
      {
        if (*previousInlineItemIndex + 1 == index && (!previousItem.IsText() || !currentItem.IsText()))
        {
          // We only know the exact soft wrap opportunity index when the previous and current items are next
          // to each other.
          return index;
        }
        // There's a soft wrap opportunity between 'previousInlineItemIndex' and 'index'.
        // Now forward-find from the start position to see where we can actually wrap.
        // [ex-][ample] vs. [ex-][inline box start][inline box end][ample]
        // where [ex-] is previousInlineItemIndex and [ample] is index.

        // inline content and their inline boxes form unbreakable content.
        // ex-<span></span>ample               : wrap opportunity is after "ex-<span></span>".
        // ex-<span>ample                      : wrap opportunity is after "ex-".
        // ex-<span><span></span></span>ample  : wrap opportunity is after "ex-<span><span></span></span>".
        // ex-</span></span>ample              : wrap opportunity is after "ex-</span></span>".
        // ex-</span><span>ample               : wrap opportunity is after "ex-</span>".
        // ex-<span><span>ample                : wrap opportunity is after "ex-".
        struct InlineBoxPosition
        {
          const Box *InlineBox {nullptr};
          size_t Index {0};
        };
        List<InlineBoxPosition> inlineBoxStack;
        auto start = *previousInlineItemIndex;
        auto end = index;
        // Soft wrap opportunity is at the first inline box that encloses the trailing content.
        for (auto candidateIndex = start + 1; candidateIndex < end; ++candidateIndex)
        {
          auto &inlineItem = inlineItemList[candidateIndex];
          krys_debug_assert(inlineItem.IsInlineBoxStartOrEnd() || inlineItem.IsOpaque());
          if (inlineItem.IsInlineBoxStart())
          {
            inlineBoxStack.push_back({&inlineItem.LayoutBox(), candidateIndex});
          }
          else if (inlineItem.IsInlineBoxEnd() && !inlineBoxStack.empty())
          {
            inlineBoxStack.pop_back();
          }
        }
        return inlineBoxStack.empty() ? index : inlineBoxStack.front().Index;
      }
      previousInlineItemIndex = index;
    }
    return layoutRange.EndIndex();
  }

  std::pair<InlineLayoutUnit, InlineLayoutUnit>
    InlineFormattingUtils::TextEmphasisForInlineBox(const Box &layoutBox, const ElementBox &rootBox) noexcept
  {
    // Generic, non-inline box inline-level content (e.g. replaced elements) can't have text-emphasis
    // annotations.
    krys_debug_assert(layoutBox.IsInlineBox() || &layoutBox == &rootBox);

    auto &style = layoutBox.Style();
    auto hasTextEmphasis = !style.GetTextEmphasisStyle().IsNone();
    if (!hasTextEmphasis)
    {
      return {};
    }
    auto emphasisPosition = style.GetTextEmphasisPosition();
    // Normally we resolve visual -> logical values at pre-layout time, but emphasis values are not part of
    // the general box geometry.
    auto hasAboveTextEmphasis = false;
    auto hasUnderTextEmphasis = false;
    if (style.GetWritingMode().IsVerticalTypographic())
    {
      hasAboveTextEmphasis = !emphasisPosition.Contains(style::TextEmphasisPositionValue::Left);
      hasUnderTextEmphasis = !hasAboveTextEmphasis;
    }
    else
    {
      hasAboveTextEmphasis = !emphasisPosition.Contains(style::TextEmphasisPositionValue::Under);
      hasUnderTextEmphasis = !hasAboveTextEmphasis;
    }

    if (!hasAboveTextEmphasis && !hasUnderTextEmphasis)
    {
      return {};
    }

    auto enclosingRubyBase = [&]() -> const ElementBox *
    {
      if (&layoutBox == &rootBox)
      {
        return nullptr;
      }
      for (auto *ancestor = &layoutBox.Parent(); ancestor != &rootBox; ancestor = &ancestor->Parent())
      {
        if (ancestor->IsRubyBase())
        {
          return ancestor;
        }
      }
      return nullptr;
    };
    if (auto *rubyBase = enclosingRubyBase();
        rubyBase && RubyFormattingContext::HasInterlinearAnnotation(*rubyBase))
    {
      auto annotationPosition = rubyBase->Style().GetRubyPosition();
      if ((hasAboveTextEmphasis && annotationPosition == RubyPosition::Over)
          || (hasUnderTextEmphasis && annotationPosition == RubyPosition::Under))
      {
        // FIXME: Check if annotation box has content.
        return {};
      }
    }
    auto annotationSize =
      style.GetFontCascade().FloatEmphasisMarkHeight(style.GetTextEmphasisStyle().MarkString());
    return {hasAboveTextEmphasis ? annotationSize : 0.f, hasAboveTextEmphasis ? 0.f : annotationSize};
  }

  LineEndingTruncationPolicy InlineFormattingUtils::GetLineEndingTruncationPolicy(
    const RenderStyle &rootStyle, size_t numberOfContentfulLines, Maybe<size_t> numberOfVisibleLinesAllowed,
    bool currentLineIsContentful) noexcept
  {
    if (numberOfVisibleLinesAllowed)
    {
      // text-overflow: ellipsis should not apply inside clamping content.
      if (!currentLineIsContentful)
      {
        // Content with no inline should never ever receive ellipsis.
        return LineEndingTruncationPolicy::NoTruncation;
      }
      return *numberOfVisibleLinesAllowed == numberOfContentfulLines
               ? LineEndingTruncationPolicy::WhenContentOverflowsInBlockDirection
               : LineEndingTruncationPolicy::NoTruncation;
    }

    // Truncation is in effect when the block container has overflow other than visible.
    if (rootStyle.OverflowX() != Overflow::Visible && rootStyle.GetTextOverflow() == TextOverflow::Ellipsis)
    {
      return LineEndingTruncationPolicy::WhenContentOverflowsInInlineDirection;
    }
    return LineEndingTruncationPolicy::NoTruncation;
  }

  Maybe<LineLayoutResult::InlineContentEnding>
    InlineFormattingUtils::InlineContentEnding(const Line::Result &lineContent) noexcept
  {
    if (!lineContent.Runs.empty() && lineContent.Runs.back().IsBlock())
    {
#if KRYS_DEBUG
      for (auto &run : lineContent.Runs)
      {
        krys_debug_assert(run.IsLineSpanningInlineBoxStart() || run.IsBlock());
      }
#endif
      return {};
    }

    for (auto &run : lineContent.Runs | std::views::reverse)
    {
      krys_debug_assert(!run.IsBlock());
      if (run.IsOpaque())
      {
        continue;
      }
      if (run.IsLineBreak())
      {
        return LineLayoutResult::InlineContentEnding::LineBreak;
      }
      if (auto &textContent = run.TextContent(); textContent && textContent->NeedsHyphen)
      {
        return LineLayoutResult::InlineContentEnding::Hyphen;
      }
      return LineLayoutResult::InlineContentEnding::Generic;
    }
    return {};
  }

  bool InlineFormattingUtils::ShouldDiscardRemainingContentInBlockDirection() const noexcept
  {
    auto &inlineLayoutState = GetFormattingContext().GetLayoutState();
    auto lineClamp = inlineLayoutState.ParentBlockLayoutState().GetLineClamp();
    if (!lineClamp || !lineClamp->ShouldDiscardOverflow)
    {
      return false;
    }
    krys_debug_assert(!lineClamp->IsLegacy);
    return lineClamp->MaximumLines == inlineLayoutState.LineCountWithInlineContentIncludingNestedBlocks();
  }

  // Subpixel inline layout (in the block direction) is off: vertical metrics are snapped to integers.

  InlineLayoutUnit InlineFormattingUtils::Ascent(const FontMetrics &fontMetrics, FontBaseline fontBaseline,
                                                 const InlineLevelBox &inlineLevelBox) noexcept
  {
    return Ascent(fontMetrics, fontBaseline, inlineLevelBox.LayoutBox());
  }

  InlineLayoutUnit InlineFormattingUtils::Descent(const FontMetrics &fontMetrics, FontBaseline fontBaseline,
                                                  const InlineLevelBox &inlineLevelBox) noexcept
  {
    return Descent(fontMetrics, fontBaseline, inlineLevelBox.LayoutBox());
  }

  InlineLayoutUnit InlineFormattingUtils::Ascent(const FontMetrics &fontMetrics, FontBaseline fontBaseline,
                                                 const Box &) noexcept
  {
    return static_cast<InlineLayoutUnit>(fontMetrics.IntAscent(fontBaseline));
  }

  InlineLayoutUnit InlineFormattingUtils::Descent(const FontMetrics &fontMetrics, FontBaseline fontBaseline,
                                                  const Box &) noexcept
  {
    return static_cast<InlineLayoutUnit>(fontMetrics.IntDescent(fontBaseline));
  }

  InlineLayoutUnit InlineFormattingUtils::SnapToInt(InlineLayoutUnit value,
                                                    const InlineLevelBox &inlineLevelBox,
                                                    SnapDirection direction) noexcept
  {
    return SnapToInt(value, inlineLevelBox.LayoutBox(), direction);
  }

  InlineLayoutUnit InlineFormattingUtils::SnapToInt(InlineLayoutUnit value, const Box &,
                                                    SnapDirection direction) noexcept
  {
    switch (direction)
    {
      case SnapDirection::Floor: return std::floor(value);
      case SnapDirection::Ceil:  return std::ceil(value);
      case SnapDirection::Round: return std::round(value);
    }
    krys_debug_assert(false);
    return {};
  }
}
