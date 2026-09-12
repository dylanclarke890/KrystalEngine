#include "Krystal.Booey/CSS/Properties/Consumers/String.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
{
  CSSOMStringView ConsumeStringRaw(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::String)
    {
      return {};
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return token.IdentCodePoints();
  }

  RefPtr<CSSPrimitiveValue> ConsumeString(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::String)
    {
      return nullptr;
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return CSSPrimitiveValue::Create(CSSOMString(token.IdentCodePoints()));
  }
}