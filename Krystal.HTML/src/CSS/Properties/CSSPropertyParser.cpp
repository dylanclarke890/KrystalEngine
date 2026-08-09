#include "Krystal.HTML/CSS/Properties/CSSPropertyParser.hpp"

namespace Krys::HTML
{
  namespace
  {
    KRYS_NODISCARD bool ParseShorthandStyleProperty(CSSTokenRange tokens, CSSPropertyParserState& state, ParsedCSSPropertyList &properties) noexcept
    {
      return false;
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
        success = ConsumeStyleProperty(tokens, context, id, important, ruleType);
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

    if (CSSProperty::IsShorthand(id))
    {
      return ParseShorthandStyleProperty(tokens, state, properties);
    }
    else
    {
    }
  }
}