#pragma once

#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  class Font;
  class FontCascade;
  class RenderStyle;
}

namespace krys::boo::layout
{
  class Box;
  class InlineTextBox;
  class InlineTextItem;

  /// @brief Text measurement and analysis for inline layout. Positions and lengths are byte offsets into the
  /// UTF-8 content.
  class TextUtil
  {
  public:
    TextUtil() = delete;

    enum class UseTrailingWhitespaceMeasuringOptimization : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD static InlineLayoutUnit Width(const InlineTextItem &inlineTextItem,
                                                 const FontCascade &fontCascade,
                                                 InlineLayoutUnit contentLogicalLeft) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit
      Width(const InlineTextItem &inlineTextItem, const FontCascade &fontCascade, uint32 from, uint32 to,
            InlineLayoutUnit contentLogicalLeft,
            UseTrailingWhitespaceMeasuringOptimization useTrailingWhitespaceMeasuringOptimization =
              UseTrailingWhitespaceMeasuringOptimization::Yes,
            text_spacing::SpacingState spacingState = {}) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit
      Width(const InlineTextBox &inlineTextBox, const FontCascade &fontCascade, uint32 from, uint32 to,
            InlineLayoutUnit contentLogicalLeft,
            UseTrailingWhitespaceMeasuringOptimization useTrailingWhitespaceMeasuringOptimization =
              UseTrailingWhitespaceMeasuringOptimization::Yes,
            text_spacing::SpacingState spacingState = {}) noexcept;

    KRYS_NODISCARD static InlineLayoutUnit TrailingWhitespaceWidth(const InlineTextBox &inlineTextBox,
                                                                   const FontCascade &fontCascade,
                                                                   size_t startPosition,
                                                                   size_t endPosition) noexcept;

    using FallbackFontList = HashSet<RefPtr<const Font>>;
    enum class IncludeHyphen : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD static FallbackFontList FallbackFontsForText(utf8_stringview textContent,
                                                                const RenderStyle &style,
                                                                IncludeHyphen includeHyphen) noexcept;

    struct EnclosingAscentDescent
    {
      InlineLayoutUnit Ascent {0.f};
      InlineLayoutUnit Descent {0.f};
    };
    enum class ShouldUseSimpleGlyphOverflowCodePath : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD static EnclosingAscentDescent EnclosingGlyphBoundsForText(
      utf8_stringview textContent, const RenderStyle &style,
      ShouldUseSimpleGlyphOverflowCodePath shouldUseSimpleGlyphOverflowCodePath) noexcept;

    struct WordBreakLeft
    {
      size_t Length {0};
      InlineLayoutUnit LogicalWidth {0};
    };
    KRYS_NODISCARD static WordBreakLeft BreakWord(const InlineTextBox &inlineTextBox, size_t start,
                                                  size_t length, InlineLayoutUnit width,
                                                  InlineLayoutUnit availableWidth,
                                                  InlineLayoutUnit contentLogicalLeft,
                                                  const FontCascade &fontCascade) noexcept;
    KRYS_NODISCARD static WordBreakLeft BreakWord(const InlineTextItem &inlineTextItem,
                                                  const FontCascade &fontCascade, InlineLayoutUnit textWidth,
                                                  InlineLayoutUnit availableWidth,
                                                  InlineLayoutUnit contentLogicalLeft) noexcept;

    KRYS_NODISCARD static bool MayBreakInBetween(const InlineTextItem &previousInlineItem,
                                                 const InlineTextItem &nextInlineItem) noexcept;
    // FIXME: Remove when computeInlinePreferredLogicalWidths is all IFC.
    KRYS_NODISCARD static bool MayBreakInBetween(utf8_stringview previousContent,
                                                 const RenderStyle &previousContentStyle,
                                                 utf8_stringview nextContent,
                                                 const RenderStyle &nextContentStyle) noexcept;
    KRYS_NODISCARD static uint32
      FindNextBreakablePosition(CachedLineBreakIteratorFactory &lineBreakIteratorFactory,
                                uint32 startPosition, const RenderStyle &style) noexcept;
    KRYS_NODISCARD static TextBreakLineBehavior LineBreakIteratorMode(LineBreak lineBreak) noexcept;
    KRYS_NODISCARD static TextBreakIterator::ContentAnalysis GetContentAnalysis(WordBreak wordBreak) noexcept;

    KRYS_NODISCARD static bool ShouldPreserveSpacesAndTabs(const Box &layoutBox) noexcept;
    KRYS_NODISCARD static bool ShouldPreserveNewline(const Box &layoutBox) noexcept;
    KRYS_NODISCARD static bool IsWrappingAllowed(const RenderStyle &style) noexcept;
    KRYS_NODISCARD static bool ShouldTrailingWhitespaceHang(const RenderStyle &style) noexcept;

    KRYS_NODISCARD static bool IsStrongDirectionalityCharacter(char32 character) noexcept;
    KRYS_NODISCARD static bool ContainsStrongDirectionalityText(utf8_stringview text) noexcept;

    KRYS_NODISCARD static const StringAtom &EllipsisTextInInlineDirection(bool isHorizontal = true) noexcept;

    KRYS_NODISCARD static InlineLayoutUnit HyphenWidth(const RenderStyle &style) noexcept;

    KRYS_NODISCARD static size_t
      FirstUserPerceivedCharacterLength(const InlineTextItem &inlineTextItem) noexcept;
    KRYS_NODISCARD static size_t FirstUserPerceivedCharacterLength(const InlineTextBox &inlineTextBox,
                                                                   size_t startPosition,
                                                                   size_t length) noexcept;
    KRYS_NODISCARD static TextDirection DirectionForTextContent(utf8_stringview content) noexcept;

    KRYS_NODISCARD static bool HasHangablePunctuationStart(const InlineTextItem &inlineTextItem,
                                                           const RenderStyle &style) noexcept;
    KRYS_NODISCARD static float HangablePunctuationStartWidth(const InlineTextItem &inlineTextItem,
                                                              const RenderStyle &style) noexcept;

    KRYS_NODISCARD static bool HasHangablePunctuationEnd(const InlineTextItem &inlineTextItem,
                                                         const RenderStyle &style) noexcept;
    KRYS_NODISCARD static float HangablePunctuationEndWidth(const InlineTextItem &inlineTextItem,
                                                            const RenderStyle &style) noexcept;

    KRYS_NODISCARD static bool HasHangableStopOrCommaEnd(const InlineTextItem &inlineTextItem,
                                                         const RenderStyle &style) noexcept;
    KRYS_NODISCARD static float HangableStopOrCommaEndWidth(const InlineTextItem &inlineTextItem,
                                                            const RenderStyle &style) noexcept;

    KRYS_NODISCARD static bool CanUseSimplifiedTextMeasuring(utf8_stringview textContent,
                                                             const FontCascade &fontCascade,
                                                             bool whitespaceIsCollapsed,
                                                             const RenderStyle *firstLineStyle) noexcept;
    KRYS_NODISCARD static bool HasPositionDependentContentWidth(utf8_stringview textContent) noexcept;

    KRYS_NODISCARD static char32 LastBaseCharacterFromText(utf8_stringview string) noexcept;
  };
}
