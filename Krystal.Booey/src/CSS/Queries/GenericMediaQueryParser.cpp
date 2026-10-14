#include "Krystal.Booey/CSS/Queries/GenericMediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Parser/VariableParser.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ratio.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ResolutionDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::css::mq
{
  KRYS_NODISCARD static CSSOMStringAtom ConsumeFeatureName(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident)
    {
      return CSSOMStringAtom::Null();
    }
    auto name = tokens.ConsumeIncludingWhitespace().IdentCodePoints();
    if (IsCustomPropertyName(name))
    {
      return CSSOMStringAtom(name);
    }
    return CSSOMStringAtom(text::ToASCIILower(name));
  }

  Maybe<Feature> FeatureParser::ConsumeFeature(TokenRange &tokens,
                                               const MediaQueryParserContext &context) noexcept
  {
    auto rangeCopy = tokens;
    if (auto feature = ConsumeBooleanOrPlainFeature(tokens, context))
    {
      return feature;
    }

    tokens = rangeCopy;
    return ConsumeRangeFeature(tokens, context);
  }

  KRYS_NODISCARD static RefPtr<Value>
    ConsumeCustomPropertyValue(const CSSOMStringAtom &propertyName, TokenRange &tokens,
                               const MediaQueryParserContext &context) noexcept
  {
    auto valueRange = tokens;
    std::ignore = tokens.ConsumeAll();

    // Syntax is that of a valid declaration so !important is allowed. It just gets ignored.
    std::ignore = Parser::ConsumeTrailingImportantAndWhitespace(valueRange);

    if (valueRange.IsAtEnd())
    {
      return CustomPropertyValue::CreateEmpty(propertyName);
    }

    return VariableParser::ParseDeclarationValue(propertyName, valueRange, context.Context);
  }

  Maybe<Feature> FeatureParser::ConsumeBooleanOrPlainFeature(TokenRange &tokens,
                                                             const MediaQueryParserContext &context) noexcept
  {
    auto consumePlainFeatureName = [&]() -> Pair<CSSOMStringAtom, ComparisonOperator>
    {
      auto name = ConsumeFeatureName(tokens);
      if (name.View().empty())
      {
        return {CSSOMStringAtom::Null(), ComparisonOperator::Equal};
      }
      if (IsCustomPropertyName(name.View()))
      {
        return {name, ComparisonOperator::Equal};
      }
      if (name.View().starts_with(u8"min-"))
      {
        return {CSSOMStringAtom(name.View().substr(4)), ComparisonOperator::GreaterThanOrEqual};
      }
      if (name.View().starts_with(u8"max-"))
      {
        return {CSSOMStringAtom(name.View().substr(4)), ComparisonOperator::LessThanOrEqual};
      }

      return {name, ComparisonOperator::Equal};
    };

    auto [featureName, op] = consumePlainFeatureName();
    if (featureName.View().empty())
    {
      return {};
    }

    tokens.DiscardWhitespace();

    if (tokens.IsAtEnd())
    {
      if (op != ComparisonOperator::Equal)
      {
        return {};
      }

      return Feature {featureName, Syntax::Boolean, {}, {}};
    }

    if (tokens.Peek().Type() != TokenType::Colon)
    {
      return {};
    }

    tokens.Discard();
    tokens.DiscardWhitespace();

    auto value = IsCustomPropertyName(featureName.View())
                   ? ConsumeCustomPropertyValue(featureName, tokens, context)
                   : ConsumeValue(tokens, context);

    if (!value)
    {
      return {};
    }

    if (!tokens.IsAtEnd())
    {
      return {};
    }

    return Feature {featureName, Syntax::Plain, {}, Comparison {op, krys::move(value)}};
  }

  Maybe<Feature> FeatureParser::ConsumeRangeFeature(TokenRange &tokens,
                                                    const MediaQueryParserContext &context) noexcept
  {
    auto consumeRangeOperator = [&]() -> Maybe<ComparisonOperator>
    {
      if (tokens.IsAtEnd())
      {
        return {};
      }
      auto &opToken = tokens.Consume();
      if (tokens.IsAtEnd() || opToken.Type() != TokenType::Delim)
      {
        return {};
      }

      auto isEqualsDelimiter = [&]
      {
        return tokens.Peek().Type() == TokenType::Delim && tokens.Peek().Delimiter() == U'=';
      };

      switch (opToken.Delimiter())
      {
        case U'=': tokens.DiscardWhitespace(); return ComparisonOperator::Equal;
        case U'<':
          if (isEqualsDelimiter())
          {
            tokens.Discard();
            tokens.DiscardWhitespace();
            return ComparisonOperator::LessThanOrEqual;
          }
          tokens.DiscardWhitespace();
          return ComparisonOperator::LessThan;
        case U'>':
          if (isEqualsDelimiter())
          {
            tokens.Discard();
            tokens.DiscardWhitespace();
            return ComparisonOperator::GreaterThanOrEqual;
          }
          tokens.DiscardWhitespace();
          return ComparisonOperator::GreaterThan;
        default: return {};
      }
    };

    bool didFailParsing = false;

    auto consumeLeftComparison = [&]() -> Maybe<Comparison>
    {
      if (tokens.Peek().Type() == TokenType::Ident)
      {
        return {};
      }
      auto value = ConsumeValue(tokens, context);
      if (!value)
      {
        return {};
      }
      auto op = consumeRangeOperator();
      if (!op)
      {
        didFailParsing = true;
        return {};
      }

      return Comparison {*op, krys::move(value)};
    };

    auto consumeRightComparison = [&]() -> Maybe<Comparison>
    {
      auto op = consumeRangeOperator();
      if (!op)
      {
        return {};
      }
      auto value = ConsumeValue(tokens, context);
      if (!value)
      {
        didFailParsing = true;
        return {};
      }

      return Comparison {*op, krys::move(value)};
    };

    auto leftComparison = consumeLeftComparison();

    auto featureName = ConsumeFeatureName(tokens);
    if (featureName.View().empty())
    {
      return {};
    }

    auto rightComparison = consumeRightComparison();

    auto validateComparisons = [&]
    {
      if (didFailParsing)
      {
        return false;
      }
      if (!leftComparison && !rightComparison)
      {
        return false;
      }
      if (!leftComparison || !rightComparison)
      {
        return true;
      }
      // Disallow comparisons like (a=b=c), (a=b<c).
      if (leftComparison->Op == ComparisonOperator::Equal || rightComparison->Op == ComparisonOperator::Equal)
      {
        return false;
      }
      // Disallow comparisons like (a<b>c).
      bool leftIsLess = leftComparison->Op == ComparisonOperator::LessThan
                        || leftComparison->Op == ComparisonOperator::LessThanOrEqual;
      bool rightIsLess = rightComparison->Op == ComparisonOperator::LessThan
                         || rightComparison->Op == ComparisonOperator::LessThanOrEqual;
      return leftIsLess == rightIsLess;
    };

    if (!tokens.IsAtEnd() || !validateComparisons())
    {
      return {};
    }

    return Feature {featureName, Syntax::Range, krys::move(leftComparison), krys::move(rightComparison)};
  }

  RefPtr<Value> FeatureParser::ConsumeValue(TokenRange &tokens,
                                            const MediaQueryParserContext &context) noexcept
  {
    using namespace PropertyParserHelpers;

    if (tokens.IsAtEnd())
    {
      return nullptr;
    }

    if (auto value = ConsumeIdent(tokens))
    {
      return value;
    }

    auto state = PropertyParserState {.Context = context.Context};

    if (auto value = ConsumeRatioWithBothNumeratorAndDenominator(tokens, state))
    {
      return value;
    }
    if (auto value = PrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state))
    {
      return value;
    }
    if (auto value = PrimitiveValueResolver<Number<>>::ConsumeAndResolve(tokens, state))
    {
      return value;
    }
    // FIXME: Figure out and document why overrideParserMode is explicitly set to HTMLStandardMode here.
    if (auto value = PrimitiveValueResolver<Length<>>::ConsumeAndResolve(
          tokens, state, {.OverrideParserMode = ParserMode::HTMLStandard}))
    {
      return value;
    }
    if (auto value = PrimitiveValueResolver<Resolution<>>::ConsumeAndResolve(tokens, state))
    {
      return value;
    }

    return nullptr;
  }

  bool FeatureParser::ValidateFeatureAgainstSchema(Feature &feature, const FeatureSchema &schema) noexcept
  {
    auto validateValue = [&](RefPtr<Value> &value)
    {
      auto *primitiveValue = value ? DynamicDowncast<PrimitiveValue>(*value) : nullptr;
      switch (schema.FeatureValueType)
      {
        case FeatureSchema::ValueType::Integer: return primitiveValue && primitiveValue->IsInteger();

        case FeatureSchema::ValueType::Number:  return primitiveValue && primitiveValue->IsNumberOrInteger();

        case FeatureSchema::ValueType::Length:
          if (!primitiveValue)
          {
            return false;
          }
          if (primitiveValue->IsInteger() && !primitiveValue->ResolveAsIntegerDeprecated())
          {
            return true;
          }
          return primitiveValue->IsLength();

        case FeatureSchema::ValueType::Resolution: return primitiveValue && primitiveValue->IsResolution();

        case FeatureSchema::ValueType::Identifier:
          return primitiveValue && primitiveValue->IsValueId()
                 && schema.ValueIdentifiers.contains(primitiveValue->ValueId());

        case FeatureSchema::ValueType::Ratio:
          if (primitiveValue && primitiveValue->IsNumberOrInteger())
          {
            auto number = primitiveValue->ResolveAsNumberDeprecated<double>();
            if (number < 0)
            {
              return false;
            }
            value = RatioValue::Create(Ratio {Number<NonNegative> {number}, Number<NonNegative> {1.0}});
            return true;
          }
          return value && Is<RatioValue>(*value);

        case FeatureSchema::ValueType::CustomProperty: return value && value->IsCustomPropertyValue();
      }
      krys_debug_assert_msg(false, "Unknown feature value type");
      return false;
    };

    auto isValid = [&]
    {
      if (schema.FeatureType == FeatureSchema::Type::Discrete)
      {
        if (feature.Syntax == Syntax::Range)
        {
          return false;
        }
        if (feature.RightComparison && feature.RightComparison->Op != ComparisonOperator::Equal)
        {
          return false;
        }
      }
      if (schema.FeatureValueType == FeatureSchema::ValueType::CustomProperty)
      {
        if (!IsCustomPropertyName(feature.Name.View()))
        {
          return false;
        }
      }
      if (feature.LeftComparison)
      {
        if (!validateValue(feature.LeftComparison->Value))
        {
          return false;
        }
      }
      if (feature.RightComparison)
      {
        if (!validateValue(feature.RightComparison->Value))
        {
          return false;
        }
      }
      return true;
    }();

    feature.Schema = isValid ? &schema : nullptr;
    return isValid;
  }
}
