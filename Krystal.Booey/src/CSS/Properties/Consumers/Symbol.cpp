#include "Krystal.Booey/CSS/Properties/Consumers/SymbolDefinitions.hpp"

namespace krys::boo::css
{
  Maybe<SymbolRaw> ValidatedRange(SymbolRaw value, PropertyParserOptions) noexcept
  {
    return value;
  }

  Maybe<SymbolRaw> SymbolKnownTokenTypeIdentConsumer::Consume(TokenRange &tokens, PropertyParserState &,
                                                              AllowedSymbols symbolsAllowed,
                                                              PropertyParserOptions) noexcept
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::Ident);

    auto symbol = tokens.Peek().ValueId();
    if (symbolsAllowed.Contains(symbol))
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
      return SymbolRaw {symbol};
    }

    return null;
  }
}
