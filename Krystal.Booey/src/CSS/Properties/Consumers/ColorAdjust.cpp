#include "Krystal.Booey/CSS/Properties/Consumers/ColorAdjust.hpp"
#include "Krystal.Booey/CSS/Parser/Context/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/CSS/Values/ColorAdjust/CSSColorSchemeValue.hpp"

namespace krys::boo::css
{
  Maybe<ColorScheme> ConsumeUnresolvedColorScheme(TokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <'color-scheme'> = normal | [ light | dark | <custom-ident> ]+ && only?
    // https://drafts.csswg.org/css-color-adjust/#propdef-color-scheme

    if (tokens.Peek().ValueId() == CSSValueId::Normal)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      // NOTE: `normal` is represented in CSS::ColorScheme as an empty list of schemes.
      return ColorScheme {.schemes = {}, .only = {}};
    }

    Maybe<ColorScheme> result = ColorScheme {.schemes = {}, .only = {}};

    if (tokens.Peek().ValueId() == CSSValueId::Only)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      result->only = Keywords::Only {};
    }

    while (!tokens.IsAtEnd())
    {
      if (tokens.Peek().Type() != TokenType::Ident)
      {
        return {};
      }

      CSSValueId id = tokens.Peek().ValueId();

      switch (id)
      {
        case CSSValueId::Normal:
        {
          // `normal` is only allowed as a single value, and was handled earlier.
          // Don't allow it in the list.
          return {};
        }
        case CSSValueId::Only:
        {
          // `only` can either appear first, handled before the loop, or last,
          // handled here.
          if (result->only)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          result->only = Keywords::Only {};

          if (!tokens.IsAtEnd())
          {
            return {};
          }

          break;
        }
        default:
        {
          if (!IsValidCustomIdentifier(id))
          {
            return {};
          }

          auto value = tokens.Consume().IdentCodePoints();
          tokens.DiscardWhitespace();

          result->schemes.value.push_back(CustomIdentifier {value});
          break;
        }
      }
    }

    if (result->schemes.empty())
    {
      return {};
    }

    return result;
  }

  Maybe<ColorScheme> ParseUnresolvedColorScheme(const CSSOMString &scheme,
                                                const ParserContext &context) noexcept
  {
    InputStream inputStream {CSSOMString(scheme)};
    auto tokenizer = Tokenizer(inputStream);
    auto tokens = tokenizer.Tokens();

    // Handle leading whitespace.
    tokens.DiscardWhitespace();

    auto state = CSSPropertyParserState {.Context = context};
    auto result = ConsumeUnresolvedColorScheme(tokens, state);

    // Handle trailing whitespace.
    tokens.DiscardWhitespace();

    if (!tokens.IsAtEnd())
    {
      return {};
    }

    return result;
  }

  RefPtr<CSSValue> ConsumeColorScheme(TokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    auto colorScheme = ConsumeUnresolvedColorScheme(tokens, state);
    if (!colorScheme)
    {
      return {};
    }

    return CSSColorSchemeValue::Create(krys::move(*colorScheme));
  }
}