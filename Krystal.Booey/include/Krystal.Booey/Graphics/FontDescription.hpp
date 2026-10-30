#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/Graphics/FontPalette.hpp"
#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "Krystal.Booey/Graphics/FontSizeAdjust.hpp"
#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <unicode/uscript.h>

namespace krys::boo
{
  /// @brief The font a run of text asks for: size, selection request (weight, width, slope), variants,
  /// features, locale and rendering options.
  class FontDescription
  {
  private:
    // FIXME: Investigate moving these into their own object on the heap (to save memory).
    FontFeatureSettings _featureSettings;
    FontVariationSettings _variationSettings;
    FontVariantAlternates _variantAlternates {FontVariantAlternates::Normal()};
    boo::FontPalette _fontPalette {boo::FontPalette::Type::Normal, StringAtom::Null()};
    boo::FontSizeAdjust _sizeAdjust;
    StringAtom _locale {StringAtom::Null()};
    StringAtom _specifiedLocale {StringAtom::Null()};

    boo::FontSelectionRequest _fontSelectionRequest {NormalWeightValue(), NormalWidthValue(), std::nullopt};
    boo::TextSpacingTrim _textSpacingTrim;
    boo::TextAutospace _textAutospace;
    float _computedSize {0}; // Computed size adjusted for the minimum font size and the zoom factor.
    float _usedZoomFactor {1.0};

    // Whether the font is rendering on a horizontal line or a vertical line.
    uint32 _orientation : 1 {ToUnderlying(FontOrientation::Horizontal)};
    // Only used by vertical text. Determines the default orientation for non-ideograph glyphs.
    uint32 _nonCJKGlyphOrientation : 1 {ToUnderlying(boo::NonCJKGlyphOrientation::Mixed)};
    uint32 _widthVariant : 2 {ToUnderlying(FontWidthVariant::RegularWidth)};
    uint32 _textRendering : 2 {ToUnderlying(boo::TextRenderingMode::Auto)};
    uint32 _script : 7 {USCRIPT_COMMON}; // Used to help choose an appropriate font for generic font families.
    uint32 _fontSynthesisWeight : 1 {ToUnderlying(FontSynthesisLonghandValue::Auto)};
    uint32 _fontSynthesisStyle : 1 {ToUnderlying(FontSynthesisLonghandValue::Auto)};
    uint32 _fontSynthesisCaps : 1 {ToUnderlying(FontSynthesisLonghandValue::Auto)};
    uint32 _variantCommonLigatures : 2 {ToUnderlying(FontVariantLigatures::Normal)};
    uint32 _variantDiscretionaryLigatures : 2 {ToUnderlying(FontVariantLigatures::Normal)};
    uint32 _variantHistoricalLigatures : 2 {ToUnderlying(FontVariantLigatures::Normal)};
    uint32 _variantContextualAlternates : 2 {ToUnderlying(FontVariantLigatures::Normal)};
    uint32 _variantPosition : 2 {ToUnderlying(FontVariantPosition::Normal)};
    uint32 _variantCaps : 3 {ToUnderlying(FontVariantCaps::Normal)};
    uint32 _variantNumericFigure : 2 {ToUnderlying(FontVariantNumericFigure::Normal)};
    uint32 _variantNumericSpacing : 2 {ToUnderlying(FontVariantNumericSpacing::Normal)};
    uint32 _variantNumericFraction : 2 {ToUnderlying(FontVariantNumericFraction::Normal)};
    uint32 _variantNumericOrdinal : 1 {ToUnderlying(FontVariantNumericOrdinal::Normal)};
    uint32 _variantNumericSlashedZero : 1 {ToUnderlying(FontVariantNumericSlashedZero::Normal)};
    uint32 _variantEastAsianVariant : 3 {ToUnderlying(FontVariantEastAsianVariant::Normal)};
    uint32 _variantEastAsianWidth : 2 {ToUnderlying(FontVariantEastAsianWidth::Normal)};
    uint32 _variantEastAsianRuby : 1 {ToUnderlying(FontVariantEastAsianRuby::Normal)};
    uint32 _variantEmoji : 2 {ToUnderlying(FontVariantEmoji::Normal)};
    uint32 _opticalSizing : 1 {ToUnderlying(FontOpticalSizing::Auto)};
    uint32 _fontStyleAxis : 1 {ToUnderlying(boo::FontStyleAxis::slnt)};
    // If this description is allowed to match a user-installed font.
    uint32 _shouldAllowUserInstalledFonts : 1 {ToUnderlying(AllowUserInstalledFonts::No)};
    // If letter-spacing is nonzero, we need to disable ligatures, which affects font preparation.
    uint32 _shouldDisableLigaturesForSpacing : 1 {false};
    uint32 _evaluationTimeZoomEnabled : 1 {false};

  public:
    FontDescription() noexcept = default;

    KRYS_NODISCARD bool operator==(const FontDescription &) const noexcept = default;

    KRYS_NODISCARD float ComputedSize() const noexcept
    {
      return _computedSize;
    }
    KRYS_NODISCARD float UsedZoomFactor() const noexcept
    {
      return _usedZoomFactor;
    }
    KRYS_NODISCARD float ComputedSizeForRangeZoomOption(css::RangeZoomOptions option) const noexcept
    {
      return (EvaluationTimeZoomEnabled() && option == css::RangeZoomOptions::Unzoomed)
               ? UnzoomedComputedSize()
               : ComputedSize();
    }
    KRYS_NODISCARD float UnzoomedComputedSize() const noexcept
    {
      return _computedSize / _usedZoomFactor;
    }
    /// @brief Adjusted size regarding @font-face size-adjust but not regarding font-size-adjust. The latter
    /// adjustment is done with updateSizeWithFontSizeAdjust() after the font's creation.
    KRYS_NODISCARD float AdjustedSizeForFontFace(float) const noexcept;
    KRYS_NODISCARD Maybe<FontSelectionValue> FontStyleSlope() const noexcept
    {
      return _fontSelectionRequest.Slope;
    }
    KRYS_NODISCARD FontSelectionValue Width() const noexcept
    {
      return _fontSelectionRequest.Width;
    }
    KRYS_NODISCARD FontSelectionValue Weight() const noexcept
    {
      return _fontSelectionRequest.Weight;
    }
    KRYS_NODISCARD const boo::FontSelectionRequest &GetFontSelectionRequest() const noexcept
    {
      return _fontSelectionRequest;
    }
    KRYS_NODISCARD boo::TextRenderingMode GetTextRenderingMode() const noexcept
    {
      return static_cast<boo::TextRenderingMode>(_textRendering);
    }
    KRYS_NODISCARD boo::TextSpacingTrim GetTextSpacingTrim() const noexcept
    {
      return _textSpacingTrim;
    }
    KRYS_NODISCARD boo::TextAutospace GetTextAutospace() const noexcept
    {
      return _textAutospace;
    }
    KRYS_NODISCARD UScriptCode Script() const noexcept
    {
      return static_cast<UScriptCode>(_script);
    }
    /// @brief What you should be using for things like text shaping and font fallback.
    KRYS_NODISCARD const StringAtom &ComputedLocale() const noexcept
    {
      return _locale;
    }
    /// @brief What you should be using for web-exposed things like `-webkit-locale`.
    KRYS_NODISCARD const StringAtom &SpecifiedLocale() const noexcept
    {
      return _specifiedLocale;
    }
    KRYS_NODISCARD bool EvaluationTimeZoomEnabled() const noexcept
    {
      return _evaluationTimeZoomEnabled;
    }

    KRYS_NODISCARD FontOrientation Orientation() const noexcept
    {
      return static_cast<FontOrientation>(_orientation);
    }
    KRYS_NODISCARD boo::NonCJKGlyphOrientation GetNonCJKGlyphOrientation() const noexcept
    {
      return static_cast<boo::NonCJKGlyphOrientation>(_nonCJKGlyphOrientation);
    }
    KRYS_NODISCARD FontWidthVariant WidthVariant() const noexcept
    {
      return static_cast<FontWidthVariant>(_widthVariant);
    }
    KRYS_NODISCARD const FontFeatureSettings &FeatureSettings() const noexcept
    {
      return _featureSettings;
    }
    KRYS_NODISCARD const FontVariationSettings &VariationSettings() const noexcept
    {
      return _variationSettings;
    }
    KRYS_NODISCARD FontSynthesisLonghandValue FontSynthesisWeight() const noexcept
    {
      return static_cast<FontSynthesisLonghandValue>(_fontSynthesisWeight);
    }
    KRYS_NODISCARD FontSynthesisLonghandValue FontSynthesisStyle() const noexcept
    {
      return static_cast<FontSynthesisLonghandValue>(_fontSynthesisStyle);
    }
    KRYS_NODISCARD FontSynthesisLonghandValue FontSynthesisSmallCaps() const noexcept
    {
      return static_cast<FontSynthesisLonghandValue>(_fontSynthesisCaps);
    }
    KRYS_NODISCARD bool HasAutoFontSynthesisWeight() const noexcept
    {
      return FontSynthesisWeight() == FontSynthesisLonghandValue::Auto;
    }
    KRYS_NODISCARD bool HasAutoFontSynthesisStyle() const noexcept
    {
      return FontSynthesisStyle() == FontSynthesisLonghandValue::Auto;
    }
    KRYS_NODISCARD bool HasAutoFontSynthesisSmallCaps() const noexcept
    {
      return FontSynthesisSmallCaps() == FontSynthesisLonghandValue::Auto;
    }
    KRYS_NODISCARD FontVariantLigatures VariantCommonLigatures() const noexcept
    {
      return static_cast<FontVariantLigatures>(_variantCommonLigatures);
    }
    KRYS_NODISCARD FontVariantLigatures VariantDiscretionaryLigatures() const noexcept
    {
      return static_cast<FontVariantLigatures>(_variantDiscretionaryLigatures);
    }
    KRYS_NODISCARD FontVariantLigatures VariantHistoricalLigatures() const noexcept
    {
      return static_cast<FontVariantLigatures>(_variantHistoricalLigatures);
    }
    KRYS_NODISCARD FontVariantLigatures VariantContextualAlternates() const noexcept
    {
      return static_cast<FontVariantLigatures>(_variantContextualAlternates);
    }
    KRYS_NODISCARD FontVariantPosition VariantPosition() const noexcept
    {
      return static_cast<FontVariantPosition>(_variantPosition);
    }
    KRYS_NODISCARD FontVariantCaps VariantCaps() const noexcept
    {
      return static_cast<FontVariantCaps>(_variantCaps);
    }
    KRYS_NODISCARD FontVariantNumericFigure VariantNumericFigure() const noexcept
    {
      return static_cast<FontVariantNumericFigure>(_variantNumericFigure);
    }
    KRYS_NODISCARD FontVariantNumericSpacing VariantNumericSpacing() const noexcept
    {
      return static_cast<FontVariantNumericSpacing>(_variantNumericSpacing);
    }
    KRYS_NODISCARD FontVariantNumericFraction VariantNumericFraction() const noexcept
    {
      return static_cast<FontVariantNumericFraction>(_variantNumericFraction);
    }
    KRYS_NODISCARD FontVariantNumericOrdinal VariantNumericOrdinal() const noexcept
    {
      return static_cast<FontVariantNumericOrdinal>(_variantNumericOrdinal);
    }
    KRYS_NODISCARD FontVariantNumericSlashedZero VariantNumericSlashedZero() const noexcept
    {
      return static_cast<FontVariantNumericSlashedZero>(_variantNumericSlashedZero);
    }
    KRYS_NODISCARD const FontVariantAlternates &VariantAlternates() const noexcept
    {
      return _variantAlternates;
    }
    KRYS_NODISCARD FontVariantEastAsianVariant VariantEastAsianVariant() const noexcept
    {
      return static_cast<FontVariantEastAsianVariant>(_variantEastAsianVariant);
    }
    KRYS_NODISCARD FontVariantEastAsianWidth VariantEastAsianWidth() const noexcept
    {
      return static_cast<FontVariantEastAsianWidth>(_variantEastAsianWidth);
    }
    KRYS_NODISCARD FontVariantEastAsianRuby VariantEastAsianRuby() const noexcept
    {
      return static_cast<FontVariantEastAsianRuby>(_variantEastAsianRuby);
    }
    KRYS_NODISCARD FontVariantEmoji VariantEmoji() const noexcept
    {
      return static_cast<FontVariantEmoji>(_variantEmoji);
    }
    KRYS_NODISCARD FontVariantEastAsianValues VariantEastAsian() const noexcept;
    KRYS_NODISCARD FontVariantNumericValues VariantNumeric() const noexcept;
    KRYS_NODISCARD FontVariantLigaturesValues VariantLigatures() const noexcept;
    KRYS_NODISCARD FontVariantSettings VariantSettings() const noexcept;
    KRYS_NODISCARD FontOpticalSizing OpticalSizing() const noexcept
    {
      return static_cast<FontOpticalSizing>(_opticalSizing);
    }
    KRYS_NODISCARD boo::FontStyleAxis GetFontStyleAxis() const noexcept
    {
      return static_cast<boo::FontStyleAxis>(_fontStyleAxis);
    }
    KRYS_NODISCARD AllowUserInstalledFonts ShouldAllowUserInstalledFonts() const noexcept
    {
      return static_cast<AllowUserInstalledFonts>(_shouldAllowUserInstalledFonts);
    }
    KRYS_NODISCARD bool ShouldDisableLigaturesForSpacing() const noexcept
    {
      return _shouldDisableLigaturesForSpacing;
    }
    KRYS_NODISCARD const boo::FontPalette &GetFontPalette() const noexcept
    {
      return _fontPalette;
    }
    KRYS_NODISCARD boo::FontSizeAdjust GetFontSizeAdjust() const noexcept
    {
      return _sizeAdjust;
    }

    void SetComputedSize(float s, float zoom = 1.0f) noexcept
    {
      _computedSize = ClampTo<float>(s);
      _usedZoomFactor = zoom;
    }
    void SetTextSpacingTrim(boo::TextSpacingTrim v) noexcept
    {
      _textSpacingTrim = v;
    }
    void SetTextAutospace(boo::TextAutospace v) noexcept
    {
      _textAutospace = v;
    }
    void SetFontStyleAxis(boo::FontStyleAxis axis) noexcept
    {
      _fontStyleAxis = ToUnderlying(axis);
    }
    void SetFontStyleSlope(Maybe<FontSelectionValue> slope) noexcept
    {
      _fontSelectionRequest.Slope = slope;
    }
    void SetIsItalic(bool isItalic) noexcept
    {
      SetFontStyleSlope(isItalic ? Maybe<FontSelectionValue> {ItalicValue()} : Maybe<FontSelectionValue> {});
    }
    void SetWeight(FontSelectionValue weight) noexcept
    {
      _fontSelectionRequest.Weight = weight;
    }
    void SetWidth(FontSelectionValue width) noexcept
    {
      _fontSelectionRequest.Width = width;
    }
    void SetTextRenderingMode(boo::TextRenderingMode rendering) noexcept
    {
      _textRendering = ToUnderlying(rendering);
    }
    void SetOrientation(FontOrientation orientation) noexcept
    {
      _orientation = ToUnderlying(orientation);
    }
    void SetNonCJKGlyphOrientation(boo::NonCJKGlyphOrientation orientation) noexcept
    {
      _nonCJKGlyphOrientation = ToUnderlying(orientation);
    }
    /// @note Make sure new callers of this sync with FontPlatformData::isForTextCombine()!
    void SetWidthVariant(FontWidthVariant widthVariant) noexcept
    {
      _widthVariant = ToUnderlying(widthVariant);
    }
    void SetSpecifiedLocale(const StringAtom &) noexcept;
    void SetFeatureSettings(FontFeatureSettings &&settings) noexcept
    {
      _featureSettings = krys::move(settings);
    }
    void SetVariationSettings(FontVariationSettings &&settings) noexcept
    {
      _variationSettings = krys::move(settings);
    }
    void SetFontSynthesisWeight(FontSynthesisLonghandValue value) noexcept
    {
      _fontSynthesisWeight = ToUnderlying(value);
    }
    void SetFontSynthesisStyle(FontSynthesisLonghandValue value) noexcept
    {
      _fontSynthesisStyle = ToUnderlying(value);
    }
    void SetFontSynthesisSmallCaps(FontSynthesisLonghandValue value) noexcept
    {
      _fontSynthesisCaps = ToUnderlying(value);
    }
    void SetVariantCommonLigatures(FontVariantLigatures variant) noexcept
    {
      _variantCommonLigatures = ToUnderlying(variant);
    }
    void SetVariantDiscretionaryLigatures(FontVariantLigatures variant) noexcept
    {
      _variantDiscretionaryLigatures = ToUnderlying(variant);
    }
    void SetVariantHistoricalLigatures(FontVariantLigatures variant) noexcept
    {
      _variantHistoricalLigatures = ToUnderlying(variant);
    }
    void SetVariantContextualAlternates(FontVariantLigatures variant) noexcept
    {
      _variantContextualAlternates = ToUnderlying(variant);
    }
    void SetVariantPosition(FontVariantPosition variant) noexcept
    {
      _variantPosition = ToUnderlying(variant);
    }
    void SetVariantCaps(FontVariantCaps variant) noexcept
    {
      _variantCaps = ToUnderlying(variant);
    }
    void SetVariantNumericFigure(FontVariantNumericFigure variant) noexcept
    {
      _variantNumericFigure = ToUnderlying(variant);
    }
    void SetVariantNumericSpacing(FontVariantNumericSpacing variant) noexcept
    {
      _variantNumericSpacing = ToUnderlying(variant);
    }
    void SetVariantNumericFraction(FontVariantNumericFraction variant) noexcept
    {
      _variantNumericFraction = ToUnderlying(variant);
    }
    void SetVariantNumericOrdinal(FontVariantNumericOrdinal variant) noexcept
    {
      _variantNumericOrdinal = ToUnderlying(variant);
    }
    void SetVariantNumericSlashedZero(FontVariantNumericSlashedZero variant) noexcept
    {
      _variantNumericSlashedZero = ToUnderlying(variant);
    }
    void SetVariantAlternates(const FontVariantAlternates &variant) noexcept
    {
      _variantAlternates = variant;
    }
    void SetVariantAlternates(FontVariantAlternates &&variant) noexcept
    {
      _variantAlternates = krys::move(variant);
    }
    void SetVariantEastAsianVariant(FontVariantEastAsianVariant variant) noexcept
    {
      _variantEastAsianVariant = ToUnderlying(variant);
    }
    void SetVariantEastAsianWidth(FontVariantEastAsianWidth variant) noexcept
    {
      _variantEastAsianWidth = ToUnderlying(variant);
    }
    void SetVariantEastAsianRuby(FontVariantEastAsianRuby variant) noexcept
    {
      _variantEastAsianRuby = ToUnderlying(variant);
    }
    void SetVariantEmoji(FontVariantEmoji variant) noexcept
    {
      _variantEmoji = ToUnderlying(variant);
    }
    void SetVariantEastAsian(FontVariantEastAsianValues) noexcept;
    void SetVariantNumeric(FontVariantNumericValues) noexcept;
    void SetVariantLigatures(FontVariantLigaturesValues) noexcept;
    void SetOpticalSizing(FontOpticalSizing sizing) noexcept
    {
      _opticalSizing = ToUnderlying(sizing);
    }
    void SetShouldAllowUserInstalledFonts(AllowUserInstalledFonts shouldAllowUserInstalledFonts) noexcept
    {
      _shouldAllowUserInstalledFonts = ToUnderlying(shouldAllowUserInstalledFonts);
    }
    void SetShouldDisableLigaturesForSpacing(bool shouldDisableLigaturesForSpacing) noexcept
    {
      _shouldDisableLigaturesForSpacing = shouldDisableLigaturesForSpacing;
    }
    void SetFontPalette(const boo::FontPalette &fontPalette) noexcept
    {
      _fontPalette = fontPalette;
    }
    void SetFontSizeAdjust(boo::FontSizeAdjust fontSizeAdjust) noexcept
    {
      _sizeAdjust = fontSizeAdjust;
    }
    void SetEvaluationTimeZoomEnabled(bool evaluationTimeZoomEnabled) noexcept
    {
      _evaluationTimeZoomEnabled = evaluationTimeZoomEnabled;
    }

    KRYS_NODISCARD static StringAtom PlatformResolveGenericFamily(UScriptCode, const StringAtom &locale,
                                                                  const StringAtom &familyName) noexcept;
  };
}
