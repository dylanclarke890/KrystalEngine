#include "Krystal.Booey/CSS/Properties/CSSPropertyParser.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeCSSWideKeywordValue(TokenRange &tokens) noexcept
    {
      auto tokensCopy = tokens;
      auto &identifier = tokensCopy.Consume();

      if (!tokensCopy.IsAtEnd())
      {
        return nullptr;
      }

      auto valueId = FindCSSValueKeyword(identifier.IdentCodePoints());
      if (!CSSValue::IsCSSWideKeyword(valueId))
      {
        return nullptr;
      }

      tokens = tokensCopy;
      return CSSPrimitiveValue::Create(valueId);
    }
  }

  bool CSSPropertyParser::ParseValue(TokenRange tokens, const ParserContext &context, CSSPropertyId id,
                                     RuleType ruleType, IsImportant important,
                                     ParsedPropertyList &properties) noexcept
  {
    tokens.DiscardWhitespace();

    bool success = false;
    switch (ruleType)
    {
      case RuleType::Style:
      default:
      {
        success = ConsumeStyleProperty(tokens, context, id, important, ruleType, properties);
        break;
      }
    }

    return success;
  }

  bool CSSPropertyParser::ConsumeStyleProperty(TokenRange &tokens, const ParserContext &context,
                                               CSSPropertyId id, IsImportant important, RuleType ruleType,
                                               ParsedPropertyList &properties) noexcept
  {
    auto state = CSSPropertyParserState {
      .Context = context,
      .CurrentProperty = id,
      .CurrentRule = ruleType,
      .Important = important,
    };

    auto tokensCopy = tokens;
    if (Property::IsShorthand(id))
    {
      if (auto cssWideKeywordValue = ConsumeCSSWideKeywordValue(tokensCopy))
      {
        for (auto longhand : LonghandsForShorthand(id).Properties())
        {
          properties.emplace_back(longhand, cssWideKeywordValue, important);
          tokens = tokensCopy;
        }
        return true;
      }

      return CSSPropertyConsumer::ParseShorthandStyleProperty(tokens, state, properties);
    }

    if (auto cssWideKeywordValue = ConsumeCSSWideKeywordValue(tokensCopy))
    {
      properties.emplace_back(id, krys::move(cssWideKeywordValue), important);
      tokens = tokensCopy;
      return true;
    }

    return CSSPropertyConsumer::ParseLonghandStyleProperty(tokens, state, properties);
  }
}