#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Symbol.hpp"

namespace krys::boo::css
{
  KRYS_NODISCARD Maybe<SymbolRaw> ValidatedRange(SymbolRaw value, PropertyParserOptions options) noexcept;

  struct SymbolKnownTokenTypeIdentConsumer
  {
    constexpr static TokenType TokenType = TokenType::Ident;

    KRYS_NODISCARD static Maybe<SymbolRaw> Consume(TokenRange &tokens, PropertyParserState &state,
                                                   AllowedSymbols symbolsAllowed,
                                                   PropertyParserOptions options) noexcept;
  };

  template <>
  struct ConsumerDefinition<Symbol>
  {
    using IdentToken = SymbolKnownTokenTypeIdentConsumer;
  };
}
