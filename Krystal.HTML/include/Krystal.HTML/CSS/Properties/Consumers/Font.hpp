#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSPrimitiveValue;
  class CSSValue;
  class CSSValueList;
  class ScriptExecutionContext;

  enum class FontTechnology : uint8_t;

  struct CSSParserContext;

  namespace WebKitFontFamilyNames
  {
    enum class FamilyNamesIndex;
  }

  namespace CSSPropertyParserHelpers
  {
    // MARK: - Font

    // normal | italic | oblique <angle [-90deg,90deg]>?
    using UnresolvedFontStyleObliqueAngle = Angle<CSSRange {-90, 90}>;
    using UnresolvedFontStyle = Variant<CSSValueId, UnresolvedFontStyleObliqueAngle>;

    // normal | small-caps
    using UnresolvedFontVariantCaps = CSSValueId;

    // normal | bold | bolder | lighter | <number [1,1000]>
    using UnresolvedFontWeightNumber = Number<CSSRange {1, 1'000}>;
    using UnresolvedFontWeight = Variant<CSSValueId, UnresolvedFontWeightNumber>;

    // normal | <percentage [0,∞]> | ultra-condensed | extra-condensed | condensed | semi-condensed |
    // semi-expanded | expanded | extra-expanded | ultra-expanded
    using UnresolvedFontWidthPercentage = Percentage<NonNegative>;
    using UnresolvedFontWidth = Variant<CSSValueId, UnresolvedFontWidthPercentage>;

    // <absolute-size> | <relative-size> | <length-percentage [0,∞]>
    using UnresolvedFontSize = Variant<CSSValueId, LengthPercentage<NonNegative>>;

    // normal | <number [0,∞]> | <length-percentage [0,∞]>
    using UnresolvedFontLineHeight = Variant<CSSValueId, Number<NonNegative>, LengthPercentage<NonNegative>>;

    // [ <family-name> | <generic-family> ]#
    using UnresolvedFontFamilyName = Variant<CSSValueId, CSSOMStringAtom>;
    using UnresolvedFontFamily = SmallList<UnresolvedFontFamilyName>;

    struct UnresolvedFont
    {
      UnresolvedFontStyle style;
      UnresolvedFontVariantCaps variantCaps;
      UnresolvedFontWeight weight;
      UnresolvedFontWidth width;
      UnresolvedFontSize size;
      UnresolvedFontLineHeight lineHeight;
      UnresolvedFontFamily family;
    };

    // MARK: 'font' (shorthand)
    // https://drafts.csswg.org/css-fonts-4/#font-prop
    KRYS_NODISCARD Maybe<UnresolvedFont>
      ParseUnresolvedFont(const CSSOMString &font, ScriptExecutionContext &context,
                          Maybe<CSSParserMode> parserModeOverride = Null) noexcept;

    // MARK: 'font-style'
    // https://drafts.csswg.org/css-fonts-4/#font-style-prop
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeFontStyle(CSSTokenRange &tokens,
                                                     CSSPropertyParserState &state) noexcept;

    // MARK: 'font-family'
    // https://drafts.csswg.org/css-fonts-4/#font-family-prop
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeFontFamily(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;

    // Sub-production of 'font-family': <family-name>
    // https://drafts.csswg.org/css-fonts-4/#family-name-syntax
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeFamilyName(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;

    // Sub-production of 'font-family': <generic-family>
    // https://drafts.csswg.org/css-fonts-4/#generic-family-name-syntax
    KRYS_NODISCARD const CSSOMStringAtom &GenericFontFamily(CSSValueId) noexcept;

    KRYS_NODISCARD WebKitFontFamilyNames::FamilyNamesIndex GenericFontFamilyIndex(CSSValueId) noexcept;

    // MARK: 'font-size-adjust'
    // https://drafts.csswg.org/css-fonts-4/#font-size-adjust-prop
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeFontSizeAdjust(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state) noexcept;

    // MARK: - @font-face descriptor consumers

    // MARK: @font-face 'src'
    // https://drafts.csswg.org/css-fonts-4/#src-desc
    KRYS_NODISCARD RefPtr<CSSValueList> ParseFontFaceSrc(const CSSOMString &,
                                                         ScriptExecutionContext &) noexcept;

    KRYS_NODISCARD RefPtr<CSSValueList> ConsumeFontFaceSrc(CSSTokenRange &tokens,
                                                           CSSPropertyParserState &state) noexcept;

    // Sub-production of 'src: <font-tech>
    // https://drafts.csswg.org/css-fonts-4/#font-tech-values
    KRYS_NODISCARD SmallList<FontTechnology> ConsumeFontTech(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state,
                                                             bool singleValue = false) noexcept;

    // Sub-production of 'src': <font-format>
    // https://drafts.csswg.org/css-fonts-4/#font-format-values
    KRYS_NODISCARD CSSOMString ConsumeFontFormat(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                 bool rejectStringValues = false) noexcept;

    // MARK: @font-face 'size-adjust'
    // https://drafts.csswg.org/css-fonts-5/#descdef-font-face-size-adjust
    KRYS_NODISCARD RefPtr<CSSValue> ParseFontFaceSizeAdjust(const CSSOMString &,
                                                            ScriptExecutionContext &) noexcept;

    // MARK: @font-face 'unicode-range'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-unicode-range
    KRYS_NODISCARD RefPtr<CSSValueList> ParseFontFaceUnicodeRange(const CSSOMString &,
                                                                  ScriptExecutionContext &) noexcept;

    // MARK: @font-face 'font-display'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-display
    KRYS_NODISCARD RefPtr<CSSValue> ParseFontFaceDisplay(const CSSOMString &,
                                                         ScriptExecutionContext &) noexcept;

    // MARK: @font-face 'font-style'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-style
    KRYS_NODISCARD RefPtr<CSSValue> ParseFontFaceFontStyle(const CSSOMString &,
                                                           ScriptExecutionContext &) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeFontFaceFontStyle(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept;

    // MARK: @font-face 'font-feature-settings'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-feature-settings
    KRYS_NODISCARD RefPtr<CSSValue> ParseFontFaceFeatureSettings(const CSSOMString &,
                                                                 ScriptExecutionContext &) noexcept;
    // Sub-production of 'font-feature-settings': <feature-tag-value>
    // https://drafts.csswg.org/css-fonts-4/#feature-tag-value
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeFeatureTagValue(CSSTokenRange &tokens,
                                                           CSSPropertyParserState &state) noexcept;

    // MARK: @font-face 'font-variation-settings'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-variation-settings
    // Sub-production of 'font-variation-settings': <variation-tag-value>
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeVariationTagValue(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept;

    // MARK: @font-face 'font-width'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-width
    KRYS_NODISCARD RefPtr<CSSValue> ParseFontFaceFontWidth(const CSSOMString &,
                                                           ScriptExecutionContext &) noexcept;

    // MARK: @font-face 'font-weight'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-weight
    KRYS_NODISCARD RefPtr<CSSValue> ParseFontFaceFontWeight(const CSSOMString &,
                                                            ScriptExecutionContext &) noexcept;

    // MARK: - @font-feature-values descriptor consumers

    // MARK: @font-feature-values 'prelude family name list'
    // https://drafts.csswg.org/css-fonts-4/#font-feature-values-syntax
    KRYS_NODISCARD SmallList<CSSOMStringAtom>
      ConsumeFontFeatureValuesPreludeFamilyNameList(CSSTokenRange &tokens, const CSSParserContext &) noexcept;

    // Template and inline implementations are at the bottom of the file for readability.

    KRYS_NODISCARD constexpr bool IsSystemFontShorthand(CSSValueId valueId) noexcept
    {
      // This needs to stay in sync with SystemFontDatabase::FontShorthand.
      // static_assert(CSSValueId::StatusBar - CSSValueId::Caption
      //              == static_cast<SystemFontDatabase::FontShorthandUnderlyingType>(
      //                SystemFontDatabase::FontShorthand::StatusBar));
      return valueId >= CSSValueId::Caption && valueId <= CSSValueId::StatusBar;
    }

    // inline SystemFontDatabase::FontShorthand lowerFontShorthand(CSSValueId valueId)
    //{
    //   // This needs to stay in sync with SystemFontDatabase::FontShorthand.
    //   ASSERT(isSystemFontShorthand(valueId));
    //   return static_cast<SystemFontDatabase::FontShorthand>(valueId - CSSValueCaption);
    // }
  }
}