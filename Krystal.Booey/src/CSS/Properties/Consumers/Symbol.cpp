/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
