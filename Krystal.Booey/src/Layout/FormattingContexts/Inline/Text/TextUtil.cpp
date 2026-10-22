#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Core/CharacterProperties.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/Core/UnicodeHelpers.hpp"
#include "Krystal.Booey/Graphics/ComplexTextController.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/BreakablePositions.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Text/TabSize.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <cmath>
#include <numeric>
#include <unicode/uchar.h>

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD text::DecodedCodePoint DecodeAt(utf8_stringview text, size_t position) noexcept
    {
      return text::DecodeUTF8(Span<const char8>(text.data() + position, text.size() - position));
    }

    KRYS_NODISCARD inline InlineLayoutUnit SpaceWidth(const FontCascade &fontCascade,
                                                      bool canUseSimplifiedContentMeasuring) noexcept
    {
      if (canUseSimplifiedContentMeasuring)
      {
        return fontCascade.PrimaryFont()->SpaceWidth();
      }
      return fontCascade.WidthOfSpaceString();
    }

    void FallbackFontsForRun(TextUtil::FallbackFontList &fallbackFonts, const FontCascade &fontCascade,
                             const TextRun &run) noexcept
    {
      auto isRTL = run.Rtl();
      auto isSmallCaps = fontCascade.IsSmallCaps();
      Ref<const Font> primaryFont = fontCascade.PrimaryFont();

      for (auto it = text::CodePointIterator(run.Text()); !it.AtEnd(); ++it)
      {
        auto addFallbackFontForCharacterIfApplicable = [&](char32 character)
        {
          if (isSmallCaps)
          {
            character = static_cast<char32>(u_toupper(static_cast<UChar32>(character)));
          }

          auto glyphData = fontCascade.GlyphDataForCharacter(character, isRTL);
          if (glyphData.Glyph && glyphData.Font && glyphData.Font.get() != primaryFont.get())
          {
            auto isNonSpacingMark = U_MASK(u_charType(static_cast<UChar32>(character))) & U_GC_MN_MASK;

            // https://drafts.csswg.org/css-text-3/#white-space-processing
            // "Unsupported Default_ignorable characters must be ignored for text rendering."
            auto isIgnored = IsDefaultIgnorableCodePoint(character);

            // If we include the synthetic bold expansion, then even zero-width glyphs will have their fonts
            // added.
            if (isNonSpacingMark
                || glyphData.Font->WidthForGlyph(glyphData.Glyph, Font::SyntheticBoldInclusion::Exclude))
            {
              if (!isIgnored)
              {
                fallbackFonts.insert(ShareRefPtr(glyphData.Font.get()));
              }
            }
          }
        };
        addFallbackFontForCharacterIfApplicable(*it);
      }
    }

    KRYS_NODISCARD TextUtil::EnclosingAscentDescent
      EnclosingGlyphBoundsForRun(const FontCascade &fontCascade, bool isRTL, utf8_stringview content) noexcept
    {
      auto enclosingAscent = Maybe<InlineLayoutUnit> {};
      auto enclosingDescent = Maybe<InlineLayoutUnit> {};
      auto isSmallCaps = fontCascade.IsSmallCaps();
      Ref<const Font> primaryFont = fontCascade.PrimaryFont();

      for (auto it = text::CodePointIterator(content); !it.AtEnd(); ++it)
      {
        auto computeTopAndBottomForCharacter = [&](char32 character)
        {
          if (isSmallCaps)
          {
            character = static_cast<char32>(u_toupper(static_cast<UChar32>(character)));
          }

          auto glyphData = fontCascade.GlyphDataForCharacter(character, isRTL);
          const Font &font = glyphData.Font ? *glyphData.Font : *primaryFont;
          auto bounds = font.BoundsForGlyph(glyphData.Glyph);

          enclosingAscent = std::min(enclosingAscent.value_or(bounds.Y()), bounds.Y());
          enclosingDescent = std::max(enclosingDescent.value_or(bounds.MaxY()), bounds.MaxY());
        };
        computeTopAndBottomForCharacter(*it);
      }
      return {enclosingAscent.value_or(0.f), enclosingDescent.value_or(0.f)};
    }

    // True if the character may need the Bidi reordering. If false, the
    // `Bidi_Class` of `ch` isn't `R`, `AL`, nor Bidi controls.
    // https://util.unicode.org/UnicodeJsps/list-unicodeset.jsp?a=%5B%5B%3Abc%3DR%3A%5D%5B%3Abc%3DAL%3A%5D%5D&g=bc
    // https://util.unicode.org/UnicodeJsps/list-unicodeset.jsp?a=[:Bidi_C:]
    KRYS_NODISCARD bool MayBeBidiRTL(char32 ch) noexcept
    {
      if (ch < 0x0590)
      {
        return false;
      }
      // General Punctuation such as curly quotes.
      if (ch >= 0x2010 && ch <= 0x2029)
      {
        return false;
      }
      // CJK etc., up to Surrogate Pairs.
      if (ch >= 0x206A && ch <= 0xD7FF)
      {
        return false;
      }
      // Common in CJK.
      if (ch >= 0xFF00 && ch <= 0xFFFF)
      {
        return false;
      }
      return true;
    }
  }

  InlineLayoutUnit
    TextUtil::Width(const InlineTextBox &inlineTextBox, const FontCascade &fontCascade, uint32 from,
                    uint32 to, InlineLayoutUnit contentLogicalLeft,
                    UseTrailingWhitespaceMeasuringOptimization useTrailingWhitespaceMeasuringOptimization,
                    text_spacing::SpacingState spacingState) noexcept
  {
    if (from == to)
    {
      return 0;
    }

    if (inlineTextBox.IsCombined())
    {
      return fontCascade.Size();
    }

    auto text = utf8_stringview {inlineTextBox.Content()};
    krys_debug_assert(to <= text.length());
    auto hasKerningOrLigatures = fontCascade.EnableKerning() || fontCascade.RequiresShaping();
    // The "non-whitespace" + "whitespace" pattern is very common for inline content and since most of the
    // "non-whitespace" runs end up with their "whitespace" pair on the line (notable exception is when
    // trailing whitespace is trimmed). Including the trailing whitespace here enables us to cut the number of
    // text measures when placing content on the line.
    auto extendedMeasuring =
      useTrailingWhitespaceMeasuringOptimization == UseTrailingWhitespaceMeasuringOptimization::Yes
      && hasKerningOrLigatures && to < text.length() && text[to] == u8' ';
    if (extendedMeasuring)
    {
      ++to;
    }
    auto width = 0.f;
    auto useSimplifiedContentMeasuring = inlineTextBox.CanUseSimplifiedContentMeasuring();
    auto view = text.substr(from, to - from);
    if (useSimplifiedContentMeasuring)
    {
      if (fontCascade.CanTakeFixedPitchFastContentMeasuring())
      {
        width =
          fontCascade.WidthForSimpleTextWithFixedPitch(view, inlineTextBox.Style().CollapseWhiteSpace());
      }
      else
      {
        width = fontCascade.WidthForTextUsingSimplifiedMeasuring(view);
      }
    }
    else
    {
      auto &style = inlineTextBox.Style();
      auto directionalOverride = IsOverride(style.GetUnicodeBidi());
      auto run = TextRun(view, contentLogicalLeft, {}, ExpansionBehavior::DefaultBehavior(),
                         directionalOverride ? style.GetWritingMode().BidiDirection() : TextDirection::LTR,
                         directionalOverride);
      if (!style.CollapseWhiteSpace() && !style.GetTabSize().IsZero())
      {
        run.SetTabSize(true, style::ToPlatform<style::TabSize> {}(style.GetTabSize()));
      }
      // FIXME: consider moving this to TextRun ctor
      run.SetTextSpacingState(spacingState);
      width = fontCascade.Width(run);
    }

    if (extendedMeasuring)
    {
      width -= (SpaceWidth(fontCascade, useSimplifiedContentMeasuring) + fontCascade.WordSpacing());
    }

    if (std::isnan(width) || std::isinf(width)) KRYS_UNLIKELY
    {
      return std::isnan(width) ? 0.0f : MaxInlineLayoutUnit();
    }
    return std::max(0.f, width);
  }

  InlineLayoutUnit TextUtil::Width(const InlineTextItem &inlineTextItem, const FontCascade &fontCascade,
                                   InlineLayoutUnit contentLogicalLeft) noexcept
  {
    return TextUtil::Width(inlineTextItem, fontCascade, inlineTextItem.Start(), inlineTextItem.End(),
                           contentLogicalLeft);
  }

  InlineLayoutUnit
    TextUtil::Width(const InlineTextItem &inlineTextItem, const FontCascade &fontCascade, uint32 from,
                    uint32 to, InlineLayoutUnit contentLogicalLeft,
                    UseTrailingWhitespaceMeasuringOptimization useTrailingWhitespaceMeasuringOptimization,
                    text_spacing::SpacingState spacingState) noexcept
  {
    krys_assert(from >= inlineTextItem.Start());
    krys_assert(to <= inlineTextItem.End());

    if (inlineTextItem.IsWhitespace())
    {
      auto &inlineTextBox = inlineTextItem.GetInlineTextBox();
      auto useSimplifiedContentMeasuring = inlineTextBox.CanUseSimplifiedContentMeasuring();
      // Collapsible whitespace is measured as a single space. Preserved whitespace is measured as is: it may
      // be a tab, whose width depends on its position.
      auto singleWhiteSpace = !TextUtil::ShouldPreserveSpacesAndTabs(inlineTextBox);

      if (singleWhiteSpace)
      {
        auto width = SpaceWidth(fontCascade, useSimplifiedContentMeasuring);
        if (std::isnan(width) || std::isinf(width)) KRYS_UNLIKELY
        {
          return std::isnan(width) ? 0.0f : MaxInlineLayoutUnit();
        }
        return std::max(0.f, width);
      }
    }
    return Width(inlineTextItem.GetInlineTextBox(), fontCascade, from, to, contentLogicalLeft,
                 useTrailingWhitespaceMeasuringOptimization, spacingState);
  }

  InlineLayoutUnit TextUtil::TrailingWhitespaceWidth(const InlineTextBox &inlineTextBox,
                                                     const FontCascade &fontCascade, size_t startPosition,
                                                     size_t endPosition) noexcept
  {
    krys_debug_assert(endPosition > startPosition + 1);
    krys_debug_assert(inlineTextBox.Content()[endPosition - 1] == u8' ');
    return Width(inlineTextBox, fontCascade, static_cast<uint32>(startPosition),
                 static_cast<uint32>(endPosition), {}, UseTrailingWhitespaceMeasuringOptimization::Yes)
           - Width(inlineTextBox, fontCascade, static_cast<uint32>(startPosition),
                   static_cast<uint32>(endPosition - 1), {}, UseTrailingWhitespaceMeasuringOptimization::No);
  }

  TextUtil::FallbackFontList TextUtil::FallbackFontsForText(utf8_stringview textContent,
                                                            const RenderStyle &style,
                                                            IncludeHyphen includeHyphen) noexcept
  {
    TextUtil::FallbackFontList fallbackFonts;

    auto collectFallbackFonts = [&](const TextRun &textRun)
    {
      if (textRun.Text().empty())
      {
        return;
      }
      FallbackFontsForRun(fallbackFonts, style.GetFontCascade(), textRun);
    };

    if (includeHyphen == IncludeHyphen::Yes)
    {
      collectFallbackFonts(TextRun(style.HyphenString().View(), {}, {}, ExpansionBehavior::DefaultBehavior(),
                                   style.GetWritingMode().BidiDirection()));
    }
    collectFallbackFonts(TextRun(textContent, {}, {}, ExpansionBehavior::DefaultBehavior(),
                                 style.GetWritingMode().BidiDirection()));
    return fallbackFonts;
  }

  TextUtil::EnclosingAscentDescent TextUtil::EnclosingGlyphBoundsForText(
    utf8_stringview textContent, const RenderStyle &style,
    ShouldUseSimpleGlyphOverflowCodePath shouldUseSimpleGlyphOverflowCodePath) noexcept
  {
    if (textContent.empty())
    {
      return {};
    }

    if (shouldUseSimpleGlyphOverflowCodePath == ShouldUseSimpleGlyphOverflowCodePath::No)
    {
      auto overflow =
        ComplexTextController::EnclosingGlyphBoundsForTextRun(style.GetFontCascade(), TextRun(textContent));
      return {overflow.first, overflow.second};
    }

    return EnclosingGlyphBoundsForRun(style.GetFontCascade(), style.GetWritingMode().IsBidiRTL(),
                                      textContent);
  }

  TextUtil::WordBreakLeft TextUtil::BreakWord(const InlineTextItem &inlineTextItem,
                                              const FontCascade &fontCascade, InlineLayoutUnit textWidth,
                                              InlineLayoutUnit availableWidth,
                                              InlineLayoutUnit contentLogicalLeft) noexcept
  {
    return BreakWord(inlineTextItem.GetInlineTextBox(), inlineTextItem.Start(), inlineTextItem.Length(),
                     textWidth, availableWidth, contentLogicalLeft, fontCascade);
  }

  TextUtil::WordBreakLeft TextUtil::BreakWord(const InlineTextBox &inlineTextBox, size_t startPosition,
                                              size_t length, InlineLayoutUnit textWidth,
                                              InlineLayoutUnit availableWidth,
                                              InlineLayoutUnit contentLogicalLeft,
                                              const FontCascade &fontCascade) noexcept
  {
    krys_debug_assert(availableWidth >= 0);
    krys_debug_assert(length);
    auto text = utf8_stringview {inlineTextBox.Content()};

    if (!textWidth) KRYS_UNLIKELY
    {
      krys_debug_assert(false);
      return {};
    }

    auto width = [&](size_t from, size_t to)
    {
      return TextUtil::Width(inlineTextBox, fontCascade, static_cast<uint32>(from), static_cast<uint32>(to),
                             contentLogicalLeft);
    };

    if (inlineTextBox.CanUseSimpleFontCodePath())
    {
      auto findBreakingPositionInSimpleText = [&]
      {
        auto userPerceivedCharacterBoundaryAlignedIndex = [&](size_t index) -> size_t
        {
          auto alignedStartIndex = index;
          while (alignedStartIndex > startPosition && alignedStartIndex < text.size()
                 && text::IsTrailingUTF8(text[alignedStartIndex]))
          {
            --alignedStartIndex;
          }
          krys_debug_assert(alignedStartIndex >= startPosition);
          return alignedStartIndex;
        };

        auto trySimplifiedBreakingPosition = [&](size_t start) -> Maybe<WordBreakLeft>
        {
          auto mayUseSimplifiedBreakingPositionForFixedPitch =
            fontCascade.IsFixedPitch() && inlineTextBox.CanUseSimplifiedContentMeasuring();
          if (!mayUseSimplifiedBreakingPositionForFixedPitch)
          {
            return {};
          }
          // FIXME: Check if we could bring webkit.org/b/221581 back for system monospace fonts.
          auto monospaceCharacterWidth = fontCascade.WidthOfSpaceString();
          auto estimatedCharacterCount =
            static_cast<size_t>(std::floor(availableWidth / monospaceCharacterWidth));
          auto end = userPerceivedCharacterBoundaryAlignedIndex(
            std::min(start + estimatedCharacterCount, start + length - 1));
          auto underflowWidth = width(start, end);
          if (underflowWidth > availableWidth || underflowWidth + monospaceCharacterWidth < availableWidth)
          {
            // This does not look like a real fixed pitch font. Let's just fall back to regular bisect.
            // In some edge cases (float precision) using monospaceCharacterWidth here may produce an
            // incorrect off-by-one visual overflow.
            return {};
          }
          return WordBreakLeft {end - start, underflowWidth};
        };
        if (auto leftSide = trySimplifiedBreakingPosition(startPosition))
        {
          return *leftSide;
        }

        auto tryEstimateBreakingPosition = [&](size_t start) -> Maybe<WordBreakLeft>
        {
          // Determine if the breaking position can be found within the range of [-1, 1] by using the average
          // width of the characters.
          auto averageCharacterWidth = InlineLayoutUnit {textWidth / static_cast<float>(length)};
          auto candidateLength = static_cast<size_t>(availableWidth / averageCharacterWidth);
          auto candidateEnd = userPerceivedCharacterBoundaryAlignedIndex(start + candidateLength);
          if (candidateEnd <= start || candidateEnd >= start + length)
          {
            return {};
          }
          auto contentWidthAtCandidatePosition = width(start, candidateEnd);
          if (contentWidthAtCandidatePosition == availableWidth)
          {
            return WordBreakLeft {candidateEnd - start, contentWidthAtCandidatePosition};
          }

          if (contentWidthAtCandidatePosition > availableWidth)
          {
            // Overshot.
            if (auto adjustedCandidateEnd = userPerceivedCharacterBoundaryAlignedIndex(candidateEnd - 1);
                adjustedCandidateEnd > start)
            {
              auto adjustedContentWidth = width(start, adjustedCandidateEnd);
              if (adjustedContentWidth <= availableWidth)
              {
                return WordBreakLeft {adjustedCandidateEnd - start, adjustedContentWidth};
              }
            }
          }
          else if (auto adjustedCandidateEnd = candidateEnd + DecodeAt(text, candidateEnd).Length;
                   adjustedCandidateEnd < start + length)
          {
            // Undershot.
            auto adjustedContentWidth = width(start, adjustedCandidateEnd);
            if (adjustedContentWidth > availableWidth)
            {
              return WordBreakLeft {candidateEnd - start, contentWidthAtCandidatePosition};
            }
            if (adjustedContentWidth == availableWidth)
            {
              return WordBreakLeft {adjustedCandidateEnd - start, adjustedContentWidth};
            }
          }
          return {};
        };
        if (auto leftSide = tryEstimateBreakingPosition(startPosition))
        {
          return *leftSide;
        }

        auto nextUserPerceivedCharacterIndex = [&](size_t index) -> size_t
        {
          return std::min(index + DecodeAt(text, index).Length, startPosition + length);
        };

        auto left = startPosition;
        auto right = left + length - 1;
        // Pathological case of (extremely)long string and narrow lines.
        // Adjust the range so that we can pick a reasonable midpoint.
        auto averageCharacterWidth = InlineLayoutUnit {textWidth / static_cast<float>(length)};
        // Overshot the midpoint so that biscection starts at the left side of the content.
        auto startOffset = static_cast<size_t>(2 * availableWidth / averageCharacterWidth);
        right = userPerceivedCharacterBoundaryAlignedIndex(std::min(left + startOffset, right));
        // Preserve the left width for the final split position so that we don't need to remeasure the left
        // side again.
        auto leftSideWidth = InlineLayoutUnit {0};
        while (left < right)
        {
          auto middle = userPerceivedCharacterBoundaryAlignedIndex(std::midpoint(left, right));
          krys_debug_assert(middle >= left && middle < right);
          auto endOfMiddleCharacter = nextUserPerceivedCharacterIndex(middle);
          auto middleWidth = width(startPosition, endOfMiddleCharacter);
          if (middleWidth < availableWidth)
          {
            left = endOfMiddleCharacter;
            leftSideWidth = middleWidth;
          }
          else if (middleWidth > availableWidth)
          {
            right = middle;
          }
          else
          {
            right = endOfMiddleCharacter;
            leftSideWidth = middleWidth;
            break;
          }
        }
        krys_assert(right >= startPosition);
        return WordBreakLeft {right - startPosition, leftSideWidth};
      };
      return findBreakingPositionInSimpleText();
    }

    auto graphemeClusterIterator = NonSharedCharacterBreakIterator {text.substr(startPosition, length)};
    auto leftSide = TextUtil::WordBreakLeft {};
    for (auto clusterStartPosition = ubrk_next(graphemeClusterIterator); clusterStartPosition != UBRK_DONE;
         clusterStartPosition = ubrk_next(graphemeClusterIterator))
    {
      auto clusterWidth = width(startPosition, startPosition + static_cast<size_t>(clusterStartPosition));
      if (clusterWidth > availableWidth)
      {
        return leftSide;
      }
      leftSide = {static_cast<size_t>(clusterStartPosition), clusterWidth};
    }
    // This content is not supposed to fit availableWidth.
    krys_debug_assert(false);
    return leftSide;
  }

  bool TextUtil::MayBreakInBetween(const InlineTextItem &previousInlineItem,
                                   const InlineTextItem &nextInlineItem) noexcept
  {
    // Check if these 2 adjacent non-whitespace inline items are connected at a breakable position.
    krys_debug_assert(!previousInlineItem.IsWhitespace() && !nextInlineItem.IsWhitespace());
    return MayBreakInBetween(previousInlineItem.GetInlineTextBox().Content(), previousInlineItem.Style(),
                             nextInlineItem.GetInlineTextBox().Content(), nextInlineItem.Style());
  }

  bool TextUtil::MayBreakInBetween(utf8_stringview previousContent, const RenderStyle &previousContentStyle,
                                   utf8_stringview nextContent, const RenderStyle &nextContentStyle) noexcept
  {
    // Now we need to collect at least 3 adjacent characters to be able to make a decision whether the
    // previous text item ends with breaking opportunity. [ex-][ample] <- second to last[x] last[-] current[a]
    // We need at least 1 character in the current inline text item and 2 more from previous inline items.
    auto lineBreakIteratorFactory =
      CachedLineBreakIteratorFactory {nextContent, nextContentStyle.ComputedLocale().GetPlatform(),
                                      TextUtil::LineBreakIteratorMode(nextContentStyle.GetLineBreak()),
                                      TextUtil::GetContentAnalysis(nextContentStyle.GetWordBreak())};
    // FIXME: We should look into the entire uncommitted content for more text context.
    char32 lastCharacter = 0;
    char32 secondToLastCharacter = 0;
    if (!previousContent.empty())
    {
      auto lastCharacterStart = text::PreviousCodePointStart(previousContent, previousContent.size());
      lastCharacter = DecodeAt(previousContent, lastCharacterStart).CodePoint;
      if (lastCharacterStart)
      {
        secondToLastCharacter =
          DecodeAt(previousContent, text::PreviousCodePointStart(previousContent, lastCharacterStart))
            .CodePoint;
      }
    }
    if (lastCharacter == text::SoftHyphen && previousContentStyle.GetHyphens() == Hyphens::None)
    {
      return false;
    }
    lineBreakIteratorFactory.GetPriorContext().Set({secondToLastCharacter, lastCharacter});
    // Now check if we can break right at the inline item boundary.
    // With the [ex-ample], findNextBreakablePosition should return the startPosition (0).
    // FIXME: Check if there's a more correct way of finding breaking opportunities.
    return !FindNextBreakablePosition(lineBreakIteratorFactory, 0, nextContentStyle);
  }

  uint32 TextUtil::FindNextBreakablePosition(CachedLineBreakIteratorFactory &lineBreakIteratorFactory,
                                             uint32 startPosition, const RenderStyle &style) noexcept
  {
    using LineBreakRules = BreakablePositions::LineBreakRules;
    using WordBreakBehavior = BreakablePositions::WordBreakBehavior;
    using NoBreakSpaceBehavior = BreakablePositions::NoBreakSpaceBehavior;

    // No-break spaces only break with `nbsp-mode: space`, which isn't supported.
    auto wordBreak = style.GetWordBreak();
    if (wordBreak == WordBreak::KeepAll)
    {
      return BreakablePositions::Next<LineBreakRules::Special, WordBreakBehavior::KeepAll,
                                      NoBreakSpaceBehavior::Normal>(lineBreakIteratorFactory, startPosition);
    }

    if (wordBreak == WordBreak::AutoPhrase)
    {
      return BreakablePositions::Next<LineBreakRules::Special, WordBreakBehavior::AutoPhrase,
                                      NoBreakSpaceBehavior::Normal>(lineBreakIteratorFactory, startPosition);
    }

    if (lineBreakIteratorFactory.Mode() == TextBreakLineBehavior::Default)
    {
      return BreakablePositions::Next<LineBreakRules::Normal, WordBreakBehavior::Normal,
                                      NoBreakSpaceBehavior::Normal>(lineBreakIteratorFactory, startPosition);
    }

    return BreakablePositions::Next<LineBreakRules::Special, WordBreakBehavior::Normal,
                                    NoBreakSpaceBehavior::Normal>(lineBreakIteratorFactory, startPosition);
  }

  bool TextUtil::ShouldPreserveSpacesAndTabs(const Box &layoutBox) noexcept
  {
    // https://www.w3.org/TR/css-text-4/#white-space-collapsing
    auto whitespaceCollapse = layoutBox.Style().GetWhiteSpaceCollapse();
    return whitespaceCollapse == WhiteSpaceCollapse::Preserve
           || whitespaceCollapse == WhiteSpaceCollapse::BreakSpaces;
  }

  bool TextUtil::ShouldPreserveNewline(const Box &layoutBox) noexcept
  {
    // https://www.w3.org/TR/css-text-4/#white-space-collapsing
    auto whitespaceCollapse = layoutBox.Style().GetWhiteSpaceCollapse();
    return whitespaceCollapse == WhiteSpaceCollapse::Preserve
           || whitespaceCollapse == WhiteSpaceCollapse::PreserveBreaks
           || whitespaceCollapse == WhiteSpaceCollapse::BreakSpaces;
  }

  bool TextUtil::IsWrappingAllowed(const RenderStyle &style) noexcept
  {
    // https://www.w3.org/TR/css-text-4/#text-wrap
    return style.GetTextWrapMode() != TextWrapMode::NoWrap;
  }

  bool TextUtil::ShouldTrailingWhitespaceHang(const RenderStyle &style) noexcept
  {
    // https://www.w3.org/TR/css-text-4/#white-space-phase-2
    return style.GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve
           && style.GetTextWrapMode() != TextWrapMode::NoWrap;
  }

  TextBreakLineBehavior TextUtil::LineBreakIteratorMode(LineBreak lineBreak) noexcept
  {
    switch (lineBreak)
    {
      case LineBreak::Auto:
      case LineBreak::AfterWhiteSpace:
      case LineBreak::Anywhere:        return TextBreakLineBehavior::Default;
      case LineBreak::Loose:           return TextBreakLineBehavior::Loose;
      case LineBreak::Normal:          return TextBreakLineBehavior::Normal;
      case LineBreak::Strict:          return TextBreakLineBehavior::Strict;
    }
    krys_debug_assert(false);
    return TextBreakLineBehavior::Default;
  }

  TextBreakIterator::ContentAnalysis TextUtil::GetContentAnalysis(WordBreak wordBreak) noexcept
  {
    switch (wordBreak)
    {
      case WordBreak::Normal:
      case WordBreak::BreakAll:
      case WordBreak::KeepAll:
      case WordBreak::BreakWord:  return TextBreakIterator::ContentAnalysis::Mechanical;
      case WordBreak::AutoPhrase: return TextBreakIterator::ContentAnalysis::Linguistic;
    }
    return TextBreakIterator::ContentAnalysis::Mechanical;
  }

  bool TextUtil::IsStrongDirectionalityCharacter(char32 character) noexcept
  {
    if (!MayBeBidiRTL(character))
    {
      return false;
    }

    auto bidiCategory = u_charDirection(static_cast<UChar32>(character));
    return bidiCategory == U_RIGHT_TO_LEFT || bidiCategory == U_RIGHT_TO_LEFT_ARABIC
           || bidiCategory == U_RIGHT_TO_LEFT_EMBEDDING || bidiCategory == U_RIGHT_TO_LEFT_OVERRIDE
           || bidiCategory == U_LEFT_TO_RIGHT_EMBEDDING || bidiCategory == U_LEFT_TO_RIGHT_OVERRIDE
           || bidiCategory == U_POP_DIRECTIONAL_FORMAT;
  }

  bool TextUtil::ContainsStrongDirectionalityText(utf8_stringview text) noexcept
  {
    for (auto it = text::CodePointIterator(text); !it.AtEnd(); ++it)
    {
      if (IsStrongDirectionalityCharacter(*it))
      {
        return true;
      }
    }

    return false;
  }

  size_t TextUtil::FirstUserPerceivedCharacterLength(const InlineTextBox &inlineTextBox, size_t startPosition,
                                                     size_t length) noexcept
  {
    auto textContent = utf8_stringview {inlineTextBox.Content()};
    krys_assert(!textContent.empty());

    if (inlineTextBox.CanUseSimpleFontCodePath())
    {
      auto endOfCodePoint = startPosition + DecodeAt(textContent, startPosition).Length;
      krys_debug_assert(endOfCodePoint > startPosition);
      return endOfCodePoint - startPosition;
    }
    auto graphemeClustersIterator = NonSharedCharacterBreakIterator {textContent};
    auto nextPosition = ubrk_following(graphemeClustersIterator, static_cast<int32_t>(startPosition));
    if (nextPosition == UBRK_DONE)
    {
      return length;
    }
    return static_cast<size_t>(nextPosition) - startPosition;
  }

  size_t TextUtil::FirstUserPerceivedCharacterLength(const InlineTextItem &inlineTextItem) noexcept
  {
    auto length = FirstUserPerceivedCharacterLength(inlineTextItem.GetInlineTextBox(), inlineTextItem.Start(),
                                                    inlineTextItem.Length());
    return std::min<size_t>(inlineTextItem.Length(), length);
  }

  TextDirection TextUtil::DirectionForTextContent(utf8_stringview content) noexcept
  {
    return BaseTextDirection(content).value_or(TextDirection::LTR);
  }

  const StringAtom &TextUtil::EllipsisTextInInlineDirection(bool isHorizontal) noexcept
  {
    if (isHorizontal)
    {
      static const StringAtom horizontalEllipsisStr {u8"\u2026"};
      return horizontalEllipsisStr;
    }
    static const StringAtom verticalEllipsisStr {u8"\u22EE"};
    return verticalEllipsisStr;
  }

  InlineLayoutUnit TextUtil::HyphenWidth(const RenderStyle &style) noexcept
  {
    return std::max(0.f, style.GetFontCascade().Width(style.HyphenString().View()));
  }

  bool TextUtil::HasHangablePunctuationStart(const InlineTextItem &inlineTextItem,
                                             const RenderStyle &style) noexcept
  {
    if (!inlineTextItem.Length()
        || !style.GetHangingPunctuation().Contains(style::HangingPunctuationValue::First))
    {
      return false;
    }
    auto leadingCharacter = DecodeAt(inlineTextItem.Content(), 0).CodePoint;
    return U_GET_GC_MASK(static_cast<UChar32>(leadingCharacter))
           & (U_GC_PS_MASK | U_GC_PI_MASK | U_GC_PF_MASK);
  }

  float TextUtil::HangablePunctuationStartWidth(const InlineTextItem &inlineTextItem,
                                                const RenderStyle &style) noexcept
  {
    if (!HasHangablePunctuationStart(inlineTextItem, style))
    {
      return {};
    }
    krys_debug_assert(inlineTextItem.Length());
    auto leadingPosition = inlineTextItem.Start();
    auto leadingLength = DecodeAt(inlineTextItem.Content(), 0).Length;
    return Width(inlineTextItem, style.GetFontCascade(), leadingPosition, leadingPosition + leadingLength,
                 {});
  }

  bool TextUtil::HasHangablePunctuationEnd(const InlineTextItem &inlineTextItem,
                                           const RenderStyle &style) noexcept
  {
    if (!inlineTextItem.Length()
        || !style.GetHangingPunctuation().Contains(style::HangingPunctuationValue::Last))
    {
      return false;
    }
    auto content = inlineTextItem.Content();
    auto trailingCharacter =
      DecodeAt(content, text::PreviousCodePointStart(content, content.size())).CodePoint;
    return U_GET_GC_MASK(static_cast<UChar32>(trailingCharacter))
           & (U_GC_PE_MASK | U_GC_PI_MASK | U_GC_PF_MASK);
  }

  float TextUtil::HangablePunctuationEndWidth(const InlineTextItem &inlineTextItem,
                                              const RenderStyle &style) noexcept
  {
    if (!HasHangablePunctuationEnd(inlineTextItem, style))
    {
      return {};
    }
    krys_debug_assert(inlineTextItem.Length());
    auto content = inlineTextItem.Content();
    auto trailingPosition =
      inlineTextItem.Start() + static_cast<uint32>(text::PreviousCodePointStart(content, content.size()));
    return Width(inlineTextItem, style.GetFontCascade(), trailingPosition, inlineTextItem.End(), {});
  }

  bool TextUtil::HasHangableStopOrCommaEnd(const InlineTextItem &inlineTextItem,
                                           const RenderStyle &style) noexcept
  {
    if (!inlineTextItem.Length()
        || !style.GetHangingPunctuation().ContainsAny(
          {style::HangingPunctuationValue::AllowEnd, style::HangingPunctuationValue::ForceEnd}))
    {
      return false;
    }
    auto content = inlineTextItem.Content();
    auto trailingCharacter =
      DecodeAt(content, text::PreviousCodePointStart(content, content.size())).CodePoint;
    auto isHangableStopOrComma =
      trailingCharacter == 0x002C || trailingCharacter == 0x002E || trailingCharacter == 0x060C
      || trailingCharacter == 0x06D4 || trailingCharacter == 0x3001 || trailingCharacter == 0x3002
      || trailingCharacter == 0xFF0C || trailingCharacter == 0xFF0E || trailingCharacter == 0xFE50
      || trailingCharacter == 0xFE51 || trailingCharacter == 0xFE52 || trailingCharacter == 0xFF61
      || trailingCharacter == 0xFF64;
    return isHangableStopOrComma;
  }

  float TextUtil::HangableStopOrCommaEndWidth(const InlineTextItem &inlineTextItem,
                                              const RenderStyle &style) noexcept
  {
    if (!HasHangableStopOrCommaEnd(inlineTextItem, style))
    {
      return {};
    }
    krys_debug_assert(inlineTextItem.Length());
    auto content = inlineTextItem.Content();
    auto trailingPosition =
      inlineTextItem.Start() + static_cast<uint32>(text::PreviousCodePointStart(content, content.size()));
    return Width(inlineTextItem, style.GetFontCascade(), trailingPosition, inlineTextItem.End(), {});
  }

  bool TextUtil::CanUseSimplifiedTextMeasuring(utf8_stringview textContent, const FontCascade &fontCascade,
                                               bool whitespaceIsCollapsed,
                                               const RenderStyle *firstLineStyle) noexcept
  {
    krys_debug_assert(FontCascade::CharacterRangeCodePath(textContent) == FontCascade::CodePath::Simple);
    // FIXME: All these checks should be more fine-grained at the inline item level.
    if (fontCascade.WordSpacing() || fontCascade.LetterSpacing())
    {
      return false;
    }

    auto fontVariantCaps = fontCascade.GetFontDescription().VariantCaps();
    if (fontVariantCaps == FontVariantCaps::Small || fontVariantCaps == FontVariantCaps::AllSmall
        || fontVariantCaps == FontVariantCaps::Petite || fontVariantCaps == FontVariantCaps::AllPetite)
    {
      return false;
    }

    // Additional check on the font codepath.
    auto run = TextRun(textContent);
    run.SetCharacterScanForCodePath(false);
    if (fontCascade.GetCodePath(run) != FontCascade::CodePath::Simple)
    {
      return false;
    }

    if (firstLineStyle && fontCascade != firstLineStyle->GetFontCascade())
    {
      return false;
    }

    Ref<const Font> primaryFont = fontCascade.PrimaryFont();
    if (primaryFont->SyntheticBoldOffset())
    {
      return false;
    }

    for (auto it = text::CodePointIterator(textContent); !it.AtEnd(); ++it)
    {
      if (!fontCascade.CanUseSimplifiedTextMeasuring(*it, AutoVariant, whitespaceIsCollapsed, *primaryFont))
      {
        return false;
      }
    }
    return true;
  }

  bool TextUtil::HasPositionDependentContentWidth(utf8_stringview textContent) noexcept
  {
    return textContent.find(u8'\t') != utf8_stringview::npos;
  }

  char32 TextUtil::LastBaseCharacterFromText(utf8_stringview string) noexcept
  {
    for (auto position = string.size(); position > 0;)
    {
      position = text::PreviousCodePointStart(string, position);
      auto character = DecodeAt(string, position).CodePoint;
      if (!IsCombiningMark(character))
      {
        return character;
      }
    }
    return 0;
  }
}
