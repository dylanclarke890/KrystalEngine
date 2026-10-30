#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLine.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineSoftLineBreakItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <ranges>

namespace krys::boo::layout
{
  Line::Line(const InlineFormattingContext &inlineFormattingContext) noexcept
      : _inlineFormattingContext(inlineFormattingContext), _trimmableTrailingContent(_runs)
  {
  }

  void Line::Initialize(const SmallList<InlineItem, 1> &lineSpanningInlineBoxes,
                        bool isFirstFormattedLine) noexcept
  {
    _isFirstFormattedLine = isFirstFormattedLine;
    _inlineBoxListWithClonedDecorationEnd.clear();
    _rubyAlignContentRightOffset = {};
    _nonSpanningInlineLevelBoxCount = 0;
    _hasNonDefaultBidiLevelRun = false;
    _hasRubyContent = false;
    _contentLogicalWidth = {};
    _inlineBoxLogicalLeftStack.clear();
    _runs.clear();
    ResetTrailingContent();
    auto appendLineSpanningInlineBoxes = [&]
    {
      for (auto &inlineBoxStartItem : lineSpanningInlineBoxes)
      {
        if (inlineBoxStartItem.Style().GetBoxDecorationBreak() != BoxDecorationBreak::Clone)
        {
          _runs.push_back(Run {inlineBoxStartItem, LastRunLogicalRight(), {}});
        }
        else
        {
          // https://drafts.csswg.org/css-break/#break-decoration
          // clone: Each box fragment is independently wrapped with the border, padding, and margin.
          auto &inlineBoxGeometry = GetFormattingContext().GeometryForBox(inlineBoxStartItem.LayoutBox());
          auto marginBorderAndPaddingStart = inlineBoxGeometry.MarginBorderAndPaddingStart().ToFloat();
          auto runLogicalLeft = LastRunLogicalRight();
          _runs.push_back(Run {inlineBoxStartItem, runLogicalLeft, marginBorderAndPaddingStart});
          // Do not let negative margin make the content shorter than it already is.
          _contentLogicalWidth = std::max(_contentLogicalWidth, runLogicalLeft + marginBorderAndPaddingStart);
          _inlineBoxListWithClonedDecorationEnd.push_back(&inlineBoxStartItem.LayoutBox());
        }
      }
    };
    appendLineSpanningInlineBoxes();
  }

  void Line::ResetTrailingContent() noexcept
  {
    _trimmableTrailingContent.Reset();
    _hangingContent.ResetTrailingContent();
    _trailingSoftHyphenWidth = {};
  }

  Line::Result Line::Close() noexcept
  {
    auto trailingClonedDecorationWidth = [&]
    {
      auto decorationWidth = InlineLayoutUnit {};
      for (auto *inlineBox : _inlineBoxListWithClonedDecorationEnd)
      {
        auto &boxGeometry = GetFormattingContext().GeometryForBox(*inlineBox);
        decorationWidth += (boxGeometry.BorderEnd() + boxGeometry.PaddingEnd()).ToFloat();
      }
      return decorationWidth;
    }();

    auto contentLogicalRight = ContentLogicalRight() + _rubyAlignContentRightOffset;
    auto isContentful = LineHasVisuallyNonEmptyContent();
    return {krys::move(_runs),
            ContentLogicalWidth() + trailingClonedDecorationWidth,
            contentLogicalRight + trailingClonedDecorationWidth,
            isContentful,
            _hangingContent.TrailingWhitespaceLength() != 0,
            _hangingContent.TrailingWidth(),
            _hangingContent.LeadingPunctuationWidth(),
            _hasNonDefaultBidiLevelRun,
            _nonSpanningInlineLevelBoxCount};
  }

  InlineLayoutUnit
    Line::HandleTrailingTrimmableContent(TrailingContentAction trailingTrimmableContentAction) noexcept
  {
    if (_trimmableTrailingContent.IsEmpty() || _runs.empty())
    {
      return {};
    }

    if (trailingTrimmableContentAction == TrailingContentAction::Preserve)
    {
      _trimmableTrailingContent.Reset();
      return {};
    }

    auto trimmedWidth = _trimmableTrailingContent.Remove();
    _contentLogicalWidth -= trimmedWidth;
    return trimmedWidth;
  }

  void Line::HandleOverflowingNonBreakingSpace(TrailingContentAction trailingContentAction,
                                               InlineLayoutUnit overflowingWidth) noexcept
  {
    krys_debug_assert(_trimmableTrailingContent.IsEmpty());
    krys_debug_assert(overflowingWidth > 0);

    auto startIndex = [&]() -> size_t
    {
      auto trimmedContentWidth = InlineLayoutUnit {};
      for (auto index = _runs.size(); index--;)
      {
        auto &run = _runs[index];
        if (run.IsLineBreak() || run.IsInlineBox() || run.IsWordBreakOpportunity() || run.IsWordSeparator())
        {
          // It's ok to trim/remove non-breaking spaces across inline boxes.
          continue;
        }
        if (!run.IsNonBreakingSpace())
        {
          // Only trim/remove trailing non-breaking space.
          return index;
        }
        trimmedContentWidth += run.LogicalWidth();
        if (trimmedContentWidth >= overflowingWidth)
        {
          // This is how much non-breakable space needs to be removed or trimmed.
          return index;
        }
      }
      return 0;
    };

    auto index = startIndex();
    auto removedOrCollapsedContentWidth = InlineLayoutUnit {};
    while (index < _runs.size())
    {
      auto &run = _runs[index];

      if (!run.IsNonBreakingSpace())
      {
        run.MoveHorizontally(-removedOrCollapsedContentWidth);
        ++index;
        continue;
      }
      if (trailingContentAction == TrailingContentAction::Preserve)
      {
        run.MoveHorizontally(-removedOrCollapsedContentWidth);
        removedOrCollapsedContentWidth += run.LogicalWidth();
        run.ShrinkHorizontally(run.LogicalWidth());
        ++index;
        continue;
      }

      removedOrCollapsedContentWidth += run.LogicalWidth();
      _runs.erase(_runs.begin() + static_cast<ptrdiff_t>(index));
    }
    _contentLogicalWidth -= removedOrCollapsedContentWidth;
  }

  const Box *Line::RemoveOverflowingOutOfFlowContent() noexcept
  {
    auto lastTrailingOutOfFlowItemIndex = Maybe<size_t> {};
    for (size_t index = _runs.size(); index--;)
    {
      auto &run = _runs[index];
      if (run.IsOpaque() && run.LayoutBox().IsOutOfFlowPositioned())
      {
        lastTrailingOutOfFlowItemIndex = index;
        continue;
      }
      if (!run.LogicalWidth() && (run.IsInlineBoxStart() || run.IsInlineBoxEnd()))
      {
        continue;
      }
      break;
    }
    if (!lastTrailingOutOfFlowItemIndex)
    {
      return {};
    }
    auto *lastTrailingOpaqueBox = &_runs[*lastTrailingOutOfFlowItemIndex].LayoutBox();
    _runs.erase(_runs.begin() + static_cast<ptrdiff_t>(*lastTrailingOutOfFlowItemIndex), _runs.end());
    krys_debug_assert(!_runs.empty());
    return lastTrailingOpaqueBox;
  }

  void Line::HandleTrailingHangingContent(Maybe<IntrinsicWidthMode> intrinsicWidthMode,
                                          InlineLayoutUnit horizontalAvailableSpaceForContent,
                                          bool isLastFormattedLine) noexcept
  {
    // https://drafts.csswg.org/css-text/#hanging
    if (!_hangingContent.TrailingWidth())
    {
      return;
    }

    if (_hangingContent.IsTrailingContentPunctuation() && !isLastFormattedLine)
    {
      _hangingContent.ResetTrailingContent();
    }

    auto hangingTrailingContentIsConditional = [&]
    {
      auto lineEndsWithForcedLineBreak =
        isLastFormattedLine || (!Runs().empty() && Runs().back().IsLineBreak());
      return _hangingContent.IsTrailingContentConditional()
             || (_hangingContent.IsTrailingContentConditionalWhenFollowedByForcedLineBreak()
                 && lineEndsWithForcedLineBreak);
    }();

    if (!intrinsicWidthMode)
    {
      // Make sure when the conditionally hanging content actually fits we don't hang the content anymore
      // during normal layout.
      if (hangingTrailingContentIsConditional)
      {
        // In some cases, a glyph at the end of a line can conditionally hang: it hangs only if it does not
        // otherwise fit in the line prior to justification.
        auto doesTrailingHangingContentFit = _contentLogicalWidth <= horizontalAvailableSpaceForContent;
        if (doesTrailingHangingContentFit)
        {
          // Reset here means the trailing content is not hanging anymore -i.e. part of the line content.
          _hangingContent.ResetTrailingContent();
        }
      }
      return;
    }

    // 1. The hanging glyph is not taken into account when computing intrinsic sizes (min-content size and
    // max-content size)
    // 2. Glyphs that conditionally hang are not taken into account when computing min-content sizes, but they
    // are taken into account for max-content sizes.
    if (*intrinsicWidthMode == IntrinsicWidthMode::Minimum || !hangingTrailingContentIsConditional)
    {
      krys_debug_assert(_trimmableTrailingContent.IsEmpty());
      _contentLogicalWidth -= _hangingContent.TrailingWidth();
    }
  }

  void Line::ResetBidiLevelForTrailingWhitespace(UBiDiLevel rootBidiLevel) noexcept
  {
    krys_debug_assert(_trimmableTrailingContent.IsEmpty());
    if (!_hasNonDefaultBidiLevelRun)
    {
      return;
    }
    // UAX#9 L1: trailing whitespace should use paragraph direction.
    // see https://unicode.org/reports/tr9/#L1
    auto trailingNonWhitespaceOnlyRunIndex = Maybe<size_t> {};
    for (auto index = _runs.size(); index--;)
    {
      auto &run = _runs[index];
      if (run.IsAtomicInlineBox() || run.IsLineBreak() || (run.IsText() && !run.HasTrailingWhitespace()))
      {
        break;
      }

      if (!run.HasTrailingWhitespace())
      {
        // Skip non-content type of runs e.g. <span>
        continue;
      }
      // No need to adjust the bidi level unless the directionality is different.
      // e.g. rtl root dir with trailing whitespace attached to an rtl run.
      auto sameInlineDirection = run.BidiLevel() % 2 == rootBidiLevel % 2;
      if (!run.IsWhitespaceOnly())
      {
        trailingNonWhitespaceOnlyRunIndex = !sameInlineDirection ? Maybe<size_t> {index} : null;
        // There can't be any trailing whitespace in front of this non-whitespace/whitespace content.
        break;
      }
      // Whitespace only runs just need simple bidi level reset.
      if (!sameInlineDirection)
      {
        run.SetBidiLevel(rootBidiLevel);
      }
    }

    auto detachTrailingWhitespaceIfNeeded = [&]
    {
      if (!trailingNonWhitespaceOnlyRunIndex)
      {
        return;
      }
      auto runIndex = *trailingNonWhitespaceOnlyRunIndex;
      auto &run = _runs[runIndex];
      auto detachedTrailingRun = *run.DetachTrailingWhitespace();
      detachedTrailingRun.SetBidiLevel(rootBidiLevel);
      if (runIndex == _runs.size() - 1)
      {
        _runs.push_back(detachedTrailingRun);
        return;
      }
      _runs.insert(_runs.begin() + static_cast<ptrdiff_t>(runIndex + 1), detachedTrailingRun);
    };
    detachTrailingWhitespaceIfNeeded();
  }

  void Line::AppendInlineBoxStart(const InlineItem &inlineItem, const RenderStyle &style,
                                  InlineLayoutUnit logicalWidth,
                                  InlineLayoutUnit textSpacingAdjustment) noexcept
  {
    auto &inlineBoxGeometry = GetFormattingContext().GeometryForBox(inlineItem.LayoutBox());
    if (inlineBoxGeometry.MarginBorderAndPaddingStart())
    {
      _hangingContent.ResetTrailingContent();
    }
    // This is really just a placeholder to mark the start of the inline box <span>.
    ++_nonSpanningInlineLevelBoxCount;
    auto logicalLeft = LastRunLogicalRight();
    // Do not let negative margin make the content shorter than it already is.
    _contentLogicalWidth = std::max(_contentLogicalWidth, logicalLeft + logicalWidth);

    auto marginStart = inlineBoxGeometry.MarginStart().ToFloat();
    if (marginStart < 0)
    {
      // Negative margin-start pulls the content to the logical left direction.
      logicalLeft += marginStart;
      logicalWidth -= marginStart;
    }

    auto mayPullNonInlineBoxContentToLogicalLeft = style.UsedLetterSpacing() < 0;
    if (mayPullNonInlineBoxContentToLogicalLeft)
    {
      _inlineBoxLogicalLeftStack.push_back(logicalLeft);
    }

    _hasRubyContent = _hasRubyContent || inlineItem.LayoutBox().IsRubyBase();
    _runs.push_back(Run {inlineItem, style, logicalLeft, logicalWidth, textSpacingAdjustment});
    if (style.GetBoxDecorationBreak() == BoxDecorationBreak::Clone)
    {
      _inlineBoxListWithClonedDecorationEnd.push_back(&inlineItem.LayoutBox());
    }
  }

  void Line::AppendInlineBoxEnd(const InlineItem &inlineItem, const RenderStyle &style,
                                InlineLayoutUnit logicalWidth) noexcept
  {
    if (GetFormattingContext().GeometryForBox(inlineItem.LayoutBox()).MarginBorderAndPaddingEnd())
    {
      _hangingContent.ResetTrailingContent();
    }
    // This is really just a placeholder to mark the end of the inline box </span>.
    auto removeTrailingLetterSpacing = [&]
    {
      if (!_trimmableTrailingContent.IsTrailingRunPartiallyTrimmable())
      {
        return;
      }
      _contentLogicalWidth -= _trimmableTrailingContent.RemovePartiallyTrimmableContent();
    };
    // Prevent trailing letter-spacing from spilling out of the inline box.
    // https://drafts.csswg.org/css-text-3/#letter-spacing-property See example 21.
    removeTrailingLetterSpacing();
    auto logicalLeft = LastRunLogicalRight();
    auto mayPullNonInlineBoxContentToLogicalLeft = style.UsedLetterSpacing() < 0;
    if (mayPullNonInlineBoxContentToLogicalLeft)
    {
      // Do not let negative spacing pull content to the left of the inline box logical left.
      // e.g. <span style="border-left: solid red; letter-spacing: -200px;">content</span>This should not be
      // to the left of the red border)
      auto inlineBoxLogicalLeft = 0.f;
      if (!_inlineBoxLogicalLeftStack.empty())
      {
        inlineBoxLogicalLeft = _inlineBoxLogicalLeftStack.back();
        _inlineBoxLogicalLeftStack.pop_back();
      }
      logicalLeft = std::max(logicalLeft, inlineBoxLogicalLeft);
    }
    _runs.push_back(Run {inlineItem, style, logicalLeft, logicalWidth});
    // Do not let negative margin make the content shorter than it already is.
    _contentLogicalWidth = std::max(_contentLogicalWidth, logicalLeft + logicalWidth);
    if (style.GetBoxDecorationBreak() == BoxDecorationBreak::Clone)
    {
      krys_debug_assert(&inlineItem.LayoutBox() == _inlineBoxListWithClonedDecorationEnd.back());
      _inlineBoxListWithClonedDecorationEnd.pop_back();
    }
  }

  void Line::AppendText(const InlineTextItem &inlineTextItem, const RenderStyle &style,
                        InlineLayoutUnit logicalWidth, Maybe<Line::ShapingBoundary> shapingBoundary) noexcept
  {
    auto willCollapseCompletely = [&]
    {
      if (inlineTextItem.IsEmpty())
      {
        // Some generated content initiates empty text items. They are truly collapsible.
        return true;
      }
      if (!inlineTextItem.IsWhitespace())
      {
        return false;
      }
      if (InlineTextItem::ShouldPreserveSpacesAndTabs(inlineTextItem))
      {
        return false;
      }
      // This content is collapsible. Let's check if the last item is collapsed.
      for (auto &run : _runs | std::views::reverse)
      {
        if (run.IsAtomicInlineBox())
        {
          return false;
        }
        // https://drafts.csswg.org/css-text-3/#white-space-phase-1
        // Any collapsible space immediately following another collapsible space—even one outside the boundary
        // of the inline containing that space, provided both spaces are within the same inline formatting
        // context—is collapsed to have zero advance width.
        if (run.IsText())
        {
          return run.HasCollapsibleTrailingWhitespace();
        }
        krys_debug_assert(run.IsListMarker() || run.IsLineSpanningInlineBoxStart() || run.IsInlineBoxStart()
                          || run.IsInlineBoxEnd() || run.IsWordBreakOpportunity() || run.IsOpaque());
      }
      // Leading whitespace.
      return true;
    };

    if (willCollapseCompletely())
    {
      return;
    }

    auto needsNewRun = [&]
    {
      if (_runs.empty())
      {
        return true;
      }
      auto &lastRun = _runs.back();
      if (&lastRun.LayoutBox() != &inlineTextItem.LayoutBox())
      {
        return true;
      }
      if (lastRun.BidiLevel() != inlineTextItem.BidiLevel())
      {
        return true;
      }
      if (!lastRun.IsText())
      {
        return true;
      }
      if (lastRun.HasCollapsedTrailingWhitespace())
      {
        return true;
      }
      if (style.GetFontCascade().WordSpacing()
          && (inlineTextItem.IsWordSeparator()
              || (lastRun.IsWordSeparator() && lastRun.BidiLevel() != UBIDI_DEFAULT_LTR)))
      {
        return true;
      }
      if (inlineTextItem.IsZeroWidthSpaceSeparator())
      {
        return true;
      }
      if (inlineTextItem.IsQuirkNonBreakingSpace() || lastRun.IsNonBreakingSpace())
      {
        return true;
      }
      if (!inlineTextItem.Style().GetWritingMode().IsBidiLTR()
          && TextUtil::ShouldPreserveSpacesAndTabs(inlineTextItem.LayoutBox())
          && inlineTextItem.IsWhitespace() != lastRun.IsWhitespaceOnly())
      {
        // Whitespace content with position dependent width (e.g. tab character) does not work well when
        // included with non-whitespace content. We may end up with mismatching computed widths and misplaced
        // glyphs at paint time -unless we keep dedicated runs with explicit positions.
        return true;
      }
      if (shapingBoundary || lastRun.IsShapingBoundary())
      {
        return true;
      }
      return false;
    }();
    auto oldContentLogicalWidth = ContentLogicalWidth();
    auto runHasHangablePunctuationStart = IsFirstFormattedLine()
                                          && TextUtil::HasHangablePunctuationStart(inlineTextItem, style)
                                          && !LineHasVisuallyNonEmptyContent();
    auto contentLogicalRight = InlineLayoutUnit {};
    if (needsNewRun)
    {
      // Note, negative word spacing may cause glyph overlap.
      auto runLogicalLeft = [&]
      {
        if (runHasHangablePunctuationStart)
        {
          return -TextUtil::HangablePunctuationStartWidth(inlineTextItem, style);
        }
        return LastRunLogicalRight()
               + (inlineTextItem.IsWordSeparator() ? style.GetFontCascade().WordSpacing() : 0.0f);
      }();
      _runs.push_back(Run {inlineTextItem, style, runLogicalLeft, logicalWidth, {}, shapingBoundary});
      // Note that the _content_ logical right may be larger than the _run_ logical right.
      contentLogicalRight = runLogicalLeft + logicalWidth;
    }
    else
    {
      auto &lastRun = _runs.back();
      krys_debug_assert(lastRun.IsText());
      if (style.UsedLetterSpacing() >= 0)
      {
        lastRun.Expand(inlineTextItem, logicalWidth);
        contentLogicalRight = lastRun.LogicalRight();
      }
      else
      {
        auto contentWidthWithoutLastTextRun = [&]
        {
          // FIXME: We may need to traverse all the way to the previous non-text run (or even across inline
          // boxes).
          if (style.GetFontCascade().WordSpacing() >= 0)
          {
            return _contentLogicalWidth - std::max(0.f, lastRun.LogicalWidth());
          }
          // FIXME: Let's see if we need to optimize for this is the rare case of both letter and word spacing
          // being negative.
          auto rightMostPosition = InlineLayoutUnit {};
          for (auto &run : _runs | std::views::reverse)
          {
            rightMostPosition = std::max(rightMostPosition, run.LogicalRight());
          }
          return std::max(0.f, rightMostPosition);
        }();
        auto lastRunLogicalRight = lastRun.LogicalRight();
        lastRun.Expand(inlineTextItem, logicalWidth);
        // Negative letter spacing should only shorten the content to the boundary of the previous run.
        contentLogicalRight = std::max(contentWidthWithoutLastTextRun, lastRunLogicalRight + logicalWidth);
      }
    }
    // Ensure that property values that act like negative margin are not making the line wider.
    _contentLogicalWidth = std::max(oldContentLogicalWidth, contentLogicalRight);

    auto lastRunIndex = _runs.size() - 1;
    _trailingSoftHyphenWidth = {};
    // Handle trailing content, specifically whitespace and letter spacing.
    auto updateTrimmableStatus = [&]
    {
      if (inlineTextItem.IsFullyTrimmable())
      {
        auto trimmableWidth = logicalWidth;
        auto trimmableContentOffset = (ContentLogicalWidth() - oldContentLogicalWidth) - trimmableWidth;
        _trimmableTrailingContent.AddFullyTrimmableContent(lastRunIndex, trimmableContentOffset,
                                                           trimmableWidth);
        return true;
      }
      _trimmableTrailingContent.Reset();
      return false;
    };
    auto isTrimmable = updateTrimmableStatus();

    auto updateHangingStatus = [&]
    {
      if (runHasHangablePunctuationStart)
      {
        _hangingContent.SetLeadingPunctuation(TextUtil::HangablePunctuationStartWidth(inlineTextItem, style));
      }

      auto runHasHangableWhitespaceEnd =
        !isTrimmable && inlineTextItem.IsWhitespace()
        && TextUtil::ShouldTrailingWhitespaceHang(_runs[lastRunIndex].Style());
      if (runHasHangableWhitespaceEnd)
      {
        _hangingContent.SetTrailingWhitespace(inlineTextItem.Length(), logicalWidth);
        return;
      }
      if (TextUtil::HasHangablePunctuationEnd(inlineTextItem, style))
      {
        _hangingContent.SetTrailingPunctuation(TextUtil::HangablePunctuationEndWidth(inlineTextItem, style));
        return;
      }
      if (TextUtil::HasHangableStopOrCommaEnd(inlineTextItem, style))
      {
        auto isConditionalHanging =
          style.GetHangingPunctuation().Contains(style::HangingPunctuationValue::AllowEnd);
        _hangingContent.SetTrailingStopOrComma(TextUtil::HangableStopOrCommaEndWidth(inlineTextItem, style),
                                               isConditionalHanging);
        return;
      }
      _hangingContent.ResetTrailingContent();
    };
    updateHangingStatus();

    if (inlineTextItem.HasTrailingSoftHyphen())
    {
      _trailingSoftHyphenWidth = TextUtil::HyphenWidth(style);
    }
  }

  void Line::AppendTextFast(const InlineTextItem &inlineTextItem, const RenderStyle &style,
                            InlineLayoutUnit logicalWidth) noexcept
  {
    auto lineHasContent = !_runs.empty();
    auto willCollapseCompletely = [&]
    {
      if (inlineTextItem.IsEmpty())
      {
        return true;
      }
      if (!inlineTextItem.IsWhitespace() || InlineTextItem::ShouldPreserveSpacesAndTabs(inlineTextItem))
      {
        return false;
      }
      return !lineHasContent || _runs.back().HasCollapsibleTrailingWhitespace();
    };

    if (willCollapseCompletely())
    {
      return;
    }

    auto needsNewRun = [&]
    {
      if (!lineHasContent)
      {
        return true;
      }
      auto &lastRun = _runs.back();
      if (lastRun.HasCollapsedTrailingWhitespace())
      {
        return true;
      }
      if (&lastRun.LayoutBox() != &inlineTextItem.LayoutBox())
      {
        return true;
      }
      if (inlineTextItem.IsZeroWidthSpaceSeparator())
      {
        return true;
      }
      if (inlineTextItem.IsQuirkNonBreakingSpace() || lastRun.IsNonBreakingSpace())
      {
        return true;
      }
      return false;
    }();
    auto oldContentLogicalWidth = ContentLogicalWidth();
    if (needsNewRun)
    {
      auto runLogicalLeft = LastRunLogicalRight();
      _runs.push_back(Run {inlineTextItem, style, runLogicalLeft, logicalWidth});
      _contentLogicalWidth = runLogicalLeft + logicalWidth;
    }
    else
    {
      auto &lastRun = _runs.back();
      krys_debug_assert(lastRun.IsText());
      if (style.UsedLetterSpacing() >= 0)
      {
        lastRun.Expand(inlineTextItem, logicalWidth);
        _contentLogicalWidth = lastRun.LogicalRight();
      }
      else
      {
        auto contentWidthWithoutLastTextRun = _contentLogicalWidth - std::max(0.f, lastRun.LogicalWidth());
        auto lastRunLogicalRight = lastRun.LogicalRight();
        lastRun.Expand(inlineTextItem, logicalWidth);
        // Negative letter spacing should only shorten the content to the boundary of the previous run.
        _contentLogicalWidth = std::max(contentWidthWithoutLastTextRun, lastRunLogicalRight + logicalWidth);
      }
    }

    _trailingSoftHyphenWidth = {};
    auto updateTrimmableStatus = [&]
    {
      if (inlineTextItem.IsFullyTrimmable())
      {
        auto trimmableWidth = logicalWidth;
        auto trimmableContentOffset = (ContentLogicalWidth() - oldContentLogicalWidth) - trimmableWidth;
        _trimmableTrailingContent.AddFullyTrimmableContent(_runs.size() - 1, trimmableContentOffset,
                                                           trimmableWidth);
        return true;
      }
      _trimmableTrailingContent.Reset();
      return false;
    };
    auto isTrimmable = updateTrimmableStatus();

    auto updateHangingStatus = [&]
    {
      auto runHasHangableWhitespaceEnd =
        !isTrimmable && inlineTextItem.IsWhitespace() && TextUtil::ShouldTrailingWhitespaceHang(style);
      if (runHasHangableWhitespaceEnd)
      {
        _hangingContent.SetTrailingWhitespace(inlineTextItem.Length(), logicalWidth);
        return;
      }
      _hangingContent.ResetTrailingContent();
    };
    updateHangingStatus();

    if (inlineTextItem.HasTrailingSoftHyphen())
    {
      _trailingSoftHyphenWidth = TextUtil::HyphenWidth(style);
    }
  }

  void Line::AppendAtomicInlineBox(const InlineItem &inlineItem, const RenderStyle &style,
                                   InlineLayoutUnit marginBoxLogicalWidth) noexcept
  {
    ResetTrailingContent();
    // Do not let negative margin make the content shorter than it already is.
    _contentLogicalWidth = std::max(_contentLogicalWidth, LastRunLogicalRight() + marginBoxLogicalWidth);
    ++_nonSpanningInlineLevelBoxCount;
    auto marginStart = GetFormattingContext().GeometryForBox(inlineItem.LayoutBox()).MarginStart().ToFloat();
    if (marginStart >= 0)
    {
      _runs.push_back(Run {inlineItem, style, LastRunLogicalRight(), marginBoxLogicalWidth});
      return;
    }
    // Negative margin-start pulls the content to the logical left direction.
    // Negative margin also squeezes the margin box, we need to stretch it to make sure the subsequent content
    // won't overlap. e.g. <img style="width: 100px; margin-left: -100px;"> pulls the replaced box to -100px
    // with the margin box width of 0px. Instead we need to position it at -100px and size it to 100px so the
    // subsequent content starts at 0px.
    _runs.push_back(
      Run {inlineItem, style, LastRunLogicalRight() + marginStart, marginBoxLogicalWidth - marginStart});
  }

  void Line::AppendBlock(const InlineItem &blockItem, InlineLayoutUnit marginBoxLogicalWidth) noexcept
  {
#if KRYS_DEBUG
    // We may have added line spanning inline boxes when initializing this line for the block content.
    for (auto &run : _runs)
    {
      krys_debug_assert(run.IsLineSpanningInlineBoxStart());
    }
#endif
    // Cloned decoration should not show up on block lines. Make the line hug the block level element.
    _contentLogicalWidth = marginBoxLogicalWidth;
    _runs.push_back(Run {blockItem, blockItem.Style(), {}, marginBoxLogicalWidth});
  }

  void Line::AppendLineBreak(const InlineItem &inlineItem, const RenderStyle &style) noexcept
  {
    _trailingSoftHyphenWidth = {};
    if (inlineItem.IsHardLineBreak())
    {
      ++_nonSpanningInlineLevelBoxCount;
      _runs.push_back(Run {inlineItem, style, LastRunLogicalRight()});
      return;
    }
    // Soft line breaks (preserved new line characters) require inline text boxes for compatibility reasons.
    _runs.push_back(
      Run {Downcast<InlineSoftLineBreakItem>(inlineItem), inlineItem.Style(), LastRunLogicalRight()});
  }

  void Line::AppendWordBreakOpportunity(const InlineItem &inlineItem, const RenderStyle &style) noexcept
  {
    _runs.push_back(Run {inlineItem, style, LastRunLogicalRight()});
  }

  void Line::AppendOpaqueBox(const InlineItem &inlineItem, const RenderStyle &style) noexcept
  {
    _runs.push_back(Run {inlineItem, style, LastRunLogicalRight()});
  }

  void Line::AddTrailingHyphen(InlineLayoutUnit hyphenLogicalWidth) noexcept
  {
    for (auto &run : _runs | std::views::reverse)
    {
      if (!run.IsText())
      {
        continue;
      }
      run.SetNeedsHyphen(hyphenLogicalWidth);
      _contentLogicalWidth += hyphenLogicalWidth;
      return;
    }
    krys_debug_assert(false);
  }

  bool Line::HasContentOrListMarker() const noexcept
  {
    if (_runs.empty())
    {
      return false;
    }
    if (_runs.front().IsListMarkerInside())
    {
      return true;
    }
    return Line::HasContent();
  }

  bool Line::HasContent() const noexcept
  {
    for (auto &run : _runs | std::views::reverse)
    {
      if (run.IsContentful() && !run.IsGenerated())
      {
        return true;
      }
    }
    return false;
  }

  bool Line::LineHasVisuallyNonEmptyContent() const noexcept
  {
    auto &formattingContext = GetFormattingContext();
    for (auto &run : _runs | std::views::reverse)
    {
      if (Line::Run::IsContentfulOrHasDecoration(run, formattingContext))
      {
        return true;
      }
    }
    return false;
  }

  bool Line::AppendTrailingInlineItemAsTrailingRun(RunList &runs,
                                                   InlineLayoutUnit trimmedTrailingWhitespaceWidth,
                                                   InlineItemRange inlineItemRange,
                                                   const InlineItemList &inlineItems) noexcept
  {
    auto candidateInlineItemIndex = [&]() -> Maybe<size_t>
    {
      if (inlineItemRange.EndIndex() > inlineItems.size())
      {
        return {};
      }
      auto &lastRunLayoutBox = runs.back().LayoutBox();
      for (auto index = inlineItemRange.EndIndex(); index--;)
      {
        if (&inlineItems[index].LayoutBox() == &lastRunLayoutBox)
        {
          return ++index;
        }
      }
      return {};
    };

    auto trailingInlineItemIndex = candidateInlineItemIndex();
    if (!trailingInlineItemIndex || *trailingInlineItemIndex >= inlineItems.size())
    {
      krys_debug_assert(false);
      return false;
    }

    auto *trailingInlineTextItem = DynamicDowncast<InlineTextItem>(inlineItems[*trailingInlineItemIndex]);
    if (!trailingInlineTextItem || !trailingInlineTextItem->IsFullyTrimmable())
    {
      krys_debug_assert(false);
      return false;
    }

    runs.push_back(Run {*trailingInlineTextItem, trailingInlineTextItem->Style(), runs.back().LogicalRight(),
                        trimmedTrailingWhitespaceWidth});
    return true;
  }

  bool Line::RestoreTrimmedTrailingWhitespace(InlineLayoutUnit trimmedTrailingWhitespaceWidth, RunList &runs,
                                              InlineItemRange inlineItemRange,
                                              const InlineItemList &inlineItems) noexcept
  {
    auto restore = [&](Run &trailingRun)
    {
      krys_debug_assert(trailingRun.IsText());
      auto &layoutBox = Downcast<InlineTextBox>(trailingRun.LayoutBox());
      if (trailingRun._textContent->Start + trailingRun._textContent->Length == layoutBox.Content().length())
      {
        // It looks like we didn't _trim_ this trailing run but removed the lat run instead (happens with
        // dedicated layout box). Let's put that content back as trailing run.
        if (AppendTrailingInlineItemAsTrailingRun(runs, trimmedTrailingWhitespaceWidth, inlineItemRange,
                                                  inlineItems))
        {
          return true;
        }
        krys_debug_assert(false);
        return false;
      }
      trailingRun._logicalWidth += trimmedTrailingWhitespaceWidth;
      // This must be collapsed whitespace.
      trailingRun._textContent->Length += 1;
      return true;
    };

    auto &lastRun = runs.back();
    if (lastRun.IsText())
    {
      return restore(lastRun);
    }

    auto isHardLineBreakWithTrailingTextRun =
      lastRun.IsHardLineBreak() && runs.size() > 1 && runs[runs.size() - 2].IsText();
    if (isHardLineBreakWithTrailingTextRun)
    {
      if (!restore(runs[runs.size() - 2]))
      {
        return false;
      }
      runs.back().MoveHorizontally(trimmedTrailingWhitespaceWidth);
      return true;
    }
    krys_debug_assert(false);
    return false;
  }

  bool Line::HasTrailingForcedLineBreak(const RunList &runs) noexcept
  {
    return !runs.empty() && runs.back().IsLineBreak();
  }

  void Line::TrimmableTrailingContent::AddFullyTrimmableContent(size_t runIndex,
                                                                InlineLayoutUnit trimmableContentOffset,
                                                                InlineLayoutUnit trimmableWidth) noexcept
  {
    // Any subsequent trimmable whitespace should collapse to zero advanced width and ignored at
    // ::appendText()
    krys_debug_assert(!_hasFullyTrimmableContent);
    _fullyTrimmableWidth = trimmableContentOffset + trimmableWidth;
    _trimmableContentOffset = trimmableContentOffset;
    // Note that just because the trimmable width is 0 (font-size: 0px), it does not mean we don't have a
    // trimmable trailing content.
    _hasFullyTrimmableContent = true;
    _firstTrimmableRunIndex = _firstTrimmableRunIndex.value_or(runIndex);
  }

  void Line::TrimmableTrailingContent::AddPartiallyTrimmableContent(size_t runIndex,
                                                                    InlineLayoutUnit trimmableWidth) noexcept
  {
    // Do not add trimmable letter spacing after a fully trimmable whitespace.
    krys_debug_assert(!_firstTrimmableRunIndex);
    krys_debug_assert(!_hasFullyTrimmableContent);
    krys_debug_assert(!_partiallyTrimmableWidth);
    krys_debug_assert(trimmableWidth);
    _partiallyTrimmableWidth = trimmableWidth;
    _firstTrimmableRunIndex = runIndex;
  }

  InlineLayoutUnit Line::TrimmableTrailingContent::Remove() noexcept
  {
    // Remove trimmable trailing content and move all the subsequent trailing runs.
    // <span> </span><span></span>
    // [trailing whitespace][inline box end][inline box start][inline box end]
    // Trim the whitespace run and move the trailing inline box runs to the logical left.
    krys_debug_assert(!IsEmpty());
    auto &trimmableRun = _runs[*_firstTrimmableRunIndex];
    krys_debug_assert(trimmableRun.IsText());

    auto trimmedWidth = _trimmableContentOffset;
    if (_hasFullyTrimmableContent)
    {
      trimmedWidth += trimmableRun.RemoveTrailingWhitespace();
    }
    if (_partiallyTrimmableWidth)
    {
      trimmedWidth += trimmableRun.RemoveTrailingLetterSpacing();
    }

    // When the trimmable run is followed by some non-content runs, we need to adjust their horizontal
    // positions. e.g. <div>text is followed by trimmable content    <span> </span></div> When the [text...]
    // run is trimmed (trailing whitespace is removed), both "<span>" and "</span>" runs need to be moved
    // horizontally to catch up with the [text...] run. Note that the whitespace inside the <span> does not
    // produce a run since in ::appendText() we see it as a fully trimmable run.
    for (auto index = *_firstTrimmableRunIndex + 1; index < _runs.size(); ++index)
    {
      auto &run = _runs[index];
      krys_debug_assert(run.IsWordBreakOpportunity() || run.IsLineSpanningInlineBoxStart()
                        || run.IsInlineBoxStart() || run.IsInlineBoxEnd() || run.IsLineBreak()
                        || run.IsOpaque());
      run.MoveHorizontally(-trimmedWidth);
    }
    if (!trimmableRun.TextContent()->Length)
    {
      // This trimmable run is fully collapsed now (e.g. <div><img>    <span></span></div>).
      // We don't need to keep it around anymore.
      _runs.erase(_runs.begin() + static_cast<ptrdiff_t>(*_firstTrimmableRunIndex));
    }
    Reset();
    return trimmedWidth;
  }

  InlineLayoutUnit Line::TrimmableTrailingContent::RemovePartiallyTrimmableContent() noexcept
  {
    // Partially trimmable content is always gated by a fully trimmable content.
    // We can't just trim spacing in the middle.
    krys_debug_assert(!_fullyTrimmableWidth);
    return Remove();
  }

  namespace
  {
    KRYS_NODISCARD inline Line::Run::Type ToLineRunType(const InlineItem &inlineItem) noexcept
    {
      switch (inlineItem.GetType())
      {
        case InlineItem::Type::HardLineBreak:        return Line::Run::Type::HardLineBreak;
        case InlineItem::Type::WordBreakOpportunity: return Line::Run::Type::WordBreakOpportunity;
        case InlineItem::Type::AtomicInlineBox:
          if (inlineItem.LayoutBox().IsListMarkerBox())
          {
            return Downcast<ElementBox>(inlineItem.LayoutBox()).IsListMarkerOutside()
                     ? Line::Run::Type::ListMarkerOutside
                     : Line::Run::Type::ListMarkerInside;
          }
          return Line::Run::Type::AtomicInlineBox;
        case InlineItem::Type::InlineBoxStart: return Line::Run::Type::InlineBoxStart;
        case InlineItem::Type::InlineBoxEnd:   return Line::Run::Type::InlineBoxEnd;
        case InlineItem::Type::Opaque:         return Line::Run::Type::Opaque;
        case InlineItem::Type::Block:          return Line::Run::Type::Block;
        default:                               krys_debug_assert(false); return {};
      }
    }
  }

  Maybe<Line::Run::TrailingWhitespace::Type>
    Line::Run::TrailingWhitespaceType(const InlineTextItem &inlineTextItem) noexcept
  {
    if (!inlineTextItem.IsWhitespace())
    {
      return {};
    }
    if (InlineTextItem::ShouldPreserveSpacesAndTabs(inlineTextItem))
    {
      return TrailingWhitespace::Type::NotCollapsible;
    }
    if (inlineTextItem.Length() == 1)
    {
      return TrailingWhitespace::Type::Collapsible;
    }
    return TrailingWhitespace::Type::Collapsed;
  }

  Line::Run::Run(const InlineItem &inlineItem, const RenderStyle &style, InlineLayoutUnit logicalLeft,
                 InlineLayoutUnit logicalWidth, InlineLayoutUnit textSpacingAdjustment) noexcept
      : _type(ToLineRunType(inlineItem)), _layoutBox(&inlineItem.LayoutBox()), _style(&style),
        _logicalLeft(logicalLeft), _logicalWidth(logicalWidth), _bidiLevel(inlineItem.BidiLevel()),
        _textSpacingAdjustment(textSpacingAdjustment)
  {
  }

  Line::Run::Run(const InlineItem &zeroWidthInlineItem, const RenderStyle &style,
                 InlineLayoutUnit logicalLeft) noexcept
      : _type(ToLineRunType(zeroWidthInlineItem)), _layoutBox(&zeroWidthInlineItem.LayoutBox()),
        _style(&style), _logicalLeft(logicalLeft), _bidiLevel(zeroWidthInlineItem.BidiLevel())
  {
  }

  Line::Run::Run(const InlineItem &lineSpanningInlineBoxItem, InlineLayoutUnit logicalLeft,
                 InlineLayoutUnit logicalWidth, InlineLayoutUnit textSpacingAdjustment) noexcept
      : _type(Type::LineSpanningInlineBoxStart), _layoutBox(&lineSpanningInlineBoxItem.LayoutBox()),
        _style(&lineSpanningInlineBoxItem.Style()), _logicalLeft(logicalLeft), _logicalWidth(logicalWidth),
        _bidiLevel(lineSpanningInlineBoxItem.BidiLevel()), _textSpacingAdjustment(textSpacingAdjustment)
  {
    krys_debug_assert(lineSpanningInlineBoxItem.IsInlineBoxStart());
  }

  Line::Run::Run(const InlineSoftLineBreakItem &softLineBreakItem, const RenderStyle &style,
                 InlineLayoutUnit logicalLeft) noexcept
      : _type(Type::SoftLineBreak), _layoutBox(&softLineBreakItem.LayoutBox()), _style(&style),
        _logicalLeft(logicalLeft), _bidiLevel(softLineBreakItem.BidiLevel()),
        _textContent(Text {softLineBreakItem.Position(), 1})
  {
  }

  Line::Run::Run(const InlineTextItem &inlineTextItem, const RenderStyle &style, InlineLayoutUnit logicalLeft,
                 InlineLayoutUnit logicalWidth, InlineLayoutUnit textSpacingAdjustment,
                 Maybe<Line::ShapingBoundary> shapingBoundary) noexcept
      : _type(inlineTextItem.IsWordSeparator()           ? Type::WordSeparator
              : inlineTextItem.IsQuirkNonBreakingSpace() ? Type::NonBreakingSpace
                                                         : Type::Text),
        _layoutBox(&inlineTextItem.LayoutBox()), _style(&style), _logicalLeft(logicalLeft),
        _logicalWidth(logicalWidth), _bidiLevel(inlineTextItem.BidiLevel()),
        _shapingBoundary(shapingBoundary), _textSpacingAdjustment(textSpacingAdjustment)
  {
    size_t length = inlineTextItem.Length();
    auto whitespaceType = TrailingWhitespaceType(inlineTextItem);
    if (whitespaceType)
    {
      if (*whitespaceType == TrailingWhitespace::Type::Collapsed)
      {
        length = 1;
      }
      _trailingWhitespace = TrailingWhitespace {*whitespaceType, logicalWidth, length};
    }
    _textContent = Text {inlineTextItem.Start(), length};
  }

  void Line::Run::Expand(const InlineTextItem &inlineTextItem, InlineLayoutUnit logicalWidth) noexcept
  {
    krys_debug_assert(!HasCollapsedTrailingWhitespace());
    krys_debug_assert(IsText() && inlineTextItem.IsText());
    krys_debug_assert(_layoutBox == &inlineTextItem.LayoutBox());
    krys_debug_assert(_bidiLevel == inlineTextItem.BidiLevel());
    krys_debug_assert(!_shapingBoundary || *_shapingBoundary != ShapingBoundary::End);

    _logicalWidth += logicalWidth;
    auto whitespaceType = TrailingWhitespaceType(inlineTextItem);

    if (!whitespaceType)
    {
      _trailingWhitespace = {};
      _textContent->Length += inlineTextItem.Length();
      _lastNonWhitespaceContentStart = inlineTextItem.Start();
      return;
    }
    auto whitespaceWidth = !_trailingWhitespace ? logicalWidth : _trailingWhitespace->Width + logicalWidth;
    size_t trailingWhitespaceLength =
      *whitespaceType == TrailingWhitespace::Type::Collapsed ? 1 : inlineTextItem.Length();
    _trailingWhitespace = TrailingWhitespace {*whitespaceType, whitespaceWidth, trailingWhitespaceLength};
    _textContent->Length += trailingWhitespaceLength;
  }

  Maybe<Line::Run> Line::Run::DetachTrailingWhitespace() noexcept
  {
    if (!_trailingWhitespace || IsWhitespaceOnly())
    {
      return {};
    }

    krys_debug_assert(_trailingWhitespace->Length < _textContent->Length);
    auto trailingWhitespaceRun = *this;

    auto leadingNonWhitespaceContentLength = _textContent->Length - _trailingWhitespace->Length;
    trailingWhitespaceRun._textContent =
      Text {_textContent->Start + leadingNonWhitespaceContentLength, _trailingWhitespace->Length, false};

    trailingWhitespaceRun._logicalWidth = _trailingWhitespace->Width;
    trailingWhitespaceRun._logicalLeft = LogicalRight() - _trailingWhitespace->Width;

    trailingWhitespaceRun._trailingWhitespace = {};
    trailingWhitespaceRun._lastNonWhitespaceContentStart = {};

    _logicalWidth -= trailingWhitespaceRun.LogicalWidth();
    _textContent->Length = leadingNonWhitespaceContentLength;
    _trailingWhitespace = {};

    return trailingWhitespaceRun;
  }

  bool Line::Run::HasTrailingLetterSpacing() const noexcept
  {
    return !HasTrailingWhitespace() && LetterSpacing() > 0;
  }

  InlineLayoutUnit Line::Run::TrailingLetterSpacing() const noexcept
  {
    if (!HasTrailingLetterSpacing())
    {
      return {};
    }
    return InlineLayoutUnit {LetterSpacing()};
  }

  InlineLayoutUnit Line::Run::RemoveTrailingLetterSpacing() noexcept
  {
    krys_debug_assert(HasTrailingLetterSpacing());
    auto trailingWidth = TrailingLetterSpacing();
    ShrinkHorizontally(trailingWidth);
    return trailingWidth;
  }

  InlineLayoutUnit Line::Run::RemoveTrailingWhitespace() noexcept
  {
    krys_debug_assert(_trailingWhitespace);
    // According to https://www.w3.org/TR/css-text-3/#white-space-property matrix
    // Trimmable whitespace is always collapsible so the length of the trailing trimmable whitespace is always
    // 1 (or non-existent).
    krys_debug_assert(_textContent && _textContent->Length);
    constexpr size_t trailingTrimmableContentLength = 1;

    auto trimmedWidth = _trailingWhitespace->Width;
    if (_lastNonWhitespaceContentStart && InlineDirection() == TextDirection::RTL)
    {
      // While LTR content could also suffer from slightly incorrect content width after trimming trailing
      // whitespace (see TextUtil::width) it hardly produces visually observable result.
      // FIXME: This may still incorrectly leave some content on the line (vs. re-measuring also at ::expand).
      auto &inlineTextBox = Downcast<InlineTextBox>(*_layoutBox);
      auto startPosition = *_lastNonWhitespaceContentStart;
      auto endPosition = _textContent->Start + _textContent->Length;
      krys_assert(startPosition < endPosition - trailingTrimmableContentLength);
      if (inlineTextBox.Content()[endPosition - 1] == u8' ')
      {
        trimmedWidth = TextUtil::TrailingWhitespaceWidth(inlineTextBox, _style->GetFontCascade(),
                                                         startPosition, endPosition);
      }
    }
    _textContent->Length -= trailingTrimmableContentLength;
    _trailingWhitespace = {};
    ShrinkHorizontally(trimmedWidth);
    return trimmedWidth;
  }

  bool Line::Run::IsContentfulOrHasDecoration(const Run &run,
                                              const InlineFormattingContext &formattingContext) noexcept
  {
    if (run.IsContentful())
    {
      return true;
    }
    if (run.IsInlineBox())
    {
      if (run.LogicalWidth())
      {
        return true;
      }
      if (run.LayoutBox().IsRubyBase())
      {
        return true;
      }
      // Even negative horizontal margin makes the line "contentful".
      auto &inlineBoxGeometry = formattingContext.GeometryForBox(run.LayoutBox());
      if (run.IsInlineBoxStart())
      {
        return inlineBoxGeometry.MarginStart() || inlineBoxGeometry.BorderStart()
               || inlineBoxGeometry.PaddingStart();
      }
      if (run.IsInlineBoxEnd())
      {
        return inlineBoxGeometry.MarginEnd() || inlineBoxGeometry.BorderEnd()
               || inlineBoxGeometry.PaddingEnd();
      }
      if (run.IsLineSpanningInlineBoxStart())
      {
        if (run.Style().GetBoxDecorationBreak() != BoxDecorationBreak::Clone)
        {
          return false;
        }
        return inlineBoxGeometry.BorderStart() || inlineBoxGeometry.PaddingStart();
      }
    }
    return false;
  }

  TextDirection Line::Run::InlineDirection() const noexcept
  {
    return _style->GetWritingMode().BidiDirection();
  }

  bool Line::Run::HasTextCombine() const noexcept
  {
    return _style->HasTextCombine();
  }

  InlineLayoutUnit Line::Run::LetterSpacing() const noexcept
  {
    return _style->UsedLetterSpacing();
  }
}
