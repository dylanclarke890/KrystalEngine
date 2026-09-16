#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Parser/CustomPropertySyntax.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Parser/VariableParser.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/List.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ResolutionDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/String.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/TimeDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.Booey/CSS/Properties/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserResult.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/PendingSubstitutionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"
#include "Krystal.Booey/CSS/Values/UrlValue.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include <memory>

#include "StyleBuilder.h"
#include "StyleCustomProperty.h"
#include "StylePrimitiveNumericTypes+CSSValueConversion.h"
#include "StylePropertyShorthand.h"
#include "StylePropertyShorthandFunctions.h"

namespace krys::boo::css
{
#pragma region Custom Properties

  KRYS_NODISCARD static Pair<RefPtr<Value>, CustomPropertySyntax::Type>
    ConsumeCustomPropertyValueWithSyntax(TokenRange &tokens, PropertyParserState &state,
                                         const CustomPropertySyntax &syntax) noexcept;

  KRYS_NODISCARD static Maybe<Variant<Ref<const style::CustomProperty>, GlobalKeyword>>
    ConsumeTypedCustomPropertyValue(TokenRange &tokens, PropertyParserState &state,
                                    const CSSOMStringAtom &name, const CustomPropertySyntax &syntax,
                                    style::BuilderState &builderState) noexcept;

#pragma endregion

#pragma region Root Consumers

  // Style properties.
  KRYS_NODISCARD static bool ConsumeStyleProperty(TokenRange &, const ParserContext &, PropertyId,
                                                  IsImportant, RuleType, PropertyParserResult &) noexcept;

  // @font-face descriptors.
  KRYS_NODISCARD static bool ConsumeFontFaceDescriptor(TokenRange &, const ParserContext &, PropertyId,
                                                       PropertyParserResult &) noexcept;

  // @font-palette-values descriptors.
  KRYS_NODISCARD static bool ConsumeFontPaletteValuesDescriptor(TokenRange &, const ParserContext &,
                                                                PropertyId, PropertyParserResult &) noexcept;

  // @counter-style descriptors.
  KRYS_NODISCARD static bool ConsumeCounterStyleDescriptor(TokenRange &, const ParserContext &, PropertyId,
                                                           PropertyParserResult &) noexcept;

  // @keyframe descriptors.
  KRYS_NODISCARD static bool ConsumeKeyframeDescriptor(TokenRange &, const ParserContext &, PropertyId,
                                                       IsImportant, PropertyParserResult &) noexcept;

  // @page descriptors.
  KRYS_NODISCARD static bool ConsumePageDescriptor(TokenRange &, const ParserContext &, PropertyId,
                                                   IsImportant, PropertyParserResult &) noexcept;

  // @property descriptors.
  KRYS_NODISCARD static bool ConsumePropertyDescriptor(TokenRange &, const ParserContext &, PropertyId,
                                                       PropertyParserResult &) noexcept;

  // @view-transition descriptors.
  KRYS_NODISCARD static bool ConsumeViewTransitionDescriptor(TokenRange &, const ParserContext &, PropertyId,
                                                             PropertyParserResult &) noexcept;

  // @position-try descriptors.
  KRYS_NODISCARD static bool ConsumePositionTryDescriptor(TokenRange &, const ParserContext &, PropertyId,
                                                          IsImportant, PropertyParserResult &) noexcept;

  // @function descriptors.
  KRYS_NODISCARD static bool ConsumeFunctionDescriptor(TokenRange &, const ParserContext &, PropertyId,
                                                       PropertyParserResult &) noexcept;

  // @-internal-base-appearance descriptors.
  KRYS_NODISCARD static bool ConsumeInternalBaseAppearanceDescriptor(TokenRange &, const ParserContext &,
                                                                     PropertyId, IsImportant,
                                                                     PropertyParserResult &) noexcept;

#pragma endregion

#pragma region PropertyId Parsing

  // template <typename CharacterType>
  // KRYS_NODISCARD static PropertyId PropertyId(std::span<const CharacterType> characters)
  //{
  //   std::array<char, maxCSSPropertyNameLength> buffer;
  //   for (size_t i = 0; i != characters.size(); ++i)
  //   {
  //     auto character = characters[i];
  //     if (!character || !isASCII(character))
  //       return PropertyId::Invalid;
  //     buffer[i] = toASCIILower(character);
  //   }
  //   return findCSSProperty(buffer.data(), characters.size());
  // }

  // KRYS_NODISCARD PropertyId PropertyId(CSSOMStringView string)
  //{
  //   auto length = string.length();
  //   if (!length)
  //     return PropertyId::Invalid;
  //   if (length > maxCSSPropertyNameLength)
  //     return PropertyId::Invalid;
  //   return string.is8Bit() ? PropertyId(string.span8()) : PropertyId(string.span16());
  // }

  // MARK: - ValueId parsing

  // FIXME: Remove this mechanism entirely once we can do it without breaking the web.
  // KRYS_NODISCARD static bool isAppleLegacyCSSValueKeyword(std::span<const char> characters)
  //{
  //  return spanHasPrefix(characters.subspan(1), "apple-"_span)
  //         && !spanHasPrefix(characters.subspan(7), "system"_span)
  //         && !spanHasPrefix(characters.subspan(7), "pay"_span)
  //         && !spanHasPrefix(characters.subspan(7), "wireless"_span);
  //}

  // template <typename CharacterType>
  // KRYS_NODISCARD static ValueId cssValueKeywordID(std::span<const CharacterType> characters)
  //{
  //   ASSERT(!characters.empty()); // Otherwise buffer[0] would access uninitialized memory below.

  // std::array<char, maxCSSValueKeywordLength + 1> buffer; // 1 to turn "apple" into "webkit"

  // for (size_t i = 0uz; i != characters.size(); ++i)
  // {
  //   auto character = characters[i];
  //   if (!character || !isASCII(character))
  //     return CSSValueInvalid;
  //   buffer[i] = toASCIILower(character);
  // }

  // // In most cases, if the prefix is -apple-, change it to -webkit-. This makes the string one character
  // // longer.
  // auto length = characters.size();
  // std::span bufferSpan {buffer};
  // if (buffer[0] == '-' && isAppleLegacyCSSValueKeyword(bufferSpan.first(length)))
  // {
  //   memmoveSpan(bufferSpan.subspan(7), bufferSpan.subspan(6, length - 6));
  //   memcpySpan(bufferSpan.subspan(1), "webkit"_span);
  //   ++length;
  // }

  // return findCSSValueKeyword(bufferSpan.first(length));
  // }

  // KRYS_NODISCARD ValueId cssValueKeywordID(CSSOMStringView string)
  //{
  //   size_t length = string.length();
  //   if (!length)
  //     return CSSValueInvalid;
  //   if (length > maxCSSValueKeywordLength)
  //     return CSSValueInvalid;

  // return string.is8Bit() ? cssValueKeywordID(string.span8()) : cssValueKeywordID(string.span16());
  // }

#pragma endregion

#pragma region Custom Property Name Validation

  bool IsCustomPropertyName(CSSOMStringView propertyName) noexcept
  {
    return propertyName.length() > 2 && propertyName[0] == '-' && propertyName[1] == '-';
  }

#pragma endregion

#pragma region GlobalKeyword Consumer

  KRYS_NODISCARD static RefPtr<PrimitiveValue> ConsumeGlobalKeywordValue(TokenRange &range) noexcept
  {
    auto rangeCopy = range;
    auto valueId = rangeCopy.ConsumeIncludingWhitespace().ValueId();

    if (!rangeCopy.IsAtEnd())
    {
      return nullptr;
    }

    if (!IsGlobalKeyword(valueId))
    {
      return nullptr;
    }

    range = rangeCopy;
    return PrimitiveValue::Create(valueId);
  }

  KRYS_NODISCARD static Maybe<GlobalKeyword> ConsumeGlobalKeyword(TokenRange &range) noexcept
  {
    auto rangeCopy = range;
    auto valueId = rangeCopy.ConsumeIncludingWhitespace().ValueId();
    if (!rangeCopy.IsAtEnd())
    {
      return {};
    }

    auto keyword = ParseGlobalKeyword(valueId);
    if (!keyword)
    {
      return {};
    }

    range = rangeCopy;
    return keyword;
  }

#pragma endregion

#pragma region FunctionValue Consumer

  static bool ConsumeFunctionArgument(TokenRange &range, size_t index, PropertyId property,
                                      PropertyParserState &state, PropertyParserResult &result) noexcept
  {
    auto argument = PropertyParserHelpers::ConsumeArgument(range, index);
    if (!argument)
    {
      return false;
    }

    const auto &context = state.Context;
    auto important = state.Important;
    auto ruleType = state.CurrentRule;
    return ConsumeStyleProperty(*argument, context, property, important, ruleType, result);
  }

  static bool ConsumeInternalAutoBaseFunction(TokenRange &range, PropertyId property,
                                              PropertyParserState &state,
                                              PropertyParserResult &result) noexcept
  {
    // -internal-auto-base() = -internal-auto-base( <auto value>, <base value> )

    if (!state.Context.cssInternalAutoBaseParsingEnabled)
    {
      return false;
    }

    if (range.Peek().FunctionId() != ValueId::InternalAutoBase)
    {
      return false;
    }

    auto args = PropertyParserHelpers::ConsumeFunction(range);

    ParsedPropertyList autoProperties;
    PropertyParserResult autoResult {autoProperties};

    if (!ConsumeFunctionArgument(args, 0, property, state, autoResult))
    {
      return false;
    }

    ParsedPropertyList baseProperties;
    PropertyParserResult baseResult {baseProperties};

    if (!ConsumeFunctionArgument(args, 1, property, state, baseResult))
    {
      return false;
    }

    if (autoProperties.size() != baseProperties.size())
    {
      return false;
    }

    for (size_t index = 0uz; index < autoProperties.size(); ++index)
    {
      const Property &autoProperty = autoProperties[index];
      const Property &baseProperty = baseProperties[index];

      auto value = FunctionValue::Create(ValueId::InternalAutoBase, ShareRef(autoProperty.Value()),
                                         ShareRef(baseProperty.Value()));
      result.AddProperty(Property(autoProperty.Metadata(), krys::move(value)));
    }

    return true;
  }
#pragma endregion

#pragma region Parser entry points

  bool PropertyParser::ParseValue(PropertyId property, IsImportant important, TokenRange range,
                                  const ParserContext &context, ParsedPropertyList &parsedProperties,
                                  RuleType ruleType) noexcept
  {
    int initialParsedPropertiesSize = parsedProperties.size();

    range.DiscardWhitespace();

    PropertyParserResult result {parsedProperties};

    bool parseSuccess;
    switch (ruleType)
    {
      case RuleType::CounterStyle:
      {
        parseSuccess = ConsumeCounterStyleDescriptor(range, context, property, result);
        break;
      }
      case RuleType::FontFace:
      {
        parseSuccess = ConsumeFontFaceDescriptor(range, context, property, result);
        break;
      }
      case RuleType::FontPaletteValues:
      {
        parseSuccess = ConsumeFontPaletteValuesDescriptor(range, context, property, result);
        break;
      }
      case RuleType::Keyframe:
      {
        parseSuccess = ConsumeKeyframeDescriptor(range, context, property, important, result);
        break;
      }
      case RuleType::Page:
      {
        parseSuccess = ConsumePageDescriptor(range, context, property, important, result);
        break;
      }
      case RuleType::Property:
      {
        parseSuccess = ConsumePropertyDescriptor(range, context, property, result);
        break;
      }
      case RuleType::ViewTransition:
      {
        parseSuccess = ConsumeViewTransitionDescriptor(range, context, property, result);
        break;
      }
      case RuleType::PositionTry:
      {
        parseSuccess = ConsumePositionTryDescriptor(range, context, property, important, result);
        break;
      }
      case RuleType::Function:
      {
        parseSuccess = ConsumeFunctionDescriptor(range, context, property, result);
        break;
      }
      case RuleType::InternalBaseAppearance:
      {
        parseSuccess = ConsumeInternalBaseAppearanceDescriptor(range, context, property, important, result);
        break;
      }
      default:
      {
        parseSuccess = ConsumeStyleProperty(range, context, property, important, ruleType, result);
        break;
      }
    }

    if (!parseSuccess)
    {
      // TODO: webkit used shrink here, check this is correct.
      parsedProperties.resize(initialParsedPropertiesSize);
    }

    return parseSuccess;
  }

  RefPtr<Value> PropertyParser::ParseStylePropertyLonghand(PropertyId property, const CSSOMString &string,
                                                           const ParserContext &context) noexcept
  {
    krys_assert(!Property::IsShorthand(property));

    if (string.empty())
    {
      return nullptr;
    }

    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = RuleType::Style,
      .Important = IsImportant(false),
    };

    if (auto value = ParserFastPaths::MaybeParseValue(property, string, state))
    {
      return ShareRefPtr(&*value);
    }

    InputStream inputStream {CSSOMString(string)};
    Tokenizer tokenizer(inputStream);

    auto range = tokenizer.Tokens();
    range.DiscardWhitespace();

    if (auto value = ConsumeGlobalKeywordValue(range))
    {
      return value;
    }

    auto value = PropertyParsing::ParseStylePropertyLonghand(range, property, state);
    if (!value || !range.IsAtEnd())
    {
      return nullptr;
    }

    return value;
  }

  RefPtr<Value> PropertyParser::ParseStylePropertyLonghand(PropertyId property, TokenRange range,
                                                           const ParserContext &context) noexcept
  {
    krys_assert(!Property::IsShorthand(property));

    range.DiscardWhitespace();

    if (auto value = ConsumeGlobalKeywordValue(range))
    {
      return value;
    }

    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = RuleType::Style,
      .Important = IsImportant(false),
    };

    auto value = PropertyParsing::ParseStylePropertyLonghand(range, property, state);
    if (!value || !range.IsAtEnd())
    {
      return nullptr;
    }

    return value;
  }

  RefPtr<Value> PropertyParser::ParseCounterStyleDescriptor(PropertyId property, const CSSOMString &string,
                                                            const ParserContext &context) noexcept
  {
    InputStream inputStream {CSSOMString(string)};
    Tokenizer tokenizer(inputStream);
    auto range = tokenizer.Tokens();

    // Handle leading whitespace.
    range.DiscardWhitespace();

    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = RuleType::CounterStyle,
      .Important = IsImportant(false),
    };

    auto result = PropertyParsing::ParseCounterStyleDescriptor(range, property, state);

    // Handle trailing whitespace.
    range.DiscardWhitespace();
    if (!range.IsAtEnd())
    {
      return nullptr;
    }

    return result;
  }

#pragma endregion

#pragma region Custom Properties

  Maybe<Variant<Ref<const style::CustomProperty>, GlobalKeyword>>
    PropertyParser::ParseTypedCustomPropertyValue(const CSSOMStringAtom &name,
                                                  const CustomPropertySyntax &syntax, TokenRange range,
                                                  style::BuilderState &builderState,
                                                  const ParserContext &context) noexcept
  {
    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = PropertyId::Custom,
      .CurrentRule = RuleType::Style,
      .Important = IsImportant(false),
    };

    auto value = ConsumeTypedCustomPropertyValue(range, state, name, syntax, builderState);
    if (!value || !range.IsAtEnd())
    {
      return {};
    }

    return value;
  }

  RefPtr<const style::CustomProperty> PropertyParser::ParseTypedCustomPropertyInitialValue(
    const CSSOMStringAtom &name, const CustomPropertySyntax &syntax, TokenRange range,
    style::BuilderState &builderState, const ParserContext &context) noexcept
  {
    if (syntax.IsUniversal())
    {
      return VariableParser::ParseInitialValueForUniversalSyntax(name, range);
    }

    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = PropertyId::Custom,
      .CurrentRule = RuleType::Style,
      .Important = IsImportant(false),
    };

    auto value = ConsumeTypedCustomPropertyValue(range, state, name, syntax, builderState);
    if (!value || !range.IsAtEnd())
    {
      return {};
    }

    return krys::SwitchOn(
      *value, [](const Ref<const style::CustomProperty> &resolved) -> RefPtr<const style::CustomProperty>
      { return resolved; },
      [](const GlobalKeyword &) -> RefPtr<const style::CustomProperty> { return nullptr; });
  }

  ComputedStyleDependencies PropertyParser::CollectParsedCustomPropertyValueDependencies(
    const CustomPropertySyntax &syntax, TokenRange range, const ParserContext &context) noexcept
  {
    if (syntax.IsUniversal())
    {
      return {};
    }

    range.DiscardWhitespace();

    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = PropertyId::Custom,
      .CurrentRule = RuleType::Style,
      .Important = IsImportant(false),
    };

    auto [value, syntaxType] = ConsumeCustomPropertyValueWithSyntax(range, state, syntax);
    if (!value)
    {
      return {};
    }

    return value->ComputedStyleDependencies();
  }

  bool PropertyParser::IsValidCustomPropertyValueForSyntax(const CustomPropertySyntax &syntax,
                                                           TokenRange range,
                                                           const ParserContext &context) noexcept
  {
    if (syntax.IsUniversal())
    {
      return true;
    }

    range.DiscardWhitespace();

    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = PropertyId::Custom,
      .CurrentRule = RuleType::Style,
      .Important = IsImportant(false),
    };

    return !!ConsumeCustomPropertyValueWithSyntax(range, state, syntax).first;
  }

  Maybe<GlobalKeyword> PropertyParser::ParseGlobalKeyword(TokenRange tokens) noexcept
  {
    return ConsumeGlobalKeyword(tokens);
  }

  Pair<RefPtr<Value>, CustomPropertySyntax::Type>
    ConsumeCustomPropertyValueWithSyntax(TokenRange &range, PropertyParserState &state,
                                         const CustomPropertySyntax &syntax) noexcept
  {
    krys_assert(!syntax.IsUniversal());

    auto rangeCopy = range;

    auto ConsumeSingleValue = [&](auto &range, auto &component) -> RefPtr<Value>
    {
      switch (component.type)
      {
        case CustomPropertySyntax::Type::Length:
        {
          return PrimitiveValueResolver<Length<>>::ConsumeAndResolve(range, state);
        }
        case CustomPropertySyntax::Type::LengthPercentage:
        {
          return PrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(range, state);
        }
        case CustomPropertySyntax::Type::CustomIdent:
        {
          if (RefPtr value = PropertyParserHelpers::ConsumeCustomIdent(range))
          {
            if (component.ident == CSSOMStringAtom::Null() || value->StringValue() == component.ident)
            {
              return value;
            }
          }

          return nullptr;
        }
        case CustomPropertySyntax::Type::Percentage:
        {
          return PrimitiveValueResolver<Percentage<>>::ConsumeAndResolve(range, state);
        }
        case CustomPropertySyntax::Type::Integer:
        {
          return PrimitiveValueResolver<Integer<>>::ConsumeAndResolve(range, state);
        }
        case CustomPropertySyntax::Type::Number:
        {
          return PrimitiveValueResolver<Number<>>::ConsumeAndResolve(range, state);
        }
        case CustomPropertySyntax::Type::Angle:
        {
          return PrimitiveValueResolver<Angle<>>::ConsumeAndResolve(range, state);
        }
        case CustomPropertySyntax::Type::Time:
        {
          return PrimitiveValueResolver<Time<>>::ConsumeAndResolve(range, state);
        }
        case CustomPropertySyntax::Type::Resolution:
        {
          return PrimitiveValueResolver<Resolution<>>::ConsumeAndResolve(range, state);
        }
        case CustomPropertySyntax::Type::Color:
        {
          return PropertyParserHelpers::ConsumeColor(range, state);
        }
        case CustomPropertySyntax::Type::Image:
        {
          return PropertyParserHelpers::ConsumeImage(
            range, state,
            PropertyParserHelpers::AllowedImageType::URLFunction
              | PropertyParserHelpers::AllowedImageType::GeneratedImage);
        }
        case CustomPropertySyntax::Type::URL:
        {
          return PropertyParserHelpers::ConsumeURL(range, state, {});
        }
        case CustomPropertySyntax::Type::String:
        {
          return PropertyParserHelpers::ConsumeString(range);
        }
        case CustomPropertySyntax::Type::TransformFunction:
        {
          return PropertyParsing::ConsumeTransformFunction(range, state);
        }
        case CustomPropertySyntax::Type::TransformList:
        {
          return PropertyParsing::ConsumeTransformList(range, state);
        }
        case CustomPropertySyntax::Type::Unknown:
        {
          return nullptr;
        }
      }

      krys_unreachable();
    };

    auto ConsumeComponent = [&](auto &range, const auto &component) -> RefPtr<Value>
    {
      switch (component.multiplier)
      {
        case CustomPropertySyntax::Multiplier::Single:
        {
          return ConsumeSingleValue(range, component);
        }
        case CustomPropertySyntax::Multiplier::CommaList:
        {
          return PropertyParserHelpers::ConsumeListSeparatedBy<',', PropertyParserHelpers::OneOrMore>(
            range, [&](auto &range) { return ConsumeSingleValue(range, component); });
        }
        case CustomPropertySyntax::Multiplier::SpaceList:
        {
          return PropertyParserHelpers::ConsumeListSeparatedBy<' ', PropertyParserHelpers::OneOrMore>(
            range, [&](auto &range) { return ConsumeSingleValue(range, component); });
        }
      }

      krys_unreachable();
    };

    for (auto &component : syntax.Definition)
    {
      if (RefPtr value = ConsumeComponent(range, component))
      {
        if (range.IsAtEnd())
        {
          return {value, component.type};
        }
      }

      range = rangeCopy;
    }

    return {nullptr, CustomPropertySyntax::Type::Unknown};
  }

  Maybe<Variant<Ref<const style::CustomProperty>, GlobalKeyword>>
    ConsumeTypedCustomPropertyValue(TokenRange &range, PropertyParserState &state,
                                    const CSSOMStringAtom &name, const CustomPropertySyntax &syntax,
                                    style::BuilderState &builderState) noexcept
  {
    if (syntax.IsUniversal())
    {
      return {{style::CustomProperty::CreateForVariableData(name, VariableData::Create(range.ConsumeAll()))}};
    }

    range.DiscardWhitespace();

    if (auto keyword = ConsumeGlobalKeyword(range))
    {
      return {{*keyword}};
    }

    auto [value, syntaxType] = ConsumeCustomPropertyValueWithSyntax(range, state, syntax);
    if (!value)
    {
      return {};
    }

    auto ResolveSyntaxValue = [&, syntaxType =
                                    syntaxType](const Value &value) -> Maybe<style::CustomProperty::Value>
    {
      switch (syntaxType)
      {
        case CustomPropertySyntax::Type::LengthPercentage:
        {
          return style::toStyleFromCSSValue<style::LengthPercentage<>>(builderState,
                                                                       Downcast<PrimitiveValue>(value));
        }
        case CustomPropertySyntax::Type::Length:
        {
          return style::toStyleFromCSSValue<style::Length<>>(builderState, Downcast<PrimitiveValue>(value));
        }
        case CustomPropertySyntax::Type::Integer:
        case CustomPropertySyntax::Type::Number:
        {
          return style::toStyleFromCSSValue<style::Number<>>(builderState, Downcast<PrimitiveValue>(value));
        }
        case CustomPropertySyntax::Type::Percentage:
        {
          return style::toStyleFromCSSValue<style::Percentage<>>(builderState,
                                                                 Downcast<PrimitiveValue>(value));
        }
        case CustomPropertySyntax::Type::Angle:
        {
          return style::toStyleFromCSSValue<style::Angle<>>(builderState, Downcast<PrimitiveValue>(value));
        }
        case CustomPropertySyntax::Type::Time:
        {
          return style::toStyleFromCSSValue<style::Time<>>(builderState, Downcast<PrimitiveValue>(value));
        }
        case CustomPropertySyntax::Type::Resolution:
        {
          return style::toStyleFromCSSValue<style::Resolution<>>(builderState,
                                                                 Downcast<PrimitiveValue>(value));
        }
        case CustomPropertySyntax::Type::Color:
        {
          return style::toStyleFromCSSValue<style::Color>(builderState, value, style::ForVisitedLink::No);
        }
        case CustomPropertySyntax::Type::Image:
        {
          auto styleImage = builderState.createStyleImage(value);
          if (!styleImage)
          {
            return {};
          }

          return Style::ImageWrapper {styleImage.releaseNonNull()};
        }
        case CustomPropertySyntax::Type::URL:
        {
          return Style::toStyle(downcast<CSSURLValue>(value).url(), builderState);
        }
        case CustomPropertySyntax::Type::CustomIdent:
        {
          return CustomIdentifier {CSSOMStringAtom {downcast<PrimitiveValue>(value).stringValue()}};
        }
        case CustomPropertySyntax::Type::String:
        {
          return Downcast<PrimitiveValue>(value).StringValue();
        }
        case CustomPropertySyntax::Type::TransformFunction:
        case CustomPropertySyntax::Type::TransformList:
        {
          return Style::toStyleFromCSSValue<Style::TransformFunction>(builderState, value);
        }
        case CustomPropertySyntax::Type::Unknown: return {};
      }

      krys_unreachable();
    };

    if (Is<ValueList>(value.get()) || Is<TransformListValue>(value.get()))
    {
      auto *valueList = Downcast<ValueWithSmallList>(value.get());
      auto syntaxValueList = style::CustomProperty::ValueList {{}, valueList->separator()};
      for (Ref listValue : valueList)
      {
        auto syntaxValue = ResolveSyntaxValue(listValue);
        if (!syntaxValue)
        {
          return {};
        }
        syntaxValueList.values.append(krys::move(*syntaxValue));
      }

      return {{style::CustomProperty::CreateForValueList(name, krys::move(syntaxValueList))}};
    };

    auto syntaxValue = ResolveSyntaxValue(*value);
    if (!syntaxValue)
    {
      return {};
    }

    return {{style::CustomProperty::CreateForValue(name, krys::move(*syntaxValue))}};
  }

#pragma endregion

#pragma region Root Consumers

  bool ConsumeStyleProperty(TokenRange &range, const ParserContext &context, PropertyId property,
                            IsImportant important, RuleType ruleType, PropertyParserResult &result)
  {
    if (Property::IsDescriptorOnly(property))
    {
      return false;
    }

    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = ruleType,
      .Important = important,
    };

    if (ConsumeInternalAutoBaseFunction(range, property, state, result))
      return true;

    if (Property::IsShorthand(property))
    {
      auto rangeCopy = range;
      if (RefPtr keywordValue = ConsumeGlobalKeywordValue(rangeCopy))
      {
        result.AddPropertyForAllLonghandsOfCurrentShorthand(state, krys::move(keywordValue));
        range = rangeCopy;
        return true;
      }

      auto originalRange = range;

      if (PropertyParsing::ParseStylePropertyShorthand(range, property, state, result))
      {
        return true;
      }

      if (VariableParser::ContainsValidVariableReferences(originalRange, context))
      {
        result.AddPropertyForAllLonghandsOfCurrentShorthand(
          state,
          PendingSubstitutionValue::Create(property, VariableReferenceValue::Create(originalRange, context)));
        return true;
      }
    }
    else
    {
      auto rangeCopy = range;
      if (RefPtr keywordValue = ConsumeGlobalKeywordValue(rangeCopy))
      {
        result.AddProperty(state, property, PropertyId::Invalid, krys::move(keywordValue), important);
        range = rangeCopy;
        return true;
      }

      auto originalRange = range;

      RefPtr parsedValue = PropertyParsing::ParseStylePropertyLonghand(range, property, state);
      if (parsedValue && range.IsAtEnd())
      {
        result.AddProperty(state, property, PropertyId::Invalid, krys::move(parsedValue), important);
        return true;
      }

      if (VariableParser::ContainsValidVariableReferences(originalRange, context))
      {
        result.AddProperty(state, property, PropertyId::Invalid,
                           VariableReferenceValue::Create(originalRange, context), important);
        return true;
      }
    }

    return false;
  }

  bool ConsumeFontFaceDescriptor(TokenRange &range, const ParserContext &context, PropertyId property,
                                 PropertyParserResult &result)
  {
    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = RuleType::FontFace,
      .Important = IsImportant(false),
    };

    RefPtr parsedValue = PropertyParsing::ParseFontFaceDescriptor(range, property, state);
    if (!parsedValue || !range.IsAtEnd())
    {
      return false;
    }

    result.AddProperty(state, property, PropertyId::Invalid, krys::move(parsedValue), IsImportant(false));
    return true;
  }

  bool ConsumeFontPaletteValuesDescriptor(TokenRange &range, const ParserContext &context,
                                          PropertyId property, PropertyParserResult &result)
  {
    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = RuleType::FontPaletteValues,
      .Important = IsImportant(false),
    };

    RefPtr parsedValue = PropertyParsing::ParseFontPaletteValuesDescriptor(range, property, state);
    if (!parsedValue || !range.IsAtEnd())
    {
      return false;
    }

    result.AddProperty(state, property, PropertyId::Invalid, krys::move(parsedValue), IsImportant(false));
    return true;
  }

  bool ConsumeCounterStyleDescriptor(TokenRange &range, const ParserContext &context, PropertyId property,
                                     PropertyParserResult &result)
  {
    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = RuleType::CounterStyle,
      .Important = IsImportant(false),
    };

    RefPtr parsedValue = PropertyParsing::ParseCounterStyleDescriptor(range, property, state);
    if (!parsedValue || !range.IsAtEnd())
    {
      return false;
    }

    result.AddProperty(state, property, PropertyId::Invalid, krys::move(parsedValue), IsImportant(false));
    return true;
  }

  bool ConsumeKeyframeDescriptor(TokenRange &range, const ParserContext &context, PropertyId property,
                                 IsImportant important, PropertyParserResult &result)
  {
    // https://www.w3.org/TR/css-animations-1/#keyframes
    // The <declaration-list> inside of <keyframe-block> accepts any CSS property except those
    // defined in this specification, but does accept the animation-timing-function property and
    // interprets it specially.
    switch (property)
    {
      case PropertyId::Animation:
      case PropertyId::AnimationDelay:
      case PropertyId::AnimationDirection:
      case PropertyId::AnimationDuration:
      case PropertyId::AnimationFillMode:
      case PropertyId::AnimationIterationCount:
      case PropertyId::AnimationName:
      case PropertyId::AnimationPlayState:      return false;
      default:                                  return ConsumeStyleProperty(range, context, property, important, RuleType::Keyframe, result);
    }
  }

  bool ConsumePageDescriptor(TokenRange &range, const ParserContext &context, PropertyId property,
                             IsImportant important, PropertyParserResult &result)
  {
    // Does not apply in @page per-spec.
    if (property == PropertyId::Page)
    {
      return false;
    }

    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = RuleType::Page,
      .Important = IsImportant(false),
    };

    if (RefPtr parsedValue = PropertyParsing::ParsePageDescriptor(range, property, state))
    {
      if (!range.IsAtEnd())
      {
        return false;
      }

      result.AddProperty(state, property, PropertyId::Invalid, krys::move(parsedValue), IsImportant(false));
      return true;
    }

    return ConsumeStyleProperty(range, context, property, important, RuleType::Page, result);
  }

  bool ConsumePropertyDescriptor(TokenRange &range, const ParserContext &context, PropertyId property,
                                 PropertyParserResult &result)
  {
    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = RuleType::Property,
      .Important = IsImportant(false),
    };

    RefPtr parsedValue = PropertyParsing::ParsePropertyDescriptor(range, property, state);
    if (!parsedValue || !range.IsAtEnd())
    {
      return false;
    }

    result.AddProperty(state, property, PropertyId::Invalid, krys::move(parsedValue), IsImportant(false));
    return true;
  }

  bool ConsumeViewTransitionDescriptor(TokenRange &range, const ParserContext &context, PropertyId property,
                                       PropertyParserResult &result)
  {
    krys_debug_assert(context.PropertySettings.crossDocumentViewTransitionsEnabled);

    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = RuleType::ViewTransition,
      .Important = IsImportant(false),
    };

    RefPtr parsedValue = PropertyParsing::ParseViewTransitionDescriptor(range, property, state);
    if (!parsedValue || !range.IsAtEnd())
    {
      return false;
    }

    result.AddProperty(state, property, PropertyId::Invalid, krys::move(parsedValue), IsImportant(false));
    return true;
  }

  // Checks whether a CSS property is allowed in @position-try.
  KRYS_NODISCARD static bool PropertyAllowedInPositionTryRule(PropertyId property) noexcept
  {
    return Property::IsInsetProperty(property) || Property::IsMarginProperty(property)
           || Property::IsSizingProperty(property) || property == PropertyId::AlignSelf
           || property == PropertyId::JustifySelf || property == PropertyId::PlaceSelf
           || property == PropertyId::PositionAnchor || property == PropertyId::PositionArea;
  }

  bool ConsumePositionTryDescriptor(TokenRange &range, const ParserContext &context, PropertyId property,
                                    IsImportant important, PropertyParserResult &result) noexcept
  {
    krys_debug_assert(context.PropertySettings.cssAnchorPositioningEnabled);

    // Per spec, !important is not allowed and makes the whole declaration invalid.
    if (important)
    {
      return false;
    }

    if (!PropertyAllowedInPositionTryRule(property))
    {
      return false;
    }

    return ConsumeStyleProperty(range, context, property, important, RuleType::PositionTry, result);
  }

  bool ConsumeFunctionDescriptor(TokenRange &range, const ParserContext &context, PropertyId property,
                                 PropertyParserResult &result) noexcept
  {
    krys_debug_assert(context.PropertySettings.cssFunctionAtRuleEnabled);

    auto state = PropertyParserState {
      .Context = context,
      .CurrentProperty = property,
      .CurrentRule = RuleType::Function,
      .Important = IsImportant(false),
    };

    RefPtr parsedValue = PropertyParsing::ParseFunctionDescriptor(range, property, state);
    if (!parsedValue || !range.IsAtEnd())
    {
      return false;
    }

    result.AddProperty(state, property, PropertyId::Invalid, krys::move(parsedValue), IsImportant(false));
    return true;
  }

  bool ConsumeInternalBaseAppearanceDescriptor(TokenRange &range, const ParserContext &context,
                                               PropertyId property, IsImportant important,
                                               PropertyParserResult &result) noexcept
  {
    krys_debug_assert(context.Mode == ParserMode::UASheet);

    if (property == PropertyId::Appearance)
    {
      return false;
    }

    return ConsumeStyleProperty(range, context, property, important, RuleType::InternalBaseAppearance,
                                result);
  }

#pragma endregion
}
