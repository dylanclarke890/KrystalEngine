#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  bool ConsumeComma(CSSTokenRange &tokens) noexcept
  {
    CSSToken value = tokens.Peek();
    if (value.Type() != CSSTokenType::Comma)
    {
      return false;
    }

    tokens.Discard();
    tokens.DiscardWhitespace();

    return true;
  }

  bool ConsumeSlash(CSSTokenRange &tokens) noexcept
  {
    CSSToken value = tokens.Peek();
    if (value.Type() != CSSTokenType::Delim || value.IdentCodePoints() != u8"/")
    {
      return false;
    }

    tokens.Discard();
    tokens.DiscardWhitespace();

    return true;
  }

  CSSTokenRange ConsumeFunction(CSSTokenRange &tokens) noexcept
  {
    assert(tokens.Peek().Type() == CSSTokenType::Function);

    CSSTokenRange contents = tokens.ConsumeBlock();
    tokens.DiscardWhitespace();
    contents.DiscardWhitespace();

    return contents;
  }

  Maybe<CSSTokenRange> ConsumeArgument(CSSTokenRange &tokens, size_t index) noexcept
  {
    if (tokens.IsAtEnd())
    {
      return Null;
    }

    if (index)
    {
      assert(tokens.Peek().Type() == CSSTokenType::Comma);
      tokens.Discard();
    }

    tokens.DiscardWhitespace();

    auto argumentStart = tokens;
    while (!tokens.IsAtEnd())
    {
      if (tokens.Peek().Type() == CSSTokenType::Comma)
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