#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class PrimitiveValue;
  class Value;
  class ValueList;
  class ScriptExecutionContext;

  enum class FontTechnology : uint8_t;

  struct ParserContext;

  namespace WebKitFontFamilyNames
  {
    enum class FamilyNamesIndex;
  }

  namespace PropertyParserHelpers
  {
    // MARK: - Font

    // normal | italic | oblique <angle [-90deg,90deg]>?
    using UnresolvedFontStyleObliqueAngle = Angle<Range {-90, 90}>;
    using UnresolvedFontStyle = Variant<ValueId, UnresolvedFontStyleObliqueAngle>;

    // normal | small-caps
    using UnresolvedFontVariantCaps = ValueId;

    // normal | bold | bolder | lighter | <number [1,1000]>
    using UnresolvedFontWeightNumber = Number<Range {1, 1'000}>;
    using UnresolvedFontWeight = Variant<ValueId, UnresolvedFontWeightNumber>;

    // normal | <percentage [0,∞]> | ultra-condensed | extra-condensed | condensed | semi-condensed |
    // semi-expanded | expanded | extra-expanded | ultra-expanded
    using UnresolvedFontWidthPercentage = Percentage<NonNegative>;
    using UnresolvedFontWidth = Variant<ValueId, UnresolvedFontWidthPercentage>;

    // <absolute-size> | <relative-size> | <length-percentage [0,∞]>
    using UnresolvedFontSize = Variant<ValueId, LengthPercentage<NonNegative>>;

    // normal | <number [0,∞]> | <length-percentage [0,∞]>
    using UnresolvedFontLineHeight = Variant<ValueId, Number<NonNegative>, LengthPercentage<NonNegative>>;

    // [ <family-name> | <generic-family> ]#
    using UnresolvedFontFamilyName = Variant<ValueId, CSSOMStringAtom>;
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
                          Maybe<ParserMode> parserModeOverride = null) noexcept;

    // MARK: 'font-style'
    // https://drafts.csswg.org/css-fonts-4/#font-style-prop
    KRYS_NODISCARD RefPtr<Value> ConsumeFontStyle(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: 'font-family'
    // https://drafts.csswg.org/css-fonts-4/#font-family-prop
    KRYS_NODISCARD RefPtr<Value> ConsumeFontFamily(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;

    // Sub-production of 'font-family': <family-name>
    // https://drafts.csswg.org/css-fonts-4/#family-name-syntax
    KRYS_NODISCARD RefPtr<Value> ConsumeFamilyName(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;

    // Sub-production of 'font-family': <generic-family>
    // https://drafts.csswg.org/css-fonts-4/#generic-family-name-syntax
    KRYS_NODISCARD const CSSOMStringAtom &GenericFontFamily(ValueId) noexcept;

    KRYS_NODISCARD WebKitFontFamilyNames::FamilyNamesIndex GenericFontFamilyIndex(ValueId) noexcept;

    // MARK: 'font-size-adjust'
    // https://drafts.csswg.org/css-fonts-4/#font-size-adjust-prop
    KRYS_NODISCARD RefPtr<Value> ConsumeFontSizeAdjust(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept;

    // MARK: - @font-face descriptor consumers

    // MARK: @font-face 'src'
    // https://drafts.csswg.org/css-fonts-4/#src-desc
    KRYS_NODISCARD RefPtr<ValueList> ParseFontFaceSrc(const CSSOMString &,
                                                         ScriptExecutionContext &) noexcept;

    KRYS_NODISCARD RefPtr<ValueList> ConsumeFontFaceSrc(TokenRange &tokens,
                                                           PropertyParserState &state) noexcept;

    // Sub-production of 'src: <font-tech>
    // https://drafts.csswg.org/css-fonts-4/#font-tech-values
    KRYS_NODISCARD SmallList<FontTechnology> ConsumeFontTech(TokenRange &tokens, PropertyParserState &state,
                                                             bool singleValue = false) noexcept;

    // Sub-production of 'src': <font-format>
    // https://drafts.csswg.org/css-fonts-4/#font-format-values
    KRYS_NODISCARD CSSOMString ConsumeFontFormat(TokenRange &tokens, PropertyParserState &state,
                                                 bool rejectStringValues = false) noexcept;

    // MARK: @font-face 'size-adjust'
    // https://drafts.csswg.org/css-fonts-5/#descdef-font-face-size-adjust
    KRYS_NODISCARD RefPtr<Value> ParseFontFaceSizeAdjust(const CSSOMString &,
                                                            ScriptExecutionContext &) noexcept;

    // MARK: @font-face 'unicode-range'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-unicode-range
    KRYS_NODISCARD RefPtr<ValueList> ParseFontFaceUnicodeRange(const CSSOMString &,
                                                                  ScriptExecutionContext &) noexcept;

    // MARK: @font-face 'font-display'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-display
    KRYS_NODISCARD RefPtr<Value> ParseFontFaceDisplay(const CSSOMString &,
                                                         ScriptExecutionContext &) noexcept;

    // MARK: @font-face 'font-style'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-style
    KRYS_NODISCARD RefPtr<Value> ParseFontFaceFontStyle(const CSSOMString &,
                                                           ScriptExecutionContext &) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeFontFaceFontStyle(TokenRange &tokens,
                                                             PropertyParserState &state) noexcept;

    // MARK: @font-face 'font-feature-settings'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-feature-settings
    KRYS_NODISCARD RefPtr<Value> ParseFontFaceFeatureSettings(const CSSOMString &,
                                                                 ScriptExecutionContext &) noexcept;
    // Sub-production of 'font-feature-settings': <feature-tag-value>
    // https://drafts.csswg.org/css-fonts-4/#feature-tag-value
    KRYS_NODISCARD RefPtr<Value> ConsumeFeatureTagValue(TokenRange &tokens,
                                                           PropertyParserState &state) noexcept;

    // MARK: @font-face 'font-variation-settings'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-variation-settings
    // Sub-production of 'font-variation-settings': <variation-tag-value>
    KRYS_NODISCARD RefPtr<Value> ConsumeVariationTagValue(TokenRange &tokens,
                                                             PropertyParserState &state) noexcept;

    // MARK: @font-face 'font-width'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-width
    KRYS_NODISCARD RefPtr<Value> ParseFontFaceFontWidth(const CSSOMString &,
                                                           ScriptExecutionContext &) noexcept;

    // MARK: @font-face 'font-weight'
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-weight
    KRYS_NODISCARD RefPtr<Value> ParseFontFaceFontWeight(const CSSOMString &,
                                                            ScriptExecutionContext &) noexcept;

    // MARK: - @font-feature-values descriptor consumers

    // MARK: @font-feature-values 'prelude family name list'
    // https://drafts.csswg.org/css-fonts-4/#font-feature-values-syntax
    KRYS_NODISCARD SmallList<CSSOMStringAtom>
      ConsumeFontFeatureValuesPreludeFamilyNameList(TokenRange &tokens, const ParserContext &) noexcept;

    // Template and inline implementations are at the bottom of the file for readability.

    KRYS_NODISCARD constexpr bool IsSystemFontShorthand(ValueId valueId) noexcept
    {
      // This needs to stay in sync with SystemFontDatabase::FontShorthand.
      // static_assert(ValueId::StatusBar - ValueId::Caption
      //              == static_cast<SystemFontDatabase::FontShorthandUnderlyingType>(
      //                SystemFontDatabase::FontShorthand::StatusBar));
      return valueId >= ValueId::Caption && valueId <= ValueId::StatusBar;
    }

    // inline SystemFontDatabase::FontShorthand lowerFontShorthand(ValueId valueId)
    //{
    //   // This needs to stay in sync with SystemFontDatabase::FontShorthand.
    //   ASSERT(isSystemFontShorthand(valueId));
    //   return static_cast<SystemFontDatabase::FontShorthand>(valueId - CSSValueCaption);
    // }
  }
}