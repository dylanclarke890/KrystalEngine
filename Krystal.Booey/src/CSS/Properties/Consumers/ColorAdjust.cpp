#include "Krystal.Booey/CSS/Properties/Consumers/ColorAdjust.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  Maybe<ColorScheme> ConsumeUnresolvedColorScheme(TokenRange &tokens, PropertyParserState &) noexcept
  {
    // <'color-scheme'> = normal | [ light | dark | <custom-ident> ]+ && only?
    // https://drafts.csswg.org/css-color-adjust/#propdef-color-scheme

    if (tokens.Peek().ValueId() == ValueId::Normal)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      // NOTE: `normal` is represented in ColorScheme as an empty list of schemes.
      return ColorScheme {.Schemes = {}, .Only = {}};
    }

    Maybe<ColorScheme> result = ColorScheme {.Schemes = {}, .Only = {}};

    if (tokens.Peek().ValueId() == ValueId::Only)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      result->Only = keywords::Only {};
    }

    while (!tokens.IsAtEnd())
    {
      if (tokens.Peek().Type() != TokenType::Ident)
      {
        return null;
      }

      auto id = tokens.Peek().ValueId();
      switch (id)
      {
        case ValueId::Normal:
        {
          // `normal` is only allowed as a single value, and was handled earlier.
          // Don't allow it in the list.
          return null;
        }
        case ValueId::Only:
        {
          // `only` can either appear first, handled before the loop, or last,
          // handled here.
          if (result->Only)
          {
            return null;
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          result->Only = keywords::Only {};

          if (!tokens.IsAtEnd())
          {
            return null;
          }

          break;
        }
        default:
        {
          if (!IsValidCustomIdentifier(id))
          {
            return null;
          }

          auto value = CSSOMStringAtom {tokens.Consume().IdentCodePoints()};
          tokens.DiscardWhitespace();

          result->Schemes.Value.push_back(CustomIdentifier {krys::move(value)});
          break;
        }
      }
    }

    if (result->Schemes.empty())
    {
      return null;
    }

    return result;
  }

  Maybe<ColorScheme> ParseUnresolvedColorScheme(CSSOMStringView string, const ParserContext &context) noexcept
  {
    InputStream input {CSSOMString(string)};
    Tokenizer tokenizer {input};
    tokenizer.PumpTokenizer();
    auto tokens = tokenizer.Tokens();

    // Handle leading whitespace.
    tokens.DiscardWhitespace();

    auto state = PropertyParserState {.Context = context};
    auto result = ConsumeUnresolvedColorScheme(tokens, state);

    // Handle trailing whitespace.
    tokens.DiscardWhitespace();

    if (!tokens.IsAtEnd())
    {
      return null;
    }

    return result;
  }
}
