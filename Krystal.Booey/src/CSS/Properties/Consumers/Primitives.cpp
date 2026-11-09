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

#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  bool ConsumeComma(TokenRange &tokens) noexcept
  {
    Token value = tokens.Peek();
    if (value.Type() != TokenType::Comma)
    {
      return false;
    }

    tokens.Discard();
    tokens.DiscardWhitespace();

    return true;
  }

  bool ConsumeSlash(TokenRange &tokens) noexcept
  {
    Token value = tokens.Peek();
    if (value.Type() != TokenType::Delim || value.Delimiter() != U'/')
    {
      return false;
    }

    tokens.Discard();
    tokens.DiscardWhitespace();

    return true;
  }

  TokenRange ConsumeFunction(TokenRange &tokens) noexcept
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::Function);

    TokenRange contents = tokens.ConsumeBlock();
    tokens.DiscardWhitespace();
    contents.DiscardWhitespace();

    return contents;
  }

  Maybe<TokenRange> ConsumeArgument(TokenRange &tokens, size_t index) noexcept
  {
    if (tokens.IsAtEnd())
    {
      return null;
    }

    if (index)
    {
      krys_debug_assert(tokens.Peek().Type() == TokenType::Comma);
      tokens.Discard();
    }

    tokens.DiscardWhitespace();

    auto argumentStart = tokens;
    while (!tokens.IsAtEnd())
    {
      if (tokens.Peek().Type() == TokenType::Comma)
      {
        break;
      }

      if (tokens.Peek().IsBlockStart())
      {
        (void)tokens.ConsumeBlock();
        continue;
      }

      tokens.Discard();
    }

    return argumentStart.RangeUntil(tokens);
  }
}