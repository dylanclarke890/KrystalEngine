#include "Krystal.HTML/CSS/Properties/Consumers/String.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  CSSOMStringView ConsumeStringRaw(CSSTokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != CSSTokenType::String)
    {
      return {};
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return token.IdentCodePoints();
  }

  RefPtr<CSSPrimitiveValue> ConsumeString(CSSTokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != CSSTokenType::String)
    {
      return nullptr;
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return CSSPrimitiveValue::Create(CSSOMString(token.IdentCodePoints()));
  }
}