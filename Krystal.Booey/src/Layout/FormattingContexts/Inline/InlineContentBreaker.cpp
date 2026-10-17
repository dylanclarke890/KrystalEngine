#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentBreaker.hpp"
#include "Krystal.Booey/Core/Hyphenation.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <unicode/uchar.h>

namespace krys::boo::layout
{
  struct CandidateTextRunForBreaking
  {
    size_t Index {0};
    bool IsOverflowingRun {true};
    InlineLayoutUnit LogicalLeft {0};
  };

  namespace
  {
    using ContinuousContent = InlineContentBreaker::ContinuousContent;

    KRYS_NODISCARD char32 CodePointAt(utf8_stringview text, size_t position) noexcept
    {
      return text::DecodeUTF8(Span<const char8>(text.data() + position, text.size() - position)).CodePoint;
    }

    /// @brief The byte offset of the code point that ends at @p position.
    KRYS_NODISCARD size_t PreviousCodePointStart(utf8_stringview text, size_t position) noexcept
    {
      krys_debug_assert(position);
      do
      {
        --position;
      } while (position && text::IsTrailingUTF8(text[position]));
      return position;
    }

    KRYS_NODISCARD size_t NextCodePointStart(utf8_stringview text, size_t position) noexcept
    {
      return position
             + text::DecodeUTF8(Span<const char8>(text.data() + position, text.size() - position)).Length;
    }

    KRYS_NODISCARD inline bool HasLeadingTextContent(const ContinuousContent &continuousContent) noexcept
    {
      for (auto &run : continuousContent.Runs())
      {
        auto &inlineItem = run.Item;
        if (inlineItem.IsInlineBoxStartOrEnd() || inlineItem.IsOpaque())
        {
          continue;
        }
        return inlineItem.IsText();
      }
      return false;
    }

    KRYS_NODISCARD inline Maybe<size_t> NextTextRunIndex(const ContinuousContent::RunList &runs,
                                                         size_t startIndex) noexcept
    {
      for (auto index = startIndex + 1; index < runs.size(); ++index)
      {
        if (runs[index].Item.IsText())
        {
          return index;
        }
      }
      return {};
    }

    KRYS_NODISCARD inline bool IsWhitespaceOnlyContent(const ContinuousContent &continuousContent) noexcept
    {
      // [<span></span> ] [<span> </span>] [ <span style="padding: 0px;"></span>] are all considered visually
      // empty whitespace content. [<span style="border: 1px solid red"></span> ] while this is whitespace
      // content only, it is not considered visually empty.
      krys_debug_assert(!continuousContent.Runs().empty());
      auto hasWhitespace = false;
      for (auto &run : continuousContent.Runs())
      {
        auto &inlineItem = run.Item;
        if (inlineItem.IsInlineBoxStartOrEnd() || inlineItem.IsOpaque())
        {
          continue;
        }
        auto isWhitespace = [&]
        {
          auto *textItem = DynamicDowncast<InlineTextItem>(inlineItem);
          return textItem && textItem->IsWhitespace();
        }();
        if (!isWhitespace)
        {
          return false;
        }
        hasWhitespace = true;
      }
      return hasWhitespace;
    }

    KRYS_NODISCARD inline bool IsNonContentRunsOnly(const ContinuousContent &continuousContent) noexcept
    {
      // <span></span> <- non content runs.
      for (auto &run : continuousContent.Runs())
      {
        auto &inlineItem = run.Item;
        if (inlineItem.IsInlineBoxStartOrEnd() || inlineItem.IsOpaque())
        {
          continue;
        }
        if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem);
            inlineTextItem && inlineTextItem->IsEmpty())
        {
          continue;
        }
        return false;
      }
      return true;
    }

    KRYS_NODISCARD inline Maybe<size_t>
      FirstTextRunIndex(const ContinuousContent::RunList &continuousContentRuns) noexcept
    {
      for (size_t index = 0; index < continuousContentRuns.size(); ++index)
      {
        if (continuousContentRuns[index].Item.IsText())
        {
          return index;
        }
      }
      return {};
    }

    KRYS_NODISCARD Maybe<size_t>
      FindTrailingRunIndexBeforeBreakableRun(const ContinuousContent::RunList &runs,
                                             size_t breakableRunIndex) noexcept
    {
      // When the breaking position is at the beginning of the run, the trailing run is the previous one.
      if (!breakableRunIndex)
      {
        return {};
      }
      // Try not break content at inline box boundary
      // e.g. <span>fits</span><span>overflows</span>
      // when the text "overflows" completely overflows, let's break the content right before the '<span>'.
      auto lastOpaqueItemIndex = Maybe<size_t> {};
      for (auto trailingCandidateIndex = breakableRunIndex; trailingCandidateIndex--;)
      {
        auto &inlineItem = runs[trailingCandidateIndex].Item;
        if (inlineItem.IsOpaque())
        {
          lastOpaqueItemIndex = trailingCandidateIndex;
          continue;
        }
        auto isAtInlineBox = inlineItem.IsInlineBoxStart();
        if (!isAtInlineBox)
        {
          return lastOpaqueItemIndex.value_or(trailingCandidateIndex);
        }
        lastOpaqueItemIndex = {};
      }
      return {};
    }

    KRYS_NODISCARD bool IsBreakableRun(const ContinuousContent::Run &run) noexcept
    {
      if (!run.Item.IsText())
      {
        // Can't break horizontal spacing -> e.g. <span style="padding-right: 100px;">textcontent</span>, if
        // the [inline box end] is the overflown inline item we need to check if there's another inline item
        // beyond the [inline box end] to split.
        return false;
      }
      // Check if this text run needs to stay on the current line.
      return TextUtil::IsWrappingAllowed(run.Style);
    }

    KRYS_NODISCARD inline bool CanBreakBefore(char32 character, LineBreak lineBreak) noexcept
    {
      // FIXME: This should include all the cases from https://unicode.org/reports/tr14
      // Use a breaking matrix similar to lineBreakTable in BreakablePositions.cpp
      // Also see kBreakAllLineBreakClassTable in
      // third_party/blink/renderer/platform/text/text_break_iterator.cc
      if (lineBreak != LineBreak::Loose)
      {
        // The following breaks are allowed for loose line breaking if the preceding character belongs to the
        // Unicode line breaking class ID, and are otherwise forbidden: ‐ U+2010, – U+2013
        // https://drafts.csswg.org/css-text/#line-break-property
        if (character == text::Hyphen || character == text::EnDash)
        {
          return false;
        }
      }
      if (character == text::NoBreakSpace)
      {
        return false;
      }
      auto isPunctuation = U_GET_GC_MASK(static_cast<UChar32>(character))
                           & (U_GC_PS_MASK | U_GC_PE_MASK | U_GC_PI_MASK | U_GC_PF_MASK | U_GC_PO_MASK);
      return character == text::ReverseSolidus || !isPunctuation;
    }

    KRYS_NODISCARD inline Maybe<size_t> LastValidBreakingPosition(const ContinuousContent::RunList &runs,
                                                                  size_t textRunIndex) noexcept
    {
      auto &textRun = runs[textRunIndex];
      auto &inlineTextItem = Downcast<InlineTextItem>(textRun.Item);
      krys_debug_assert(inlineTextItem.Length());
      auto lineBreak = textRun.Style.GetLineBreak();

      auto lastValidBreakingPositionInsideTextRun = [&]() -> Maybe<size_t>
      {
        // Find out if the candidate position for arbitrary breaking is valid. We can't always break between
        // any characters.
        auto text = utf8_stringview {inlineTextItem.GetInlineTextBox().Content()};
        size_t left = inlineTextItem.Start();
        for (auto index = PreviousCodePointStart(text, inlineTextItem.End()); index > left;
             index = PreviousCodePointStart(text, index))
        {
          if (CanBreakBefore(CodePointAt(text, index), lineBreak))
          {
            return index;
          }
        }
        return {};
      };

      if (auto nextTextRunCandidateIndex = NextTextRunIndex(runs, textRunIndex))
      {
        auto &nextInlineTextItem = Downcast<InlineTextItem>(runs[*nextTextRunCandidateIndex].Item);
        auto canBreakAtRunBoundary =
          nextInlineTextItem.IsWhitespace()
            ? nextInlineTextItem.Style().GetWhiteSpaceCollapse() != WhiteSpaceCollapse::BreakSpaces
            : CanBreakBefore(CodePointAt(nextInlineTextItem.Content(), 0), lineBreak);
        if (canBreakAtRunBoundary)
        {
          return inlineTextItem.End();
        }
        return lastValidBreakingPositionInsideTextRun();
      }

      if (textRunIndex == runs.size() - 1)
      {
        return inlineTextItem.End();
      }

      if (!runs[textRunIndex + 1].Item.IsInlineBoxStartOrEnd())
      {
        krys_debug_assert(false);
        return inlineTextItem.End();
      }

      return lastValidBreakingPositionInsideTextRun();
    }

    KRYS_NODISCARD Maybe<TextUtil::WordBreakLeft> MidWordBreak(const ContinuousContent::Run &textRun,
                                                               InlineLayoutUnit runLogicalLeft,
                                                               InlineLayoutUnit availableWidth) noexcept
    {
      krys_debug_assert(textRun.Style.GetWordBreak() == WordBreak::BreakAll);
      auto &inlineTextItem = Downcast<InlineTextItem>(textRun.Item);

      auto wordBreak = TextUtil::BreakWord(inlineTextItem, textRun.Style.GetFontCascade(),
                                           textRun.SpaceRequired(), availableWidth, runLogicalLeft);
      if (!wordBreak.Length || wordBreak.Length == inlineTextItem.Length())
      {
        return {};
      }

      // Find out if the candidate position for arbitrary breaking is valid. We can't always break between any
      // characters.
      auto lineBreak = textRun.Style.GetLineBreak();
      auto text = utf8_stringview {inlineTextItem.GetInlineTextBox().Content()};
      if (CanBreakBefore(CodePointAt(text, inlineTextItem.Start() + wordBreak.Length), lineBreak))
      {
        return wordBreak;
      }

      const size_t left = inlineTextItem.Start();
      auto right = left + wordBreak.Length;
      while (right > left)
      {
        if (CanBreakBefore(CodePointAt(text, right), lineBreak))
        {
          break;
        }
        right = PreviousCodePointStart(text, right);
      }
      if (left == right)
      {
        return {};
      }
      return TextUtil::WordBreakLeft {
        right - left, TextUtil::Width(inlineTextItem, textRun.Style.GetFontCascade(),
                                      static_cast<uint32>(left), static_cast<uint32>(right), runLogicalLeft)};
    }

    KRYS_NODISCARD size_t LimitBeforeValue(const RenderStyle &style) noexcept
    {
      auto value = style.HyphenateLimitBefore().TryValue();
      return value ? static_cast<size_t>(value->Value) : 0;
    }

    KRYS_NODISCARD size_t LimitAfterValue(const RenderStyle &style) noexcept
    {
      auto value = style.HyphenateLimitAfter().TryValue();
      return value ? static_cast<size_t>(value->Value) : 0;
    }

    KRYS_NODISCARD inline bool HasEnoughContentForHyphenation(size_t contentLength,
                                                              const RenderStyle &style) noexcept
    {
      return LimitBeforeValue(style) + LimitAfterValue(style) <= contentLength;
    }

    KRYS_NODISCARD Maybe<size_t> FirstHyphenPosition(utf8_stringview content,
                                                     const RenderStyle &style) noexcept
    {
      // FIXME: We may produce slightly incorrect (less fine-grained) hyphenation here as the incoming content
      // may just be a partial word. (same applies to hyphenPosition below)
      size_t contentLength = content.length();
      if (!HasEnoughContentForHyphenation(contentLength, style))
      {
        return {};
      }

      auto limitBefore = LimitBeforeValue(style);
      auto candidatePosition = std::min(contentLength, contentLength - LimitAfterValue(style) + 1);
      auto firstHyphenLocation = Maybe<size_t> {};
      while (true)
      {
        auto hyphenIndex =
          LastHyphenLocation(content, candidatePosition, style.ComputedLocale().GetPlatform());
        if (!hyphenIndex || hyphenIndex < limitBefore)
        {
          return firstHyphenLocation;
        }
        if (hyphenIndex >= candidatePosition)
        {
          krys_debug_assert(false);
          return {};
        }
        firstHyphenLocation = hyphenIndex;
        candidatePosition = hyphenIndex;
      }
    }

    KRYS_NODISCARD Maybe<size_t> LastHyphenPosition(utf8_stringview content,
                                                    const RenderStyle &style) noexcept
    {
      size_t contentLength = content.length();
      if (!HasEnoughContentForHyphenation(contentLength, style))
      {
        return {};
      }

      if (auto hyphenIndex =
            LastHyphenLocation(content, std::min(contentLength, contentLength - LimitAfterValue(style) + 1),
                               style.ComputedLocale().GetPlatform()))
      {
        return hyphenIndex >= LimitBeforeValue(style) ? Maybe<size_t> {hyphenIndex} : Maybe<size_t> {};
      }
      return {};
    }

    KRYS_NODISCARD Maybe<size_t> HyphenPositionBefore(utf8_stringview content, const RenderStyle &style,
                                                      size_t beforePosition) noexcept
    {
      // Find the hyphen position as follows:
      // 1. Split the text by taking the hyphen width into account
      // 2. Find the last hyphen position before the split position
      auto contentLength = content.length();
      if (beforePosition < LimitBeforeValue(style) || !HasEnoughContentForHyphenation(contentLength, style))
      {
        return {};
      }

      if (auto hyphenIndex =
            LastHyphenLocation(content, std::min(beforePosition, contentLength - LimitAfterValue(style)) + 1,
                               style.ComputedLocale().GetPlatform()))
      {
        return hyphenIndex >= LimitBeforeValue(style) ? Maybe<size_t> {hyphenIndex} : Maybe<size_t> {};
      }
      return {};
    }
  }

  InlineContentBreaker::Result
    InlineContentBreaker::ProcessInlineContent(const ContinuousContent &candidateContent,
                                               const LineStatus &lineStatus) noexcept
  {
    krys_debug_assert(!std::isnan(lineStatus.AvailableWidth));
    krys_debug_assert(IsMinimumInIntrinsicWidthMode()
                      || candidateContent.LogicalWidth() > lineStatus.AvailableWidth);

    if (auto result = SimplifiedMinimumInstrinsicWidthBreak(candidateContent, lineStatus))
    {
      return *result;
    }

    auto result = ProcessOverflowingContent(candidateContent, lineStatus);
    if (result.ResultAction == Result::Action::Wrap && lineStatus.TrailingSoftHyphenWidth
        && HasLeadingTextContent(candidateContent))
    {
      // A trailing soft hyphen with a wrapped text content turns into a visible hyphen.
      // Let's check if there's enough space for the hyphen character.
      auto hyphenOverflows = *lineStatus.TrailingSoftHyphenWidth > lineStatus.AvailableWidth;
      auto action = hyphenOverflows ? Result::Action::RevertToLastNonOverflowingWrapOpportunity
                                    : Result::Action::WrapWithHyphen;
      result = {action, IsEndOfLine::Yes};
    }
    return result;
  }

  InlineContentBreaker::Result
    InlineContentBreaker::ProcessOverflowingContent(const ContinuousContent &continuousContent,
                                                    const LineStatus &lineStatus) const noexcept
  {
    krys_debug_assert(!continuousContent.Runs().empty());

    krys_debug_assert(continuousContent.LogicalWidth() > lineStatus.AvailableWidth);
    auto checkForTrailingContentFit = [&]() -> Maybe<InlineContentBreaker::Result>
    {
      if (continuousContent.IsFullyTrimmable())
      {
        // fully trimmable content stays on the current line (and gets fully trimmed).
        return InlineContentBreaker::Result {Result::Action::Keep};
      }
      if (continuousContent.HasTrimmableSpace())
      {
        // Check if the content fits if we trimmed it.
        if (IsWhitespaceOnlyContent(continuousContent))
        {
          return InlineContentBreaker::Result {Result::Action::Keep};
        }
        auto spaceRequired = continuousContent.LogicalWidth() - continuousContent.TrailingTrimmableWidth();
        if (lineStatus.HasFullyTrimmableTrailingContent)
        {
          spaceRequired -= continuousContent.LeadingTrimmableWidth();
        }
        if (spaceRequired <= lineStatus.AvailableWidth)
        {
          return InlineContentBreaker::Result {Result::Action::Keep};
        }
      }

      if (continuousContent.IsHangingContent())
      {
        return InlineContentBreaker::Result {Result::Action::Keep};
      }
      if (continuousContent.HasHangingSpace())
      {
        auto spaceRequired = continuousContent.LogicalWidth() - continuousContent.HangingContentWidth();
        if (spaceRequired <= lineStatus.AvailableWidth)
        {
          return InlineContentBreaker::Result {Result::Action::Keep};
        }
      }

      auto canIgnoreNonContentTrailingRuns =
        lineStatus.TrimmableOrHangingWidth && IsNonContentRunsOnly(continuousContent);
      if (canIgnoreNonContentTrailingRuns)
      {
        // Let's see if the non-content runs fit when the line has trailing trimmable/hanging content.
        // "text content <span style="padding: 1px"></span>" <- the <span></span> runs could fit after
        // trimming the trailing whitespace.
        if (continuousContent.LogicalWidth()
            <= lineStatus.AvailableWidth + lineStatus.TrimmableOrHangingWidth)
        {
          return InlineContentBreaker::Result {Result::Action::Keep};
        }
      }

      return {};
    };
    if (auto result = checkForTrailingContentFit())
    {
      return *result;
    }

    size_t overflowingRunIndex = 0;
    if (continuousContent.HasTextContent())
    {
      auto tryBreakingContentWithText = [&]() -> Maybe<Result>
      {
        // 1. This text content is not breakable.
        // 2. This breakable text content does not fit at all. Not even the first glyph. This is a very
        // special case.
        // 3. We can break the content but it still overflows.
        // 4. Managed to break the content before the overflow point.
        auto overflowingContent = ProcessOverflowingContentWithText(continuousContent, lineStatus);
        overflowingRunIndex = overflowingContent.RunIndex;
        if (!overflowingContent.Breaking)
        {
          return {};
        }
        auto trailingContent = overflowingContent.Breaking->Trailing;
        if (!trailingContent)
        {
          // We tried to break the content but the available space can't even accommodate the first glyph.
          // 1. Wrap the content over to the next line when we've got content on the line already.
          // 2. Keep the first glyph on the empty line (or keep the whole run if it has only one
          // glyph/completely empty) including closing inline boxes e.g. <span><span>X</span></span> where "X"
          // is the overflowing glyph).
          if (lineStatus.HasContent)
          {
            return Result {Result::Action::Wrap, IsEndOfLine::Yes};
          }

          auto leadingTextRunIndex = *FirstTextRunIndex(continuousContent.Runs());
          auto &leadingTextRun = continuousContent.Runs()[leadingTextRunIndex];
          auto &inlineTextItem = Downcast<InlineTextItem>(leadingTextRun.Item);
          auto firstCharacterLength = TextUtil::FirstUserPerceivedCharacterLength(inlineTextItem);
          krys_debug_assert(firstCharacterLength > 0);

          if (inlineTextItem.Length() <= firstCharacterLength)
          {
            auto trailingRunIndex = [&]() -> Maybe<size_t>
            {
              // Keep the overflowing text content and the closing inline box runs together.
              // e.g. X</span><span>Y</span> where "X" overflows, the trailing run index is 1.
              auto &runs = continuousContent.Runs();
              if (leadingTextRunIndex == runs.size() - 1)
              {
                return {};
              }
              for (auto runIndex = leadingTextRunIndex + 1; runIndex < runs.size(); ++runIndex)
              {
                auto &inlineItem = runs[runIndex].Item;
                if (inlineItem.IsOpaque())
                {
                  continue;
                }
                if (!inlineItem.IsInlineBoxEnd())
                {
                  return runIndex - 1;
                }
              }
              return {};
            };
            if (auto runToBreakAfter = trailingRunIndex())
            {
              return Result {Result::Action::Break, IsEndOfLine::Yes,
                             Result::PartialTrailingContent {*runToBreakAfter, {}, {}}};
            }
            return Result {Result::Action::Keep, IsEndOfLine::Yes};
          }

          auto firstCharacterWidth =
            TextUtil::Width(inlineTextItem, leadingTextRun.Style.GetFontCascade(), inlineTextItem.Start(),
                            inlineTextItem.Start() + static_cast<uint32>(firstCharacterLength),
                            lineStatus.ContentLogicalRight);
          return Result {Result::Action::Break, IsEndOfLine::Yes,
                         Result::PartialTrailingContent {
                           leadingTextRunIndex, PartialRun {firstCharacterLength, firstCharacterWidth}, {}}};
        }
        if (trailingContent->Overflows && lineStatus.HasContent)
        {
          // We managed to break a run with overflow but the line already has content. Let's wrap it to the
          // next line.
          return Result {Result::Action::Wrap, IsEndOfLine::Yes};
        }
        // Either we managed to break with no overflow or the line is empty.
        auto trailingPartialContent = Result::PartialTrailingContent {
          overflowingContent.Breaking->RunIndex, trailingContent->Partial, trailingContent->HyphenWidth};
        return Result {Result::Action::Break, IsEndOfLine::Yes, trailingPartialContent};
      };
      if (auto result = tryBreakingContentWithText())
      {
        return *result;
      }
    }
    else if (continuousContent.Runs().size() > 1)
    {
      // FIXME: Add support for various content.
      auto &runs = continuousContent.Runs();
      for (size_t i = 0; i < runs.size(); ++i)
      {
        if (runs[i].Item.IsAtomicInlineBox())
        {
          overflowingRunIndex = i;
          break;
        }
      }
    }

    // If we are not allowed to break this overflowing content, we still need to decide whether keep it or
    // wrap it to the next line.
    if (!lineStatus.HasContent)
    {
      return {Result::Action::Keep, IsEndOfLine::No};
    }
    // Now either wrap this content over to the next line or revert back to an earlier wrapping opportunity,
    // or not wrap at all.
    auto shouldWrapUnbreakableContentToNextLine = [&]
    {
      // The individual runs in this continuous content don't break, let's check if we are allowed to wrap
      // this content to next line (e.g. pre would prevent us from wrapping). Parent style drives the wrapping
      // behavior here unless the overflowing run is an inline box. In such cases decoration overflow is
      // considered as "content" and we need to check the style accordingly. e.g. <div style="white-space:
      // nowrap">no wrap<div style="display: inline-block; white-space: normal">yes wrap</div></div>. While
      // the inline-block has pre-wrap which allows wrapping (for its own content), the content lives in a
      // nowrap context.
      auto &runs = continuousContent.Runs();
      auto &overflowingBox = runs[overflowingRunIndex].Item.LayoutBox();
      auto &styleToUse =
        overflowingBox.IsInlineBox() ? overflowingBox.Style() : overflowingBox.Parent().Style();
      auto isWrappingAllowed = TextUtil::IsWrappingAllowed(styleToUse);
      for (auto index = overflowingRunIndex; !isWrappingAllowed && index--;)
      {
        auto &previousStyle = runs[index].Item.LayoutBox().Parent().Style();
        isWrappingAllowed = TextUtil::IsWrappingAllowed(previousStyle);
      }
      return isWrappingAllowed;
    };
    if (shouldWrapUnbreakableContentToNextLine())
    {
      return {Result::Action::Wrap, IsEndOfLine::Yes};
    }
    if (lineStatus.HasWrapOpportunityAtPreviousPosition)
    {
      return {Result::Action::RevertToLastWrapOpportunity, IsEndOfLine::Yes};
    }
    return {Result::Action::Keep, IsEndOfLine::No};
  }

  Maybe<InlineContentBreaker::Result>
    InlineContentBreaker::SimplifiedMinimumInstrinsicWidthBreak(const ContinuousContent &candidateContent,
                                                                const LineStatus &lineStatus) const noexcept
  {
    if (!IsMinimumInIntrinsicWidthMode() || !candidateContent.IsTextOnlyContent())
    {
      return {};
    }

    auto &leadingInlineTextItem = Downcast<InlineTextItem>(candidateContent.Runs().front().Item);
    auto &style = leadingInlineTextItem.Style();
    if (!TextUtil::IsWrappingAllowed(style))
    {
      return Result {Result::Action::Keep, IsEndOfLine::No};
    }

    if (!lineStatus.HasContent)
    {
      if (leadingInlineTextItem.IsEmpty())
      {
        return Result {Result::Action::Keep, IsEndOfLine::No};
      }
      auto breakBehavior = WordBreakBehavior(style, {});
      if (breakBehavior.IsEmpty())
      {
        return Result {Result::Action::Keep, IsEndOfLine::No};
      }

      if (breakBehavior.ContainsAny(
            {WordBreakRule::AtArbitraryPositionWithinWords, WordBreakRule::AtArbitraryPosition}))
      {
        auto firstCharacterLength = TextUtil::FirstUserPerceivedCharacterLength(leadingInlineTextItem);
        if (leadingInlineTextItem.Length() <= firstCharacterLength)
        {
          return Result {Result::Action::Keep, IsEndOfLine::Yes};
        }
        auto firstCharacterWidth =
          TextUtil::Width(leadingInlineTextItem, style.GetFontCascade(), leadingInlineTextItem.Start(),
                          leadingInlineTextItem.Start() + static_cast<uint32>(firstCharacterLength), {},
                          TextUtil::UseTrailingWhitespaceMeasuringOptimization::No);
        return Result {
          Result::Action::Break, IsEndOfLine::Yes,
          Result::PartialTrailingContent {{}, PartialRun {firstCharacterLength, firstCharacterWidth}, {}}};
      }
      return {};
    }
    return Result {!lineStatus.TrailingSoftHyphenWidth
                     ? Result::Action::Wrap
                     : Result::Action::RevertToLastNonOverflowingWrapOpportunity,
                   IsEndOfLine::Yes};
  }

  Maybe<InlineContentBreaker::PartialRun> InlineContentBreaker::TryBreakingTextRun(
    const ContinuousContent::RunList &runs, const CandidateTextRunForBreaking &candidateTextRun,
    InlineLayoutUnit availableWidth, const LineStatus &lineStatus) const noexcept
  {
    auto &candidateRun = runs[candidateTextRun.Index];
    krys_debug_assert(candidateRun.Item.IsText());
    auto &inlineTextItem = Downcast<InlineTextItem>(candidateRun.Item);
    auto &style = candidateRun.Style;
    auto lineHasRoomForContent = availableWidth > 0;

    auto breakRules = WordBreakBehavior(style, lineStatus.HasWrapOpportunityAtPreviousPosition);
    if (breakRules.IsEmpty())
    {
      return {};
    }

    auto &fontCascade = style.GetFontCascade();
    if (breakRules.Contains(WordBreakRule::AtArbitraryPositionWithinWords))
    {
      auto tryBreakingAtArbitraryPositionWithinWords = [&]() -> Maybe<PartialRun>
      {
        // Breaking is allowed within “words”: specifically, in addition to soft wrap opportunities allowed
        // for normal, any typographic letter units It does not affect rules governing the soft wrap
        // opportunities created by white space. Hyphenation is not applied.
        krys_debug_assert(!breakRules.Contains(WordBreakRule::AtHyphenationOpportunities));
        if (inlineTextItem.IsWhitespace())
        {
          // AtArbitraryPositionWithinWords does not affect the breaking opportunities around whitespace.
          return {};
        }

        if (!inlineTextItem.Length())
        {
          // Empty/single character text runs may be breakable based on style, but in practice we can't really
          // split them any further.
          return {};
        }

        if (candidateTextRun.IsOverflowingRun)
        {
          if (lineHasRoomForContent)
          {
            // Try to break the overflowing run mid-word.
            if (auto wordBreak = MidWordBreak(candidateRun, candidateTextRun.LogicalLeft, availableWidth))
            {
              return PartialRun {wordBreak->Length, wordBreak->LogicalWidth};
            }
          }
          if (CanBreakBefore(CodePointAt(inlineTextItem.Content(), 0), style.GetLineBreak()))
          {
            return PartialRun {};
          }
          // Since this is an overflowing content and we are allowed to break at arbitrary position, we really
          // ought to find a breaking position. Unless of course it's really an unbreakable content with
          // nothing but e.g. punctuation characters.
          // FIXME: This should be merged with the "let's keep the first character on the line" logic (see in
          // InlineContentBreaker::processOverflowingContent)
          auto firstBreakablePosition = [&]() -> Maybe<TextUtil::WordBreakLeft>
          {
            if (lineStatus.HasContent)
            {
              return {};
            }
            auto text = utf8_stringview {inlineTextItem.GetInlineTextBox().Content()};
            const size_t left = inlineTextItem.Start();
            auto right = left;
            while (right < inlineTextItem.End())
            {
              right = NextCodePointStart(text, right);
              if (right == inlineTextItem.End())
              {
                return {};
              }
              if (CanBreakBefore(CodePointAt(text, right), style.GetLineBreak()))
              {
                return TextUtil::WordBreakLeft {
                  right - left,
                  TextUtil::Width(inlineTextItem, style.GetFontCascade(), static_cast<uint32>(left),
                                  static_cast<uint32>(right), candidateTextRun.LogicalLeft)};
              }
            }
            return {};
          };
          if (auto wordBreak = firstBreakablePosition())
          {
            return PartialRun {wordBreak->Length, wordBreak->LogicalWidth};
          }
          return {};
        }

        // This is a non-overflowing content.
        krys_debug_assert(lineHasRoomForContent || !candidateRun.SpaceRequired());
        if (auto breakingPosition = LastValidBreakingPosition(runs, candidateTextRun.Index))
        {
          krys_debug_assert(*breakingPosition <= inlineTextItem.End());
          auto trailingLength = *breakingPosition - inlineTextItem.Start();
          auto startPosition = inlineTextItem.Start();
          auto endPosition = startPosition + static_cast<uint32>(trailingLength);
          return PartialRun {trailingLength, TextUtil::Width(inlineTextItem, fontCascade, startPosition,
                                                             endPosition, candidateTextRun.LogicalLeft)};
        }
        return {};
      };
      return tryBreakingAtArbitraryPositionWithinWords();
    }

    if (breakRules.Contains(WordBreakRule::AtHyphenationOpportunities))
    {
      auto tryBreakingAtHyphenationOpportunity = [&]() -> Maybe<PartialRun>
      {
        auto hyphenWidth = TextUtil::HyphenWidth(style);
        auto hyphenLocation = [&]() -> Maybe<size_t>
        {
          if (!candidateTextRun.IsOverflowingRun)
          {
            return LastHyphenPosition(inlineTextItem.Content(), style);
          }

          auto availableWidthExcludingHyphen = availableWidth - hyphenWidth;
          auto hasSomeRoomForContent =
            availableWidthExcludingHyphen > 0
            && EnoughWidthForHyphenation(availableWidthExcludingHyphen, fontCascade.Size());
          if (hasSomeRoomForContent && candidateRun.SpaceRequired())
          {
            auto leftSideLength =
              TextUtil::BreakWord(inlineTextItem, fontCascade, candidateRun.SpaceRequired(),
                                  availableWidthExcludingHyphen, candidateTextRun.LogicalLeft)
                .Length;
            if (auto position = HyphenPositionBefore(inlineTextItem.Content(), style, leftSideLength))
            {
              return position;
            }
          }
          return !lineStatus.HasContent && *FirstTextRunIndex(runs) == candidateTextRun.Index
                   ? FirstHyphenPosition(inlineTextItem.Content(), style)
                   : null;
        };

        if (auto position = hyphenLocation())
        {
          auto trailingPartialRunWidthWithHyphen = TextUtil::Width(
            inlineTextItem, fontCascade, inlineTextItem.Start(),
            inlineTextItem.Start() + static_cast<uint32>(*position), candidateTextRun.LogicalLeft);
          return PartialRun {*position, trailingPartialRunWidthWithHyphen, hyphenWidth};
        }
        return {};
      };
      if (auto partialRun = tryBreakingAtHyphenationOpportunity())
      {
        return partialRun;
      }
    }

    if (breakRules.Contains(WordBreakRule::AtArbitraryPosition))
    {
      auto tryBreakingAtArbitraryPosition = [&]() -> Maybe<PartialRun>
      {
        if (!inlineTextItem.Length())
        {
          // Empty text runs may be breakable based on style, but in practice we can't really split them any
          // further.
          return {};
        }
        if (!candidateTextRun.IsOverflowingRun)
        {
          // When the run can be split at arbitrary position let's just return the entire run when it is
          // intended to fit on the line. However the breaking properties only set rules for text content, so
          // let's check if this run is adjacent to another text run.
          krys_debug_assert(inlineTextItem.Length());
          // FIXME: We may need to check if the "next" text run is visually adjacent to this non-overflowing
          // run too (e.g. A<span style="border: 100px solid green;"></span>B)
          if (NextTextRunIndex(runs, candidateTextRun.Index))
          {
            // We are in-between text runs. It's okay to return the entire run triggering split at the very
            // right edge.
            auto trailingPartialRunWidth =
              TextUtil::Width(inlineTextItem, fontCascade, candidateTextRun.LogicalLeft);
            return PartialRun {inlineTextItem.Length(), trailingPartialRunWidth};
          }
          auto content = inlineTextItem.Content();
          auto lastCharacterStart = PreviousCodePointStart(content, content.size());
          if (lastCharacterStart)
          {
            auto startPosition = inlineTextItem.Start();
            auto endPosition = startPosition + static_cast<uint32>(lastCharacterStart);
            return PartialRun {lastCharacterStart,
                               TextUtil::Width(inlineTextItem, fontCascade, startPosition, endPosition,
                                               candidateTextRun.LogicalLeft)};
          }
          return {};
        }
        if (!lineHasRoomForContent)
        {
          // Fast path for cases when there's no room at all. The content is breakable but we don't have space
          // for it.
          return PartialRun {};
        }
        auto wordBreak = TextUtil::BreakWord(inlineTextItem, fontCascade, candidateRun.SpaceRequired(),
                                             availableWidth, candidateTextRun.LogicalLeft);
        return PartialRun {wordBreak.Length, wordBreak.LogicalWidth};
      };
      // With arbitrary breaking there's always a valid breaking position (even if it is before the first
      // position).
      return tryBreakingAtArbitraryPosition();
    }

    return {};
  }

  Maybe<InlineContentBreaker::OverflowingTextContent::BreakingPosition>
    InlineContentBreaker::TryBreakingOverflowingRun(
      const LineStatus &lineStatus, const ContinuousContent::RunList &runs, size_t overflowingRunIndex,
      InlineLayoutUnit nonOverflowingContentWidth) const noexcept
  {
    auto &overflowingRun = runs[overflowingRunIndex];
    krys_debug_assert(!overflowingRun.Item.IsOpaque());
    if (!IsBreakableRun(overflowingRun))
    {
      return {};
    }

    auto availableWidth = std::max(0.f, lineStatus.AvailableWidth - nonOverflowingContentWidth);
    auto partialOverflowingRun = TryBreakingTextRun(
      runs, {overflowingRunIndex, true, lineStatus.ContentLogicalRight + nonOverflowingContentWidth},
      availableWidth, lineStatus);
    if (!partialOverflowingRun)
    {
      return {};
    }
    if (partialOverflowingRun->Length)
    {
      return OverflowingTextContent::BreakingPosition {
        overflowingRunIndex,
        OverflowingTextContent::BreakingPosition::TrailingContent {false, partialOverflowingRun}};
    }
    // When the breaking position is at the beginning of the run, the trailing run is the previous one.
    if (auto trailingRunIndex = FindTrailingRunIndexBeforeBreakableRun(runs, overflowingRunIndex))
    {
      return OverflowingTextContent::BreakingPosition {
        *trailingRunIndex, OverflowingTextContent::BreakingPosition::TrailingContent {}};
    }
    // Sometimes we can't accommodate even the very first character.
    // Note that this is different from when there's no breakable run in this set.
    return OverflowingTextContent::BreakingPosition {};
  }

  Maybe<InlineContentBreaker::OverflowingTextContent::BreakingPosition>
    InlineContentBreaker::TryBreakingPreviousNonOverflowingRuns(
      const LineStatus &lineStatus, const ContinuousContent::RunList &runs, size_t overflowingRunIndex,
      InlineLayoutUnit nonOverflowingContentWidth) const noexcept
  {
    auto previousContentWidth = nonOverflowingContentWidth;
    for (auto index = overflowingRunIndex; index--;)
    {
      auto &run = runs[index];
      previousContentWidth -= run.SpaceRequired();
      if (!IsBreakableRun(run))
      {
        continue;
      }
      krys_debug_assert(run.Item.IsText());
      auto availableWidth = std::max(0.f, lineStatus.AvailableWidth - previousContentWidth);
      if (auto partialRun =
            TryBreakingTextRun(runs, {index, false, lineStatus.ContentLogicalRight + previousContentWidth},
                               availableWidth, lineStatus))
      {
        // We know this run fits, so if breaking is allowed on the run, it should return a non-empty left-side
        // since it's either at hyphen position or the entire run is returned.
        krys_debug_assert(partialRun->Length);
        auto runIsFullyAccommodated = partialRun->Length == Downcast<InlineTextItem>(run.Item).Length();
        if (runIsFullyAccommodated)
        {
          auto trailingRunIndex = [&]
          {
            // Try not break content at inline box boundary.
            // e.g. <span style="word-wrap:
            // break-word">fits_and_we_break_at_the_right_edge</span><span>overflows</span> we should forward
            // the breaking index to the closing inline box.
            // FIXME: We may wanna skip over the visually empty inline boxes only e.g. <span style="word-wrap:
            // break-word">fits_and_we_break_at_the_right_edge</span><span></span><span>overflows</span>
            auto trailingInlineBoxEndIndex = Maybe<size_t> {};
            for (auto candidateIndex = index + 1; candidateIndex <= overflowingRunIndex; ++candidateIndex)
            {
              auto &trailingInlineItem = runs[candidateIndex].Item;
              if (trailingInlineItem.IsInlineBoxEnd())
              {
                trailingInlineBoxEndIndex = candidateIndex;
              }
              if (!trailingInlineItem.IsInlineBoxStartOrEnd())
              {
                break;
              }
            }
            krys_debug_assert(!trailingInlineBoxEndIndex
                              || *trailingInlineBoxEndIndex <= overflowingRunIndex);
            return trailingInlineBoxEndIndex.value_or(index);
          };
          return OverflowingTextContent::BreakingPosition {
            trailingRunIndex(), OverflowingTextContent::BreakingPosition::TrailingContent {false}};
        }
        return OverflowingTextContent::BreakingPosition {
          index, OverflowingTextContent::BreakingPosition::TrailingContent {false, partialRun}};
      }
    }
    return {};
  }

  Maybe<InlineContentBreaker::OverflowingTextContent::BreakingPosition>
    InlineContentBreaker::TryBreakingNextOverflowingRuns(
      const LineStatus &lineStatus, const ContinuousContent::RunList &runs, size_t overflowingRunIndex,
      InlineLayoutUnit nonOverflowingContentWidth) const noexcept
  {
    auto nextContentWidth = nonOverflowingContentWidth + runs[overflowingRunIndex].SpaceRequired();
    for (auto index = overflowingRunIndex + 1; index < runs.size(); ++index)
    {
      auto &run = runs[index];
      if (!IsBreakableRun(run))
      {
        nextContentWidth += run.SpaceRequired();
        continue;
      }
      krys_debug_assert(run.Item.IsText());
      // At this point the available space is zero. Let's try the break these overflowing set of runs at the
      // earliest possible.
      if (auto partialRun = TryBreakingTextRun(
            runs, {index, true, lineStatus.ContentLogicalRight + nextContentWidth}, 0, lineStatus))
      {
        // <span>unbreakable_and_overflows<span style="word-break: break-all">breakable</span>
        // The partial run length could very well be 0 meaning the trailing run is actually the overflowing
        // run (see above in the example).
        if (partialRun->Length)
        {
          // We managed to break this text run mid content. It has to be either an arbitrary mid-word or a
          // hyphen break.
          return OverflowingTextContent::BreakingPosition {
            index, OverflowingTextContent::BreakingPosition::TrailingContent {true, partialRun}};
        }
        if (auto trailingRunIndex = FindTrailingRunIndexBeforeBreakableRun(runs, index))
        {
          // We may end up _before_ the overflowing run e.g.
          // <span></span><span style="border: 10px solid">breakable</span>
          // with 0px constraint, where while the second (non-empty) [inline box start] overflows, the
          // trailing run ends up being the first [inline box end] inline item.
          return OverflowingTextContent::BreakingPosition {
            *trailingRunIndex, OverflowingTextContent::BreakingPosition::TrailingContent {true}};
        }
        // This happens when the overflowing run is also the first run in this set, no trailing run.
        return OverflowingTextContent::BreakingPosition {overflowingRunIndex, {}};
      }
      nextContentWidth += run.SpaceRequired();
    }
    return {};
  }

  Maybe<InlineContentBreaker::OverflowingTextContent::BreakingPosition>
    InlineContentBreaker::TryHyphenationAcrossOverflowingInlineTextItems(
      const LineStatus &lineStatus, const ContinuousContent::RunList &runs,
      size_t overflowingRunIndex) const noexcept
  {
    if (runs.size() == 1)
    {
      return {};
    }

    auto &style = runs.front().Item.Style();
    if (!WordBreakBehavior(style, lineStatus.HasWrapOpportunityAtPreviousPosition)
           .Contains(WordBreakRule::AtHyphenationOpportunities))
    {
      return {};
    }

    // 1. concatenate adjacent text content
    // 2. find the last hyphen location before the overflowing position
    // 3. find the inline text item where the hyphen location is and compute the partial run width
    auto content = StringBuilder {};
    size_t overflowingRunStartPosition = 0;
    for (size_t index = 0; index < runs.size(); ++index)
    {
      auto &inlineItem = runs[index].Item;
      // FIXME: Maybe content across inline boxes should be hyphenated as well.
      if (inlineItem.IsOpaque())
      {
        continue;
      }
      if (!inlineItem.Style().FontCascadeEqual(style))
      {
        return {};
      }

      auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem);
      if (!inlineTextItem)
      {
        return {};
      }
      if (inlineTextItem->IsWhitespace())
      {
        return {};
      }
      content.Append(inlineTextItem->Content());
      overflowingRunStartPosition += index < overflowingRunIndex ? inlineTextItem->Length() : 0;
    }
    // Only non-whitespace text runs with same style.
    auto &fontCascade = style.GetFontCascade();
    auto hyphenWidth = TextUtil::HyphenWidth(style);
    auto availableWidthExcludingHyphen = lineStatus.AvailableWidth - hyphenWidth;
    if (availableWidthExcludingHyphen <= 0
        || !EnoughWidthForHyphenation(availableWidthExcludingHyphen, fontCascade.Size()))
    {
      return {};
    }

    auto &overflowingRun = runs[overflowingRunIndex];
    auto *textItem = DynamicDowncast<InlineTextItem>(overflowingRun.Item);
    krys_debug_assert(textItem);
    if (!textItem)
    {
      return {};
    }
    // Make sure we always hyphenate before the overflow.
    auto overflowPositionWithHyphen =
      TextUtil::BreakWord(*textItem, fontCascade, overflowingRun.SpaceRequired(),
                          availableWidthExcludingHyphen, lineStatus.ContentLogicalRight)
        .Length;
    auto hyphenLocation =
      HyphenPositionBefore(content.View(), style, overflowingRunStartPosition + overflowPositionWithHyphen);
    if (!hyphenLocation)
    {
      return {};
    }

    // hyphenLocation must be in or before the overflowing run.
    krys_debug_assert(*hyphenLocation <= overflowingRunStartPosition + overflowPositionWithHyphen);
    auto hyphenLocationWithinInlineTextItem = *hyphenLocation;
    size_t hyphenatedRunIndex = 0;
    for (; hyphenatedRunIndex <= overflowingRunIndex; ++hyphenatedRunIndex)
    {
      auto &inlineItem = runs[hyphenatedRunIndex].Item;
      if (inlineItem.IsOpaque())
      {
        continue;
      }
      auto &inlineTextItem = Downcast<InlineTextItem>(inlineItem);
      if (inlineTextItem.Length() >= hyphenLocationWithinInlineTextItem)
      {
        break;
      }
      hyphenLocationWithinInlineTextItem -= inlineTextItem.Length();
    }
    auto &hyphenatedlineTextItem = Downcast<InlineTextItem>(runs[hyphenatedRunIndex].Item);
    if (hyphenLocationWithinInlineTextItem > hyphenatedlineTextItem.Length())
    {
      krys_debug_assert(false);
      return {};
    }

    // Hyphen may be right at the run (end) boundary.
    auto partialRun = Maybe<InlineContentBreaker::PartialRun> {};
    if (hyphenLocationWithinInlineTextItem < hyphenatedlineTextItem.Length())
    {
      auto trailingPartialRunWidthWithHyphen = TextUtil::Width(
        hyphenatedlineTextItem, fontCascade, hyphenatedlineTextItem.Start(),
        hyphenatedlineTextItem.Start() + static_cast<uint32>(hyphenLocationWithinInlineTextItem),
        lineStatus.ContentLogicalRight);
      partialRun =
        PartialRun {hyphenLocationWithinInlineTextItem, trailingPartialRunWidthWithHyphen, hyphenWidth};
    }
    return OverflowingTextContent::BreakingPosition {
      hyphenatedRunIndex,
      OverflowingTextContent::BreakingPosition::TrailingContent {false, partialRun, hyphenWidth}};
  }

  InlineContentBreaker::OverflowingTextContent
    InlineContentBreaker::ProcessOverflowingContentWithText(const ContinuousContent &continuousContent,
                                                            const LineStatus &lineStatus) const noexcept
  {
    auto &runs = continuousContent.Runs();
    krys_debug_assert(!runs.empty());

    // Check where the overflow occurs and use the corresponding style to figure out the breaking behavior.
    // <span style="word-break: normal">first</span><span style="word-break: break-all">second</span><span
    // style="word-break: normal">third</span>

    // First find the overflowing run.
    auto nonOverflowingContentWidth = InlineLayoutUnit {};
    auto overflowingRunIndex = runs.size();
    for (size_t index = 0; index < runs.size(); ++index)
    {
      auto &run = runs[index];
      if (run.Item.IsOpaque())
      {
        continue;
      }
      auto runLogicalWidth = run.SpaceRequired();
      if (nonOverflowingContentWidth + runLogicalWidth > lineStatus.AvailableWidth)
      {
        overflowingRunIndex = index;
        break;
      }
      nonOverflowingContentWidth += runLogicalWidth;
    }
    if (overflowingRunIndex == runs.size())
    {
      // We have to have either an overflowing run or a soft hyphen.
      krys_debug_assert(continuousContent.HasTrailingSoftHyphen() && runs.size());
      return {runs.size() ? runs.size() - 1 : 0};
    }

    // Check first if we can actually break the overflowing run.
    if (auto breakingPosition =
          TryBreakingOverflowingRun(lineStatus, runs, overflowingRunIndex, nonOverflowingContentWidth))
    {
      return {overflowingRunIndex, breakingPosition};
    }

    auto &overflowingInlineItem = runs[overflowingRunIndex].Item;
    // In some cases we just can't break before certain overflowing runs due to content specific CSS rules,
    // e.g. line-break: after-white-space. This is in addition to having soft wrap opportunties only after the
    // whitespace. This is about not breaking at all before the whitespace content e.g. <div
    // style="line-break: after-white-space; word-wrap: break-word">before<span style="white-space: pre">
    // </span>after</div> "before" content is not breakable sine it is _before_ the overflowing whitespace
    // content.
    auto isBreakingAllowedBeforeOverflowingRun = [&]
    {
      auto *textItem = DynamicDowncast<InlineTextItem>(overflowingInlineItem);
      return !textItem || !textItem->IsWhitespace()
             || textItem->Style().GetLineBreak() != LineBreak::AfterWhiteSpace;
    }();
    if (isBreakingAllowedBeforeOverflowingRun)
    {
      // We did not manage to break the run that overflows the line.
      // Let's try to find a previous breaking position starting from the overflowing run. It surely fits.
      if (auto breakingPosition = TryBreakingPreviousNonOverflowingRuns(lineStatus, runs, overflowingRunIndex,
                                                                        nonOverflowingContentWidth))
      {
        return {overflowingRunIndex, breakingPosition};
      }
    }

    if (auto breakingPosition =
          TryHyphenationAcrossOverflowingInlineTextItems(lineStatus, runs, overflowingRunIndex))
    {
      return {overflowingRunIndex, breakingPosition};
    }

    // At this point we know that there's no breakable run all the way to the overflowing run.
    // Now we need to check if any run after the overflowing content can break.
    // e.g. <span>this_content_overflows_but_not_breakable<span><span style="word-break:
    // break-all">but_this_is_breakable</span>
    if (auto breakingPosition =
          TryBreakingNextOverflowingRuns(lineStatus, runs, overflowingRunIndex, nonOverflowingContentWidth))
    {
      return {overflowingRunIndex, breakingPosition};
    }

    // Give up, there's no breakable run in here.
    return {overflowingRunIndex};
  }

  EnumSet<InlineContentBreaker::WordBreakRule>
    InlineContentBreaker::WordBreakBehavior(const RenderStyle &style,
                                            bool hasWrapOpportunityAtPreviousPosition) const noexcept
  {
    // Disregard any prohibition against line breaks mandated by the word-break property.
    // The different wrapping opportunities must not be prioritized.
    // Note hyphenation is not applied.
    if (style.GetLineBreak() == LineBreak::Anywhere)
    {
      return {WordBreakRule::AtArbitraryPosition};
    }

    // Breaking is allowed within “words”.
    if (style.GetWordBreak() == WordBreak::BreakAll)
    {
      return {WordBreakRule::AtArbitraryPositionWithinWords};
    }

    auto includeHyphenationIfAllowed = [&](Maybe<InlineContentBreaker::WordBreakRule> wordBreakRule)
      -> EnumSet<InlineContentBreaker::WordBreakRule>
    {
      auto hyphenationIsAllowed = !_hyphenationIsDisabled && style.GetHyphens() == Hyphens::Auto
                                  && CanHyphenate(style.ComputedLocale().GetPlatform());
      if (hyphenationIsAllowed)
      {
        if (wordBreakRule)
        {
          return {*wordBreakRule, WordBreakRule::AtHyphenationOpportunities};
        }
        return {WordBreakRule::AtHyphenationOpportunities};
      }
      if (wordBreakRule)
      {
        return {*wordBreakRule};
      }
      return {};
    };

    // For compatibility with legacy content, the word-break property also supports a deprecated break-word
    // keyword. When specified, this has the same effect as word-break: normal and overflow-wrap: anywhere,
    // regardless of the actual value of the overflow-wrap property.
    if (style.GetWordBreak() == WordBreak::BreakWord && !hasWrapOpportunityAtPreviousPosition)
    {
      return includeHyphenationIfAllowed(WordBreakRule::AtArbitraryPosition);
    }
    // OverflowWrap::BreakWord/Anywhere An otherwise unbreakable sequence of characters may be broken at an
    // arbitrary point if there are no otherwise-acceptable break points in the line. Note that this applies
    // to content where CSS properties (e.g. WordBreak::KeepAll) make it unbreakable. Soft wrap opportunities
    // introduced by overflow-wrap/word-wrap: break-word are not considered when calculating min-content
    // intrinsic sizes.
    auto overflowWrapBreakWordIsApplicable = !IsMinimumInIntrinsicWidthMode();
    if (((overflowWrapBreakWordIsApplicable && style.GetOverflowWrap() == OverflowWrap::BreakWord)
         || style.GetOverflowWrap() == OverflowWrap::Anywhere)
        && !hasWrapOpportunityAtPreviousPosition)
    {
      return includeHyphenationIfAllowed(WordBreakRule::AtArbitraryPosition);
    }
    // Breaking is forbidden within “words”.
    if (style.GetWordBreak() == WordBreak::KeepAll)
    {
      return {};
    }
    return includeHyphenationIfAllowed({});
  }

  void
    InlineContentBreaker::ContinuousContent::SetTrailingSoftHyphenWidth(InlineLayoutUnit hyphenWidth) noexcept
  {
    _logicalWidth += hyphenWidth;
    _hasTrailingSoftHyphen = true;
  }

  void InlineContentBreaker::ContinuousContent::AppendToRunList(
    const InlineItem &inlineItem, const RenderStyle &style, InlineLayoutUnit offset,
    InlineLayoutUnit contentWidth, InlineLayoutUnit textSpacingAdjustment) noexcept
  {
    _runs.emplace_back(inlineItem, style, offset, contentWidth, textSpacingAdjustment);
    _logicalWidth = ClampTo<InlineLayoutUnit>(_logicalWidth + offset + contentWidth);
  }

  void InlineContentBreaker::ContinuousContent::ResetTrailingTrimmableContent() noexcept
  {
    if (!_leadingTrimmableWidth)
    {
      _leadingTrimmableWidth = _trailingTrimmableWidth;
    }
    _trailingTrimmableWidth = {};
    _isFullyTrimmable = false;
  }

  void InlineContentBreaker::ContinuousContent::Append(const InlineItem &inlineItem, const RenderStyle &style,
                                                       InlineLayoutUnit logicalWidth,
                                                       InlineLayoutUnit textSpacingAdjustment) noexcept
  {
    krys_debug_assert(inlineItem.IsAtomicInlineBox() || inlineItem.IsInlineBoxStartOrEnd()
                      || inlineItem.IsOpaque() || inlineItem.IsBlock());
    _isTextOnlyContent = false;
    _hasTrailingWordSeparator = _hasTrailingWordSeparator && !inlineItem.IsAtomicInlineBox();
    AppendToRunList(inlineItem, style, {}, logicalWidth, textSpacingAdjustment);
    if (inlineItem.IsAtomicInlineBox())
    {
      // Inline boxes (whitespace-> <span></span>) do not prevent the trailing content from getting
      // trimmed/hung but atomic inline level boxes do.
      ResetTrailingTrimmableContent();
    }
  }

  void InlineContentBreaker::ContinuousContent::AppendTextContent(const InlineTextItem &inlineTextItem,
                                                                  const RenderStyle &style,
                                                                  InlineLayoutUnit logicalWidth) noexcept
  {
    _hasTextContent = true;
    auto isAfterWordSeparator = _hasTrailingWordSeparator;
    _hasTrailingWordSeparator = inlineTextItem.IsWordSeparator();
    // https://www.w3.org/TR/css-text-4/#white-space-phase-2
    auto isTrailingHangingContent =
      inlineTextItem.IsWhitespace() && TextUtil::ShouldTrailingWhitespaceHang(style);
    if (isTrailingHangingContent)
    {
      SetHangingContentWidth(logicalWidth);
    }

    auto trimmableWidth = [&]() -> Maybe<InlineLayoutUnit>
    {
      if (isTrailingHangingContent)
      {
        return {};
      }
      if (inlineTextItem.IsFullyTrimmable() || inlineTextItem.IsQuirkNonBreakingSpace())
      {
        return logicalWidth;
      }
      return {};
    }();
    if (!trimmableWidth)
    {
      auto contentOffset = isAfterWordSeparator ? style.UsedWordSpacing() : 0.f;
      AppendToRunList(inlineTextItem, style, contentOffset, logicalWidth);
      if (contentOffset && IsFullyTrimmable())
      {
        // word-spacing offset gets trimmed together with the leading trimmable content.
        _leadingTrimmableWidth += contentOffset;
      }
      ResetTrailingTrimmableContent();
      return;
    }

    _isFullyTrimmable = _isFullyTrimmable || _runs.empty();
    krys_debug_assert(*trimmableWidth <= logicalWidth);
    auto isLeadingTrimmable = trimmableWidth && (!LogicalWidth() || IsFullyTrimmable());
    AppendToRunList(inlineTextItem, style, isAfterWordSeparator ? style.UsedWordSpacing() : 0.f,
                    logicalWidth);
    if (isLeadingTrimmable)
    {
      krys_debug_assert(!_trailingTrimmableWidth);
      _leadingTrimmableWidth += *trimmableWidth;
      return;
    }
    _trailingTrimmableWidth =
      *trimmableWidth == logicalWidth ? _trailingTrimmableWidth + logicalWidth : *trimmableWidth;
  }

  void InlineContentBreaker::ContinuousContent::Reset() noexcept
  {
    _logicalWidth = {};
    _leadingTrimmableWidth = {};
    _trailingTrimmableWidth = {};
    _hangingContentWidth = {};
    _minimumRequiredWidth = {};
    _runs.clear();
    _hasTextContent = false;
    _isTextOnlyContent = true;
    _isFullyTrimmable = false;
    _hasTrailingWordSeparator = false;
    _hasTrailingSoftHyphen = false;
    _hasShapedContent = false;
  }
}
