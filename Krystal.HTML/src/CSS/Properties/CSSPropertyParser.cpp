#include "Krystal.HTML/CSS/Properties/CSSPropertyParser.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyShorthand.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"

namespace Krys::HTML
{
  namespace
  {
    KRYS_NODISCARD RefPtr<CSSPrimitiveValue> ConsumeCSSWideKeywordValue(CSSTokenRange &tokens) noexcept
    {
      auto tokensCopy = tokens;
      auto &identifier = tokensCopy.Consume();

      if (!tokensCopy.IsAtEnd())
      {
        return nullptr;
      }

      auto valueId = ParseCSSValueId(identifier.IdentCodePoints());
      if (!CSSValue::IsCSSWideKeyword(valueId))
      {
        return nullptr;
      }

      tokens = tokensCopy;
      return CreateRef<CSSPrimitiveValue>(valueId);
    }
  }

  bool CSSPropertyParser::ParseValue(CSSTokenRange tokens, const CSSParserContext &context, CSSPropertyId id,
                                     CSSRuleType ruleType, IsImportant important,
                                     ParsedCSSPropertyList &properties) noexcept
  {
    tokens.DiscardWhitespace();

    bool success = false;
    switch (ruleType)
    {
      case CSSRuleType::Style:
      default:
      {
        success = ConsumeStyleProperty(tokens, context, id, important, ruleType, properties);
        break;
      }
    }

    return success;
  }

  bool CSSPropertyParser::ConsumeStyleProperty(CSSTokenRange &tokens, const CSSParserContext &context,
                                               CSSPropertyId id, IsImportant important, CSSRuleType ruleType,
                                               ParsedCSSPropertyList &properties) noexcept
  {
    auto state = CSSPropertyParserState {
      .Context = context,
      .Property = id,
      .CurrentRule = ruleType,
      .Important = important,
    };

    auto tokensCopy = tokens;
    if (CSSProperty::IsShorthand(id))
    {
      if (auto cssWideKeywordValue = ConsumeCSSWideKeywordValue(tokensCopy))
      {
        for (auto longhand : LonghandsForShorthand(id).LonghandProperties)
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
      properties.emplace_back(id, Krys::Move(cssWideKeywordValue), important);
      tokens = tokensCopy;
      return true;
    }

    return CSSPropertyConsumer::ParseLonghandStyleProperty(tokens, state, properties);
  }
}