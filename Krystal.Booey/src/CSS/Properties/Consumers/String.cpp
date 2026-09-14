#include "Krystal.Booey/CSS/Properties/Consumers/String.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
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

  RefPtr<PrimitiveValue> ConsumeString(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::String)
    {
      return nullptr;
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    return PrimitiveValue::Create(CSSOMString(token.IdentCodePoints()));
  }
}