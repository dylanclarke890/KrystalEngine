#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/KeywordDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/List.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "Krystal.Booey/CSS/Values/FontFeatureValue.hpp"
#include "Krystal.Booey/CSS/Values/FontStyleRangeValue.hpp"
#include "Krystal.Booey/CSS/Values/FontStyleWithAngleValue.hpp"
#include "Krystal.Booey/CSS/Values/FontVariationValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  KRYS_NODISCARD static Ref<PrimitiveValue> ResolveToPrimitiveValue(Numeric auto &&primitive) noexcept
  {
    return PrimitiveValueResolverBase::Resolve(krys::move(primitive));
  }

  // MARK: - 'font-width'

  KRYS_NODISCARD static Maybe<UnresolvedFontWidth> ConsumeFontWidthUnresolved(TokenRange &tokens,
                                                                              PropertyParserState &) noexcept
  {
    // <'font-width'> = normal | <percentage [0,∞]> | ultra-condensed | extra-condensed | condensed |
    // semi-condensed | semi-expanded | expanded | extra-expanded | ultra-expanded
    // https://drafts.csswg.org/css-fonts-4/#propdef-font-width

    // FIXME: Add support for consuming the percentage value.
    // FIXME: Add a way to export this "raw" version from the generated PropertyParsing.

    auto keyword = ConsumeIdentRaw<ValueId::UltraCondensed, ValueId::ExtraCondensed, ValueId::Condensed,
                                   ValueId::SemiCondensed, ValueId::Normal, ValueId::SemiExpanded,
                                   ValueId::Expanded, ValueId::ExtraExpanded, ValueId::UltraExpanded>(tokens);
    if (!keyword)
    {
      return null;
    }
    return UnresolvedFontWidth {*keyword};
  }

  // MARK: - 'font-weight'

  KRYS_NODISCARD static Maybe<UnresolvedFontWeightNumber>
    ConsumeFontWeightNumberUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    auto tokensCopy = tokens;
    auto number = MetaConsumer<UnresolvedFontWeightNumber>::Consume(tokensCopy, state);
    if (!number)
    {
      return null;
    }

    tokens = tokensCopy;
    return number;
  }

  KRYS_NODISCARD static Maybe<UnresolvedFontWeight>
    ConsumeFontWeightUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'font-weight'> = normal | bold | bolder | lighter | <number [1,1000]>
    // https://drafts.csswg.org/css-fonts-4/#font-weight-prop

    if (auto keyword =
          ConsumeIdentRaw<ValueId::Normal, ValueId::Bold, ValueId::Bolder, ValueId::Lighter>(tokens))
    {
      return UnresolvedFontWeight {*keyword};
    }
    if (auto fontWeightNumber = ConsumeFontWeightNumberUnresolved(tokens, state))
    {
      return UnresolvedFontWeight {krys::move(*fontWeightNumber)};
    }
    return null;
  }

  // MARK: - 'font-style'

  KRYS_NODISCARD static Maybe<UnresolvedFontStyleObliqueAngle>
    ConsumeFontStyleAngleUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    auto tokensCopy = tokens;

    auto angle = MetaConsumer<UnresolvedFontStyleObliqueAngle>::Consume(tokensCopy, state);
    if (!angle)
    {
      return null;
    }

    tokens = tokensCopy;
    return angle;
  }

  KRYS_NODISCARD static Maybe<UnresolvedFontStyle>
    ConsumeFontStyleUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    auto keyword = ConsumeIdentRaw<ValueId::Normal, ValueId::Italic, ValueId::Oblique>(tokens);
    if (!keyword)
    {
      return null;
    }

    if (*keyword == ValueId::Oblique && !tokens.IsAtEnd())
    {
      if (auto angle = ConsumeFontStyleAngleUnresolved(tokens, state))
      {
        return UnresolvedFontStyle {krys::move(*angle)};
      }
    }

    return UnresolvedFontStyle {*keyword};
  }

  RefPtr<Value> ConsumeFontStyle(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    auto keyword = ConsumeIdentRaw<ValueId::Normal, ValueId::Italic, ValueId::Oblique>(tokens);
    if (!keyword)
    {
      return nullptr;
    }

    if (*keyword == ValueId::Oblique && !tokens.IsAtEnd())
    {
      if (auto angle = ConsumeFontStyleAngleUnresolved(tokens, state))
      {
        return FontStyleWithAngleValue::Create(krys::move(*angle));
      }
    }

    return PrimitiveValue::Create(*keyword);
  }

  // MARK: - 'font-family'

  const CSSOMStringAtom &GenericFontFamily(ValueId ident) noexcept
  {
    switch (ident)
    {
      case ValueId::Serif:
      {
        return WebKitFontFamilyNames::SerifFamily();
      }
      case ValueId::SansSerif:
      {
        return WebKitFontFamilyNames::SansSerifFamily();
      }
      case ValueId::Cursive:
      {
        return WebKitFontFamilyNames::CursiveFamily();
      }
      case ValueId::Fantasy:
      {
        return WebKitFontFamilyNames::FantasyFamily();
      }
      case ValueId::Monospace:
      {
        return WebKitFontFamilyNames::MonospaceFamily();
      }
      case ValueId::WebkitPictograph:
      {
        return WebKitFontFamilyNames::PictographFamily();
      }
      case ValueId::SystemUi:
      {
        return WebKitFontFamilyNames::SystemUiFamily();
      }
      case ValueId::Math:
      {
        return WebKitFontFamilyNames::MathFamily();
      }
      default:
      {
        return CSSOMStringAtom::Null();
      }
    }
  }

  WebKitFontFamilyNames::FamilyNamesIndex GenericFontFamilyIndex(ValueId ident) noexcept
  {
    switch (ident)
    {
      case ValueId::Serif:
      {
        return WebKitFontFamilyNames::FamilyNamesIndex::SerifFamily;
      }
      case ValueId::SansSerif:
      {
        return WebKitFontFamilyNames::FamilyNamesIndex::SansSerifFamily;
      }
      case ValueId::Cursive:
      {
        return WebKitFontFamilyNames::FamilyNamesIndex::CursiveFamily;
      }
      case ValueId::Fantasy:
      {
        return WebKitFontFamilyNames::FamilyNamesIndex::FantasyFamily;
      }
      case ValueId::Monospace:
      {
        return WebKitFontFamilyNames::FamilyNamesIndex::MonospaceFamily;
      }
      case ValueId::WebkitPictograph:
      {
        return WebKitFontFamilyNames::FamilyNamesIndex::PictographFamily;
      }
      case ValueId::SystemUi:
      {
        return WebKitFontFamilyNames::FamilyNamesIndex::SystemUiFamily;
      }
      case ValueId::Math:
      {
        return WebKitFontFamilyNames::FamilyNamesIndex::MathFamily;
      }
      default:
      {
        krys_debug_assert(false);
        return WebKitFontFamilyNames::FamilyNamesIndex::StandardFamily;
      }
    }
  }

  KRYS_NODISCARD static Maybe<CSSOMStringAtom> ConcatenateFamilyName(TokenRange &tokens) noexcept
  {
    CSSOMString builder;
    bool addedSpace = false;
    const Token &firstToken = tokens.Peek();
    while (tokens.Peek().Type() == TokenType::Ident)
    {
      if (!builder.empty())
      {
        builder += u8' ';
        addedSpace = true;
      }
      builder += tokens.ConsumeIncludingWhitespace().IdentCodePoints();
    }
    // <family-name> can't contain unquoted generic families
    if (!addedSpace
        && (!IsValidCustomIdentifier(firstToken.ValueId())
            || GenericFontFamily(firstToken.ValueId()) != CSSOMStringAtom::Null()))
    {
      return null;
    }
    return CSSOMStringAtom {krys::move(builder)};
  }

  KRYS_NODISCARD static Maybe<CSSOMStringAtom> ConsumeFamilyNameUnresolved(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() == TokenType::String)
    {
      return CSSOMStringAtom {tokens.ConsumeIncludingWhitespace().IdentCodePoints()};
    }
    if (tokens.Peek().Type() != TokenType::Ident)
    {
      return null;
    }
    return ConcatenateFamilyName(tokens);
  }

  KRYS_NODISCARD static Maybe<ValueId> ConsumeGenericFamilyUnresolved(TokenRange &tokens) noexcept
  {
    return ConsumeIdentRaw<ValueId::Serif, ValueId::SansSerif, ValueId::Cursive, ValueId::Fantasy,
                           ValueId::Monospace, ValueId::WebkitBody, ValueId::WebkitPictograph,
                           ValueId::SystemUi, ValueId::Math>(tokens);
  }

  KRYS_NODISCARD static RefPtr<Value> ConsumeGenericFamily(TokenRange &tokens,
                                                           PropertyParserState &state) noexcept
  {
    if (auto familyName = ConsumeGenericFamilyUnresolved(tokens))
    {
      // FIXME: Remove special case for system-ui.
      if (*familyName == ValueId::SystemUi)
      {
        return state.Pool.CreateFontFamilyValue(CSSOMStringAtom {ToString(*familyName)});
      }
      return PrimitiveValue::Create(*familyName);
    }
    return nullptr;
  }

  KRYS_NODISCARD static Maybe<UnresolvedFontFamily> ConsumeFontFamilyUnresolved(TokenRange &tokens) noexcept
  {
    // <'font-family'> = [ <family-name> | <generic-family> ]#
    // https://drafts.csswg.org/css-fonts-4/#font-family-prop

    UnresolvedFontFamily list;
    do
    {
      if (auto ident = ConsumeGenericFamilyUnresolved(tokens))
      {
        list.push_back(UnresolvedFontFamilyName {*ident});
      }
      else
      {
        auto familyName = ConsumeFamilyNameUnresolved(tokens);
        if (!familyName)
        {
          return null;
        }
        list.push_back(UnresolvedFontFamilyName {krys::move(*familyName)});
      }
    } while (ConsumeComma(tokens));
    return list;
  }

  RefPtr<Value> ConsumeFamilyName(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // https://drafts.csswg.org/css-fonts-4/#family-name-syntax

    auto familyName = ConsumeFamilyNameUnresolved(tokens);
    if (!familyName)
    {
      return nullptr;
    }
    return state.Pool.CreateFontFamilyValue(*familyName);
  }

  RefPtr<Value> ConsumeFontFamily(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'font-family'> = [ <family-name> | <generic-family> ]#
    // https://drafts.csswg.org/css-fonts-4/#font-family-prop

    return ConsumeListSeparatedBy<',', OneOrMore>(tokens,
                                                  [&](auto &tokens) -> RefPtr<Value>
                                                  {
                                                    if (auto parsedValue =
                                                          ConsumeGenericFamily(tokens, state))
                                                    {
                                                      return parsedValue;
                                                    }
                                                    return ConsumeFamilyName(tokens, state);
                                                  });
  }

  // MARK: - 'font-size'

  KRYS_NODISCARD static Maybe<UnresolvedFontSize>
    ConsumeFontSizeUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'font-size> = <absolute-size> | <relative-size> | <length-percentage [0,∞]> | math
    // https://drafts.csswg.org/css-fonts-4/#propdef-font-size

    // Additionally supports non-standard productions: <-webkit-absolute-size> | <-webkit-relative-size>

    // FIXME: Add a way to export this "raw" version from the generated PropertyParsing.

    // -webkit-xxx-large is a parse-time alias.
    if (tokens.Peek().ValueId() == ValueId::WebkitXxxLarge)
    {
      if (auto ident = ConsumeIdentRaw(tokens); ident && *ident == ValueId::WebkitXxxLarge)
      {
        return UnresolvedFontSize {ValueId::XxxLarge};
      }
      return null;
    }

    if ((tokens.Peek().ValueId() >= ValueId::XxSmall && tokens.Peek().ValueId() <= ValueId::Larger)
        || tokens.Peek().ValueId() == ValueId::Math)
    {
      if (auto ident = ConsumeIdentRaw(tokens))
      {
        return UnresolvedFontSize {*ident};
      }
      return null;
    }

    auto tokensCopy = tokens;

    auto lengthPercentage = MetaConsumer<LengthPercentage<NonNegative>>::Consume(tokensCopy, state);
    if (!lengthPercentage)
    {
      return null;
    }

    tokens = tokensCopy;
    return UnresolvedFontSize {krys::move(*lengthPercentage)};
  }

  // MARK: - 'line-height'

  KRYS_NODISCARD static Maybe<UnresolvedFontLineHeight>
    ConsumeLineHeightUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'line-height'> = normal | <number [0,∞]> | <length-percentage [0,∞]>
    // https://www.w3.org/TR/css-inline-3/#line-height-property

    using Consumer = MetaConsumer<keywords::Normal, Number<NonNegative>, LengthPercentage<NonNegative>>;

    return Consumer::Consume(
      tokens, state, [&](keywords::Normal) { return UnresolvedFontLineHeight {ValueId::Normal}; },
      [&](auto &&value) { return UnresolvedFontLineHeight {krys::move(value)}; });
  }

  // MARK: 'font' (shorthand)

  Maybe<UnresolvedFont> ConsumeUnresolvedFont(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    Maybe<UnresolvedFontStyle> fontStyle;
    Maybe<UnresolvedFontVariantCaps> fontVariantCaps;
    Maybe<UnresolvedFontWeight> fontWeight;
    Maybe<UnresolvedFontWidth> fontWidth;
    Maybe<UnresolvedFontSize> fontSize;
    Maybe<UnresolvedFontLineHeight> fontLineHeight;
    Maybe<UnresolvedFontFamily> fontFamily;

    // Optional font-style, font-variant, font-weight and font-width in any order.
    for (uint32 i = 0; i < 4 && !tokens.IsAtEnd(); ++i)
    {
      if (ConsumeIdentRaw<ValueId::Normal>(tokens))
      {
        continue;
      }
      if (!fontStyle && (fontStyle = ConsumeFontStyleUnresolved(tokens, state)))
      {
        continue;
      }
      if (!fontVariantCaps && (fontVariantCaps = ConsumeIdentRaw<ValueId::SmallCaps>(tokens)))
      {
        continue;
      }
      if (!fontWeight && (fontWeight = ConsumeFontWeightUnresolved(tokens, state)))
      {
        continue;
      }
      if (!fontWidth && (fontWidth = ConsumeFontWidthUnresolved(tokens, state)))
      {
        continue;
      }
      break;
    }

    if (tokens.IsAtEnd())
    {
      return null;
    }

    // Required 'font-size'.
    fontSize = ConsumeFontSizeUnresolved(tokens, state);
    if (!fontSize)
    {
      return null;
    }

    if (tokens.IsAtEnd())
    {
      return null;
    }

    if (ConsumeSlash(tokens))
    {
      // If a slash is consumed, a 'line-height' is required.
      fontLineHeight = ConsumeLineHeightUnresolved(tokens, state);
      if (!fontLineHeight)
      {
        return null;
      }
    }

    // Required 'font-family'.
    fontFamily = ConsumeFontFamilyUnresolved(tokens);
    if (!fontFamily)
    {
      return null;
    }

    if (!tokens.IsAtEnd())
    {
      return null;
    }

    return UnresolvedFont {
      .Style = fontStyle.value_or(UnresolvedFontStyle {ValueId::Normal}),
      .VariantCaps = fontVariantCaps.value_or(ValueId::Normal),
      .Weight = fontWeight.value_or(UnresolvedFontWeight {ValueId::Normal}),
      .Width = fontWidth.value_or(UnresolvedFontWidth {ValueId::Normal}),
      .Size = krys::move(*fontSize),
      .LineHeight = fontLineHeight.value_or(UnresolvedFontLineHeight {ValueId::Normal}),
      .Family = krys::move(*fontFamily),
    };
  }

  // MARK: 'font-size-adjust'

  RefPtr<Value> ConsumeFontSizeAdjust(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (tokens.Peek().ValueId() == ValueId::None || tokens.Peek().ValueId() == ValueId::FromFont)
    {
      return ConsumeIdent(tokens);
    }

    auto metric = ConsumeIdent<ValueId::ExHeight, ValueId::CapHeight, ValueId::ChWidth, ValueId::IcWidth,
                               ValueId::IcHeight>(tokens);
    auto value = PrimitiveValueResolver<Number<NonNegative>>::ConsumeAndResolve(tokens, state);
    if (!value)
    {
      value = ConsumeIdent<ValueId::FromFont>(tokens);
    }

    if (!value || !metric || metric->ValueId() == ValueId::ExHeight)
    {
      return value;
    }

    return ValuePair::Create(krys::move(metric), krys::move(value));
  }

  // MARK: - @-rule descriptor consumers:

  // MARK: - @font-face

  // MARK: @font-face 'src'

  KRYS_NODISCARD static FontTechnology FontTechnologyFromValueId(ValueId id) noexcept
  {
    switch (id)
    {
      case ValueId::ColorColrv0:
      {
        return FontTechnology::ColorColrv0;
      }
      case ValueId::ColorColrv1:
      {
        return FontTechnology::ColorColrv1;
      }
      case ValueId::ColorCbdt:
      {
        return FontTechnology::ColorCbdt;
      }
      case ValueId::ColorSbix:
      {
        return FontTechnology::ColorSbix;
      }
      case ValueId::ColorSvg:
      {
        return FontTechnology::ColorSvg;
      }
      case ValueId::FeaturesAat:
      {
        return FontTechnology::FeaturesAat;
      }
      case ValueId::FeaturesGraphite:
      {
        return FontTechnology::FeaturesGraphite;
      }
      case ValueId::FeaturesOpentype:
      {
        return FontTechnology::FeaturesOpentype;
      }
      case ValueId::Incremental:
      {
        return FontTechnology::Incremental;
      }
      case ValueId::Palettes:
      {
        return FontTechnology::Palettes;
      }
      case ValueId::Variations:
      {
        return FontTechnology::Variations;
      }
      default:
      {
        return FontTechnology::Invalid;
      }
    }
  }

  List<FontTechnology> ConsumeFontTech(TokenRange &tokens, PropertyParserState &, bool singleValue) noexcept
  {
    List<FontTechnology> technologies;
    auto args = ConsumeFunction(tokens);
    do
    {
      auto &arg = args.ConsumeIncludingWhitespace();
      if (arg.Type() != TokenType::Ident)
      {
        return {};
      }
      auto technology = FontTechnologyFromValueId(arg.ValueId());
      if (technology == FontTechnology::Invalid)
      {
        return {};
      }
      technologies.push_back(technology);
    } while (ConsumeComma(args) && !singleValue);
    if (!args.IsAtEnd())
    {
      return {};
    }
    return technologies;
  }

  KRYS_NODISCARD static bool IsFontFormatKeywordValid(ValueId id) noexcept
  {
    switch (id)
    {
      case ValueId::Collection:
      case ValueId::EmbeddedOpentype:
      case ValueId::Opentype:
      case ValueId::Svg:
      case ValueId::Truetype:
      case ValueId::Woff:
      case ValueId::Woff2:
      {
        return true;
      }
      default:
      {
        return false;
      }
    }
  }

  Maybe<CSSOMString> ConsumeFontFormat(TokenRange &tokens, PropertyParserState &,
                                       bool rejectStringValues) noexcept
  {
    // https://drafts.csswg.org/css-fonts/#descdef-font-face-src
    // FIXME: We allow any identifier here and convert to strings; specification calls for certain keywords
    // and legacy compatibility strings.
    auto args = ConsumeFunction(tokens);
    auto &arg = args.ConsumeIncludingWhitespace();
    if (!args.IsAtEnd())
    {
      return null;
    }
    if (arg.Type() == TokenType::Ident && IsFontFormatKeywordValid(arg.ValueId()))
    {
      return CSSOMString {arg.IdentCodePoints()};
    }
    if (arg.Type() == TokenType::String && !rejectStringValues)
    {
      return CSSOMString {arg.IdentCodePoints()};
    }
    return null;
  }

  KRYS_NODISCARD static RefPtr<FontFaceSrcResourceValue>
    ConsumeFontFaceSrcURI(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <font-src-uri>       = <url> [ format( <font-format> ) ]? [ tech( <font-tech># ) ]?

    // <font-format>        = <string> | collection | embedded-opentype | opentype | svg | truetype |
    //                        woff | woff2
    // <font-tech>          = <font-features-tech> | <color-font-tech> | variations | palettes | incremental
    // <font-features-tech> = features-opentype | features-aat | features-graphite
    // <color-font-tech>    = color-COLRv0 | color-COLRv1 | color-SVG | color-sbix | color-CBDT

    // https://drafts.csswg.org/css-fonts-4/#typedef-font-src

    auto location = ConsumeURLRaw(tokens, state, AllowedURLModifiers::None);
    if (!location)
    {
      return nullptr;
    }

    CSSOMString format;
    List<FontTechnology> technologies;
    if (tokens.Peek().FunctionId() == ValueId::Format)
    {
      auto consumedFormat = ConsumeFontFormat(tokens, state);
      if (!consumedFormat)
      {
        return nullptr;
      }
      format = krys::move(*consumedFormat);
    }
    if (tokens.Peek().FunctionId() == ValueId::Tech)
    {
      technologies = ConsumeFontTech(tokens, state);
      if (technologies.empty())
      {
        return nullptr;
      }
    }
    if (!tokens.IsAtEnd())
    {
      return nullptr;
    }

    return FontFaceSrcResourceValue::Create(krys::move(*location), krys::move(format),
                                            krys::move(technologies));
  }

  KRYS_NODISCARD static RefPtr<Value> ConsumeFontFaceSrcLocal(TokenRange &tokens,
                                                              PropertyParserState &) noexcept
  {
    // <font-src-local>     = local( <family-name> )
    // https://drafts.csswg.org/css-fonts-4/#typedef-font-src

    TokenRange args = ConsumeFunction(tokens);
    if (args.Peek().Type() == TokenType::String)
    {
      auto &arg = args.ConsumeIncludingWhitespace();
      if (!args.IsAtEnd() || !tokens.IsAtEnd())
      {
        return nullptr;
      }
      return FontFaceSrcLocalValue::Create(CSSOMStringAtom {arg.IdentCodePoints()});
    }
    if (args.Peek().Type() == TokenType::Ident)
    {
      auto familyName = ConcatenateFamilyName(args);
      if (!familyName || !args.IsAtEnd() || !tokens.IsAtEnd())
      {
        return nullptr;
      }
      return FontFaceSrcLocalValue::Create(krys::move(*familyName));
    }
    return nullptr;
  }

  RefPtr<ValueList> ConsumeFontFaceSrc(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <font-src-list>      = <font-src>#

    // <font-src>           = <font-src-uri> | <font-src-local>
    // <font-src-uri>       = <url> [ format( <font-format> ) ]? [ tech( <font-tech># ) ]?
    // <font-src-local>     = local( <family-name> )

    // <font-format>        = <string> | collection | embedded-opentype | opentype | svg | truetype |
    //                        woff | woff2
    // <font-tech>          = <font-features-tech> | <color-font-tech> | variations | palettes | incremental
    // <font-features-tech> = features-opentype | features-aat | features-graphite
    // <color-font-tech>    = color-COLRv0 | color-COLRv1 | color-SVG | color-sbix | color-CBDT

    // https://drafts.csswg.org/css-fonts-4/#typedef-font-src

    ValueListBuilder values;
    auto ConsumeSrcListComponent = [&](TokenRange &tokens) -> RefPtr<Value>
    {
      const Token &token = tokens.Peek();
      if (token.Type() == TokenType::Url || token.FunctionId() == ValueId::Url)
      {
        return ConsumeFontFaceSrcURI(tokens, state);
      }
      if (token.FunctionId() == ValueId::Local)
      {
        return ConsumeFontFaceSrcLocal(tokens, state);
      }
      return nullptr;
    };
    while (!tokens.IsAtEnd())
    {
      auto begin = tokens;
      while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma)
      {
        tokens.DiscardComponentValue();
      }
      auto subrange = begin.RangeUntil(tokens);
      if (auto parsedValue = ConsumeSrcListComponent(subrange))
      {
        values.push_back(krys::move(parsedValue));
      }
      if (!tokens.IsAtEnd())
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
      }
    }
    if (values.empty())
    {
      return nullptr;
    }
    return ValueList::CreateCommaSeparated(krys::move(values));
  }

  // MARK: @font-face 'font-style'

  RefPtr<Value> ConsumeFontFaceFontStyle(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'font-style'> auto | normal | italic | oblique [ <angle [-90deg,90deg]>{1,2} ]?
    // https://drafts.csswg.org/css-fonts-4/#descdef-font-face-font-style

    // FIXME: Missing support for "auto" identifier.

    auto keyword = ConsumeIdentRaw<ValueId::Normal, ValueId::Italic, ValueId::Oblique>(tokens);
    if (!keyword)
    {
      return nullptr;
    }

    if (*keyword != ValueId::Oblique || tokens.IsAtEnd())
    {
      return FontStyleRangeValue::Create(PrimitiveValue::Create(*keyword));
    }

    auto tokensAfterAngles = tokens;

    auto firstAngle = ConsumeFontStyleAngleUnresolved(tokensAfterAngles, state);
    if (!firstAngle)
    {
      return nullptr;
    }

    if (tokensAfterAngles.IsAtEnd())
    {
      tokens = tokensAfterAngles;
      return FontStyleRangeValue::Create(
        PrimitiveValue::Create(*keyword),
        ValueList::CreateSpaceSeparated(ResolveToPrimitiveValue(krys::move(*firstAngle))));
    }

    auto secondAngle = ConsumeFontStyleAngleUnresolved(tokensAfterAngles, state);
    if (!secondAngle)
    {
      return nullptr;
    }

    tokens = tokensAfterAngles;
    return FontStyleRangeValue::Create(
      PrimitiveValue::Create(*keyword),
      ValueList::CreateSpaceSeparated(ResolveToPrimitiveValue(krys::move(*firstAngle)),
                                      ResolveToPrimitiveValue(krys::move(*secondAngle))));
  }

  // MARK: @font-face 'font-feature-settings'

  KRYS_NODISCARD static Maybe<FontTag> ConsumeFontOpenTypeTag(TokenRange &tokens) noexcept
  {
    // <opentype-tag> = <string>
    // https://drafts.csswg.org/css-fonts/#typedef-opentype-tag

    FontTag tag;

    const auto &token = tokens.Peek();
    if (token.Type() != TokenType::String)
    {
      return null;
    }
    auto value = token.IdentCodePoints();
    if (value.size() != tag.size())
    {
      return null;
    }

    for (size_t i = 0; i < tag.size(); ++i)
    {
      // Limits the range of characters to 0x20-0x7E, following the tag name rules defined in the OpenType
      // specification.
      auto character = value[i];
      if (character < 0x20 || character > 0x7E)
      {
        return null;
      }

      tag[i] = static_cast<char>(character);
    }

    tokens.Discard();
    tokens.DiscardWhitespace();

    return tag;
  }

  RefPtr<Value> ConsumeFeatureTagValue(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <feature-tag-value> = <opentype-tag> [ <integer [0,∞]> | on | off ]?
    // https://drafts.csswg.org/css-fonts/#feature-tag-value

    auto tag = ConsumeFontOpenTypeTag(tokens);
    if (!tag)
    {
      return nullptr;
    }

    RefPtr<PrimitiveValue> tagValue;
    if (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma)
    {
      // Feature tag values could follow: <integer [0,∞]> | on | off
      if (auto integer = PrimitiveValueResolver<Integer<NonNegative>>::ConsumeAndResolve(tokens, state))
      {
        tagValue = krys::move(integer);
      }
      else if (tokens.Peek().ValueId() == ValueId::On || tokens.Peek().ValueId() == ValueId::Off)
      {
        tagValue =
          PrimitiveValue::CreateInteger(tokens.ConsumeIncludingWhitespace().ValueId() == ValueId::On ? 1 : 0);
      }
      else
      {
        return nullptr;
      }
    }
    else
    {
      tagValue = PrimitiveValue::CreateInteger(1);
    }

    return FontFeatureValue::Create(krys::move(*tag), krys::move(tagValue));
  }

  // MARK: @font-face 'font-variation-settings'

  RefPtr<Value> ConsumeVariationTagValue(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <variation-tag-value> = <opentype-tag> <number>
    // https://drafts.csswg.org/css-fonts/#font-variation-settings-def

    auto tag = ConsumeFontOpenTypeTag(tokens);
    if (!tag)
    {
      return nullptr;
    }

    auto tagValue = PrimitiveValueResolver<Number<>>::ConsumeAndResolve(tokens, state);
    if (!tagValue)
    {
      return nullptr;
    }

    return FontVariationValue::Create(krys::move(*tag), krys::move(tagValue));
  }

  // MARK: - @font-feature-values

  // MARK: @font-feature-values 'prelude family name list'

  List<CSSOMStringAtom> ConsumeFontFeatureValuesPreludeFamilyNameList(TokenRange &tokens,
                                                                      const ParserContext &) noexcept
  {
    // <prelude-family-name-list> = <family-name>#
    // https://drafts.csswg.org/css-fonts/#font-feature-values-syntax

    List<CSSOMStringAtom> result;
    do
    {
      auto name = ConsumeFamilyNameUnresolved(tokens);
      if (!name)
      {
        return {};
      }

      result.push_back(krys::move(*name));
    } while (ConsumeComma(tokens));
    return result;
  }
}
