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