#pragma once

#include "Krystal.Booey/Core/CharacterProperties.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatSegment.hpp"
#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/FontCascadeFonts.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Graphics/FontSelector.hpp"
#include "Krystal.Booey/Graphics/GlyphBuffer.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <bitset>
#include <cmath>
#include <utility>

namespace krys::boo
{
  class GraphicsContext;

  /// @brief How far a run's glyphs reach outside its box.
  struct GlyphOverflow
  {
    LayoutUnit Left;
    LayoutUnit Right;
    LayoutUnit Top;
    LayoutUnit Bottom;
    bool ComputeBounds {false};

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return !Left && !Right && !Top && !Bottom;
    }

    void ExtendTo(const GlyphOverflow &other) noexcept
    {
      Left = std::max(Left, other.Left);
      Right = std::max(Right, other.Right);
      Top = std::max(Top, other.Top);
      Bottom = std::max(Bottom, other.Bottom);
    }

    void ExtendTop(float extendTo) noexcept
    {
      Top = std::max(Top, LayoutUnit(std::ceil(extendTo)));
    }

    void ExtendBottom(float extendTo) noexcept
    {
      Bottom = std::max(Bottom, LayoutUnit(std::ceil(extendTo)));
    }

    /// @brief Whether the overflows differ, ignoring ComputeBounds.
    KRYS_NODISCARD bool DiffersFrom(const GlyphOverflow &other) const noexcept
    {
      return Left != other.Left || Right != other.Right || Top != other.Top || Bottom != other.Bottom;
    }
  };

  /// @brief The fonts a run of text is drawn with: a description, the spacing applied between letters and
  /// words, and the fonts that description resolves to (with the document's font selector).
  /// @note FIXME: Display lists of text runs (`displayListForTextRun`, for the glyph display list cache) are
  /// still to come.
  class FontCascade final
  {
    friend struct WidthIterator;
    friend class ComplexTextController;

  public:
    // Keep this in sync with RenderText's font code path.
    enum class CodePath : uint8
    {
      Auto,
      Simple,
      Complex,
      SimpleWithGlyphOverflow
    };

  private:
    static constexpr uint32 BitsPerCharacterInCanUseSimplifiedTextMeasuringForAutoVariantCache = 2;

    static CodePath s_codePath;

    struct Spacing
    {
      float Letter {0};
      float Word {0};

      KRYS_NODISCARD constexpr bool operator==(const Spacing &) const noexcept = default;
    };

    mutable FontCascadeDescription _fontDescription;
    Spacing _spacing;
    mutable RefPtr<FontCascadeFonts> _fonts;
    mutable RefPtr<FontSelector> _fontSelector;
    mutable uint32 _generation {0};
    bool _useBackslashAsYenSymbol {false};
    bool _enableKerning {false};   // Computed from _fontDescription.
    bool _requiresShaping {false}; // Computed from _fontDescription.
    mutable std::bitset<256 * BitsPerCharacterInCanUseSimplifiedTextMeasuringForAutoVariantCache>
      _canUseSimplifiedTextMeasuringForAutoVariantCache;

  public:
    FontCascade() noexcept;
    explicit FontCascade(FontCascadeDescription &&) noexcept;
    FontCascade(FontCascadeDescription &&, const FontCascade &) noexcept;
    /// @brief A cascade with only the font of @p platformData.
    explicit FontCascade(const FontPlatformData &platformData,
                         FontSmoothingMode fontSmoothingMode = FontSmoothingMode::Auto) noexcept;

    FontCascade(const FontCascade &) noexcept;
    FontCascade &operator=(const FontCascade &) noexcept;
    ~FontCascade() noexcept;

    KRYS_NODISCARD bool operator==(const FontCascade &other) const noexcept;

    /// @brief Whether the cascade's fonts are up to date with the font cache and @p fontSelector.
    KRYS_NODISCARD bool IsCurrent(const FontSelector &fontSelector) const noexcept;
    void UpdateFonts(Ref<FontCascadeFonts> &&fonts) const noexcept;
    /// @brief Resolves the cascade's fonts again, with @p fontSelector.
    void Update(RefPtr<FontSelector> &&fontSelector = nullptr) const noexcept;
    KRYS_NODISCARD uint32 FontSelectorVersion() const noexcept;

    KRYS_NODISCARD const FontCascadeDescription &GetFontDescription() const noexcept
    {
      return _fontDescription;
    }
    KRYS_NODISCARD FontCascadeDescription &MutableFontDescription() const noexcept
    {
      return _fontDescription;
    }

    KRYS_NODISCARD float Size() const noexcept
    {
      return GetFontDescription().ComputedSize();
    }

    enum class CustomFontNotReadyAction : bool
    {
      DoNotPaintIfFontNotReady,
      UseFallbackIfFontNotReady
    };
    /// @brief Draws @p run from byte offset @p from to @p to at @p point (its start on the baseline).
    /// @return How far the first glyph drawn is from @p point (the width of the text before @p from).
    FloatSize DrawText(GraphicsContext &context, const TextRun &run, const FloatPoint &point, uint32 from = 0,
                       Maybe<uint32> to = null,
                       CustomFontNotReadyAction customFontNotReadyAction =
                         CustomFontNotReadyAction::DoNotPaintIfFontNotReady) const noexcept;
    /// @brief Draws @p mark over each character of @p run from byte offset @p from to @p to that can take
    /// one, @p point being the run's start on the baseline the marks are drawn on.
    void DrawEmphasisMarks(GraphicsContext &context, const TextRun &run, const StringAtom &mark,
                           const FloatPoint &point, uint32 from = 0, Maybe<uint32> to = null) const noexcept;
    /// @brief Where @p run's glyphs, drawn from @p textOrigin, cross @p lineExtents (an underline or
    /// overline), as spans along the line from its left: each glyph's outline where it crosses the line's top
    /// and bottom, for decorations that skip ink.
    KRYS_NODISCARD List<FloatSegment>
      LineSegmentsForIntersectionsWithRect(const TextRun &run, const FloatPoint &textOrigin,
                                           const FloatRect &lineExtents) const noexcept;

    /// @brief The width of @p run from byte offset @p from to @p to.
    /// @param outWidthBeforeRange Set to the width of the run before @p from.
    /// @param outWidthAfterRange Set to the width of the run after @p to.
    KRYS_NODISCARD float WidthOfTextRange(const TextRun &run, uint32 from, uint32 to,
                                          float &outWidthBeforeRange,
                                          float &outWidthAfterRange) const noexcept;
    /// @brief The width of @p run.
    /// @param fallbackFonts If not null, the fonts other than the primary font that the run uses are added to
    /// it.
    /// @param glyphOverflow If not null, set to how far the run's glyphs reach outside its box.
    KRYS_NODISCARD float Width(const TextRun &run, HashSet<RefPtr<const Font>> *fallbackFonts = nullptr,
                               GlyphOverflow *glyphOverflow = nullptr) const noexcept;
    KRYS_NODISCARD float Width(utf8_stringview text) const noexcept;
    KRYS_NODISCARD float
      WidthForTextUsingSimplifiedMeasuring(utf8_stringview text,
                                           TextDirection = TextDirection::LTR) const noexcept;
    KRYS_NODISCARD float WidthForSimpleTextWithFixedPitch(utf8_stringview text,
                                                          bool whitespaceIsCollapsed) const noexcept;
    /// @brief The width of the character at byte offset @p characterPosition of @p run.
    KRYS_NODISCARD float WidthForCharacterInRun(const TextRun &run, uint32 characterPosition) const noexcept;

    KRYS_NODISCARD float WidthOfSpaceString() const noexcept
    {
      return Width(utf8_stringview {u8" "});
    }

    /// @brief The byte offset of the character at @p position (from the run's start edge) in @p run.
    /// @param includePartialGlyphs Whether a position past the middle of a glyph is after it.
    KRYS_NODISCARD int32 OffsetForPosition(const TextRun &run, float position,
                                           bool includePartialGlyphs) const noexcept;
    /// @brief Moves and sizes @p selectionRect to the text of @p run from byte offset @p from to @p to.
    void AdjustSelectionRectForText(bool canUseSimplifiedTextMeasuring, const TextRun &run,
                                    LayoutRect &selectionRect, uint32 from = 0,
                                    Maybe<uint32> to = null) const noexcept;
    /// @brief The selection rect of each byte from @p from to @p to (the rects of a character's continuation
    /// bytes are empty, at the character's end).
    KRYS_NODISCARD List<LayoutRect> CharacterSelectionRectsForText(const TextRun &run,
                                                                   const LayoutRect &selectionRect,
                                                                   uint32 from,
                                                                   Maybe<uint32> to) const noexcept;

    KRYS_NODISCARD bool IsSmallCaps() const noexcept
    {
      return _fontDescription.VariantCaps() == FontVariantCaps::Small;
    }

    KRYS_NODISCARD float LetterSpacing() const noexcept
    {
      return _spacing.Letter;
    }
    KRYS_NODISCARD float WordSpacing() const noexcept
    {
      return _spacing.Word;
    }
    void SetLetterSpacing(float spacing) noexcept
    {
      _spacing.Letter = spacing;
    }
    void SetWordSpacing(float spacing) noexcept
    {
      _spacing.Word = spacing;
    }
    KRYS_NODISCARD boo::TextSpacingTrim GetTextSpacingTrim() const noexcept
    {
      return _fontDescription.GetTextSpacingTrim();
    }
    KRYS_NODISCARD boo::TextAutospace GetTextAutospace() const noexcept
    {
      return _fontDescription.GetTextAutospace();
    }

    KRYS_NODISCARD bool EnableKerning() const noexcept
    {
      return _enableKerning;
    }
    KRYS_NODISCARD bool RequiresShaping() const noexcept
    {
      return _requiresShaping;
    }

    KRYS_NODISCARD const StringAtom &FirstFamily() const noexcept
    {
      return _fontDescription.FirstFamily();
    }
    KRYS_NODISCARD uint32 FamilyCount() const noexcept
    {
      return _fontDescription.FamilyCount();
    }
    KRYS_NODISCARD const StringAtom &FamilyAt(uint32 i) const noexcept
    {
      return _fontDescription.FamilyAt(i);
    }

    /// @brief A null return value indicates "font-style: normal".
    KRYS_NODISCARD Maybe<FontSelectionValue> FontStyleSlope() const noexcept
    {
      return _fontDescription.FontStyleSlope();
    }
    KRYS_NODISCARD FontSelectionValue Weight() const noexcept
    {
      return _fontDescription.Weight();
    }
    KRYS_NODISCARD FontWidthVariant WidthVariant() const noexcept
    {
      return _fontDescription.WidthVariant();
    }

    KRYS_NODISCARD bool IsFixedPitch() const noexcept;
    KRYS_NODISCARD bool CanTakeFixedPitchFastContentMeasuring() const noexcept;

    KRYS_NODISCARD bool IsPlatformFont() const noexcept
    {
      return EnsureFonts().IsForPlatformFont();
    }

    /// @brief The metrics of the primary font.
    KRYS_NODISCARD const FontMetrics &MetricsOfPrimaryFont() const noexcept
    {
      return PrimaryFont()->GetFontMetrics();
    }

    /// @brief The advance of the `0` glyph, or 0.5em when it can't be determined.
    KRYS_NODISCARD float ZeroWidth() const noexcept;
    KRYS_NODISCARD float TabWidth(const Font &font, const boo::TabSize &tabSize, float position,
                                  Font::SyntheticBoldInclusion syntheticBoldInclusion) const noexcept;

    KRYS_NODISCARD bool HasValidAverageCharWidth() const noexcept;
    /// @brief Sets @p width to the primary font's average character width if the first family's is valid.
    KRYS_NODISCARD bool FastAverageCharWidthIfAvailable(float &width) const noexcept;

    KRYS_NODISCARD int32 EmphasisMarkAscent(const StringAtom &mark) const noexcept;
    KRYS_NODISCARD int32 EmphasisMarkDescent(const StringAtom &mark) const noexcept;
    KRYS_NODISCARD int32 EmphasisMarkHeight(const StringAtom &mark) const noexcept;
    KRYS_NODISCARD float FloatEmphasisMarkHeight(const StringAtom &mark) const noexcept;

    /// @brief The font of the first family that has a font, which the cascade's metrics come from.
    KRYS_NODISCARD Ref<const Font> PrimaryFont() const noexcept;
    KRYS_NODISCARD const FontRanges &FallbackRangesAt(uint32 index) const noexcept;
    KRYS_NODISCARD GlyphData
      GlyphDataForCharacter(char32 character, bool mirror, FontVariant variant = AutoVariant,
                            Maybe<ResolvedEmojiPolicy> resolvedEmojiPolicy = null) const noexcept;

    KRYS_NODISCARD bool CanUseSimplifiedTextMeasuring(char32 character, FontVariant fontVariant,
                                                      bool whitespaceIsCollapsed,
                                                      const Font &primaryFont) const noexcept;

    /// @brief The font of the first code point of @p text (UTF-8), or null if no font has it.
    KRYS_NODISCARD RefPtr<const Font> FontForCombiningCharacterSequence(utf8_stringview text) const noexcept;

    KRYS_NODISCARD static bool IsCJKIdeograph(char32 c) noexcept;
    KRYS_NODISCARD static bool IsCJKIdeographOrSymbol(char32 c) noexcept;

    /// @brief The number of expansion opportunities in @p text (UTF-8), and whether the last expansion is a
    /// trailing expansion. If there are no opportunities, the bool will be true iff we are forbidding leading
    /// expansions.
    KRYS_NODISCARD static std::pair<uint32, bool>
      ExpansionOpportunityCount(utf8_stringview text, TextDirection direction,
                                ExpansionBehavior expansionBehavior) noexcept;

    /// @brief Whether or not there is an expansion opportunity just before the first character.
    /// @note This does not take an isAfterExpansion flag; this assumes that isAfterExpansion is false.
    KRYS_NODISCARD static bool LeftExpansionOpportunity(utf8_stringview text,
                                                        TextDirection direction) noexcept;
    KRYS_NODISCARD static bool RightExpansionOpportunity(utf8_stringview text,
                                                         TextDirection direction) noexcept;

    /// @brief The code path @p run (from byte offset @p from to @p to) is measured on.
    KRYS_NODISCARD CodePath GetCodePath(const TextRun &run, Maybe<uint32> from = null,
                                        Maybe<uint32> to = null) const noexcept;
    KRYS_NODISCARD static CodePath CharacterRangeCodePath(utf8_stringview text) noexcept;

    // Useful for debugging the different font rendering code paths.
    static void SetCodePath(CodePath codePath) noexcept;
    KRYS_NODISCARD static CodePath GetCodePath() noexcept;

    KRYS_NODISCARD static ResolvedEmojiPolicy ResolveEmojiPolicy(FontVariantEmoji fontVariantEmoji,
                                                                 char32) noexcept;

    KRYS_NODISCARD FontSelector *GetFontSelector() const noexcept
    {
      return _fontSelector.get();
    }

    KRYS_NODISCARD bool UseBackslashAsYenSymbol() const noexcept
    {
      return _useBackslashAsYenSymbol;
    }

    KRYS_NODISCARD FontCascadeFonts *Fonts() const noexcept
    {
      return _fonts.get();
    }

    KRYS_NODISCARD bool IsLoadingCustomFonts() const noexcept;

    void UpdateUseBackslashAsYenSymbol() noexcept
    {
      _useBackslashAsYenSymbol = ComputeUseBackslashAsYenSymbol();
    }

    KRYS_NODISCARD constexpr static float SyntheticObliqueAngle() noexcept
    {
      return 14;
    }

    KRYS_NODISCARD static bool IsInvisibleReplacementObjectCharacter(char32 character) noexcept
    {
      if (character != text::ObjectReplacementCharacter)
      {
        return false;
      }
      return false;
    }

    KRYS_NODISCARD static bool CanReceiveTextEmphasis(char32 c) noexcept;

    KRYS_NODISCARD static bool TreatAsSpace(char32 c) noexcept
    {
      return c == text::Space || c == text::TabCharacter || c == text::NewlineCharacter
             || c == text::NoBreakSpace;
    }

    KRYS_NODISCARD static bool
      IsCharacterWhoseGlyphsShouldBeDeletedForTextRendering(char32 character) noexcept
    {
      // https://www.w3.org/TR/css-text-3/#white-space-processing
      // "Control characters (Unicode category Cc)—other than tabs (U+0009), line feeds (U+000A), carriage
      // returns (U+000D) and sequences that form a segment break—must be rendered as a visible glyph"
      if (character == text::TabCharacter || character == text::NewlineCharacter
          || character == text::CarriageReturn)
      {
        return true;
      }
      // Also, we're omitting Null (U+0000) from this set because Chrome and Firefox do so and it's needed for
      // compat. See https://github.com/w3c/csswg-drafts/pull/6983.
      if (character == text::NullCharacter)
      {
        return true;
      }
      if (IsControlCharacter(character))
      {
        return false;
      }
      // "Unsupported Default_ignorable characters must be ignored for text rendering."
      return IsDefaultIgnorableCodePoint(character) || IsInvisibleReplacementObjectCharacter(character);
    }

    // FIXME: Callers of treatAsZeroWidthSpace() and treatAsZeroWidthSpaceInComplexScript() should probably be
    // calling isCharacterWhoseGlyphsShouldBeDeletedForTextRendering() instead.
    KRYS_NODISCARD static bool TreatAsZeroWidthSpace(char32 c) noexcept
    {
      return TreatAsZeroWidthSpaceInComplexScript(c) || c == text::ZeroWidthNonJoiner
             || c == text::ZeroWidthJoiner;
    }

    KRYS_NODISCARD static char32 NormalizeSpaces(char32 character) noexcept
    {
      if (TreatAsSpace(character))
      {
        return text::Space;
      }

      if (TreatAsZeroWidthSpace(character))
      {
        return text::ZeroWidthSpace;
      }

      return character;
    }

    KRYS_NODISCARD static bool TreatAsZeroWidthSpaceInComplexScript(char32 c) noexcept
    {
      return c < text::Space || (c >= text::DeleteCharacter && c < text::NoBreakSpace)
             || c == text::SoftHyphen || c == text::ZeroWidthSpace
             || (c >= text::LeftToRightMark && c <= text::RightToLeftMark)
             || (c >= text::LeftToRightEmbed && c <= text::RightToLeftOverride)
             || c == text::ZeroWidthNoBreakSpace || IsInvisibleReplacementObjectCharacter(c);
    }

    KRYS_NODISCARD uint32 Generation() const noexcept
    {
      return _generation;
    }

    void UpdateEnableKerning() noexcept
    {
      _enableKerning = ComputeEnableKerning();
    }
    void UpdateRequiresShaping() noexcept
    {
      _requiresShaping = ComputeRequiresShaping();
    }

  private:
    enum class ForTextEmphasisOrNot : bool
    {
      NotForTextEmphasis,
      ForTextEmphasis
    };

    /// @brief The glyphs of @p run from byte offset @p from to @p to, in visual order, the initial advance
    /// being the width before them.
    KRYS_NODISCARD GlyphBuffer LayoutText(
      CodePath codePath, const TextRun &run, uint32 from, uint32 to,
      ForTextEmphasisOrNot forTextEmphasis = ForTextEmphasisOrNot::NotForTextEmphasis) const noexcept;
    KRYS_NODISCARD GlyphBuffer LayoutSimpleText(
      const TextRun &run, uint32 from, uint32 to,
      ForTextEmphasisOrNot forTextEmphasis = ForTextEmphasisOrNot::NotForTextEmphasis) const noexcept;
    KRYS_NODISCARD GlyphBuffer LayoutComplexText(
      const TextRun &run, uint32 from, uint32 to,
      ForTextEmphasisOrNot forTextEmphasis = ForTextEmphasisOrNot::NotForTextEmphasis) const noexcept;
    /// @brief Draws @p glyphBuffer's glyphs from @p point, one run per font, moving @p point past them.
    /// @note @p point already includes the glyph buffer's initial advance.
    void DrawGlyphBuffer(GraphicsContext &context, const GlyphBuffer &glyphBuffer, FloatPoint &point,
                         CustomFontNotReadyAction customFontNotReadyAction) const noexcept;
    void DrawEmphasisMarks(GraphicsContext &context, const GlyphBuffer &glyphBuffer, const StringAtom &mark,
                           const FloatPoint &point) const noexcept;

    KRYS_NODISCARD int32 OffsetForPositionForSimpleText(const TextRun &run, float position,
                                                        bool includePartialGlyphs) const noexcept;
    KRYS_NODISCARD int32 OffsetForPositionForComplexText(const TextRun &run, float position,
                                                         bool includePartialGlyphs) const noexcept;
    void AdjustSelectionRectForSimpleText(const TextRun &run, LayoutRect &selectionRect, uint32 from,
                                          uint32 to) const noexcept;
    void AdjustSelectionRectForSimpleTextWithFixedPitch(const TextRun &run, LayoutRect &selectionRect,
                                                        uint32 from, uint32 to) const noexcept;
    void AdjustSelectionRectForComplexText(const TextRun &run, LayoutRect &selectionRect, uint32 from,
                                           uint32 to) const noexcept;

    KRYS_NODISCARD float Width(CodePath codePath, const TextRun &run,
                               HashSet<RefPtr<const Font>> *fallbackFonts = nullptr,
                               GlyphOverflow *glyphOverflow = nullptr) const noexcept;
    KRYS_NODISCARD float WidthForSimpleTextSlow(utf8_stringview text, TextDirection textDirection,
                                                float *cacheEntry) const noexcept;
    KRYS_NODISCARD bool CanHandleRunAsSimpleText(const TextRun &run, uint32 from, uint32 to) const noexcept;

    KRYS_NODISCARD Maybe<GlyphData> GetEmphasisMarkGlyphData(const StringAtom &mark) const noexcept;
    KRYS_NODISCARD const Font *FontForEmphasisMark(const StringAtom &mark) const noexcept;

    KRYS_NODISCARD constexpr static bool CanReturnFallbackFontsForComplexText() noexcept
    {
      return false;
    }
    KRYS_NODISCARD constexpr static bool CanExpandAroundIdeographsInComplexText() noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool ShouldUseComplexTextControllerForSimpleText() const noexcept;

    KRYS_NODISCARD bool ShouldUseComplexTextController(CodePath codePathToUse) const noexcept
    {
      switch (codePathToUse)
      {
        case CodePath::Complex:                 return true;
        case CodePath::Simple:
        case CodePath::SimpleWithGlyphOverflow: return ShouldUseComplexTextControllerForSimpleText();
        case CodePath::Auto:                    break;
      }
      krys_assert(false);
      return false;
    }

    /// @brief The cascade's fonts, resolved without a font selector if they haven't been resolved yet.
    KRYS_NODISCARD FontCascadeFonts &EnsureFonts() const noexcept;

    KRYS_NODISCARD bool ComputeUseBackslashAsYenSymbol() const noexcept;

    KRYS_NODISCARD bool AdvancedTextRenderingMode() const noexcept
    {
      return _fontDescription.GetTextRenderingMode() != TextRenderingMode::OptimizeSpeed;
    }

    KRYS_NODISCARD bool ComputeEnableKerning() const noexcept
    {
      auto kerning = _fontDescription.GetKerning();
      if (kerning == Kerning::Normal)
      {
        return true;
      }
      if (kerning == Kerning::NoShift)
      {
        return false;
      }
      return AdvancedTextRenderingMode();
    }

    KRYS_NODISCARD bool ComputeRequiresShaping() const noexcept
    {
      if (!_fontDescription.VariantSettings().IsAllNormal())
      {
        return true;
      }
      if (_fontDescription.FeatureSettings().size())
      {
        return true;
      }
      return AdvancedTextRenderingMode();
    }
  };

  KRYS_NODISCARD bool ShouldSynthesizeSmallCaps(bool dontSynthesizeSmallCaps, const Font *nextFont,
                                                char32 baseCharacter, Maybe<char32> capitalizedBase,
                                                FontVariantCaps fontVariantCaps,
                                                bool engageAllSmallCapsProcessing) noexcept;
  // FIXME: Capitalization is language-dependent and context-dependent and should operate on grapheme clusters
  // instead of codepoints.
  KRYS_NODISCARD Maybe<char32> Capitalized(char32 baseCharacter) noexcept;
}
