#pragma once

#include "Krystal.Booey/Core/CharacterProperties.hpp"
#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/FontCascadeFonts.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Graphics/FontSelector.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

namespace krys::boo
{
  /// @brief The fonts a run of text is drawn with: a description, the spacing applied between letters and
  /// words, and the fonts that description resolves to (with the document's font selector).
  /// @note FIXME: No width, drawing or text layout APIs yet (text measurement comes next).
  class FontCascade final
  {
  private:
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

    /// @brief The font of the first family that has a font, which the cascade's metrics come from.
    KRYS_NODISCARD Ref<const Font> PrimaryFont() const noexcept;
    KRYS_NODISCARD const FontRanges &FallbackRangesAt(uint32 index) const noexcept;
    KRYS_NODISCARD GlyphData
      GlyphDataForCharacter(char32 character, bool mirror, FontVariant variant = AutoVariant,
                            Maybe<ResolvedEmojiPolicy> resolvedEmojiPolicy = null) const noexcept;

    /// @brief The font of the first code point of @p text (UTF-8), or null if no font has it.
    KRYS_NODISCARD RefPtr<const Font> FontForCombiningCharacterSequence(utf8_stringview text) const noexcept;

    KRYS_NODISCARD static bool IsCJKIdeograph(char32 c) noexcept;
    KRYS_NODISCARD static bool IsCJKIdeographOrSymbol(char32 c) noexcept;

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
}
