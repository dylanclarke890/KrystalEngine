#include "Krystal.HTML/CSS/Properties/Consumers/ColorAdjust.hpp"
#include "Krystal.HTML/CSS/Parser/CSSParserContext.hpp"
#include "Krystal.HTML/CSS/Parser/CSSParserIdioms.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.HTML/CSS/Values/ColorAdjust/CSSColorSchemeValue.hpp"

namespace Krys::HTML
{
  Maybe<ColorScheme> ConsumeUnresolvedColorScheme(CSSTokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept
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
      if (tokens.Peek().Type() != CSSTokenType::Ident)
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
          if (!CSSParserIdioms::IsValidCustomIdentifier(id))
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
                                                const CSSParserContext &context) noexcept
  {
    CSSInputStream inputStream {CSSOMString(scheme)};
    auto tokenizer = CSSTokenizer(inputStream);
    auto range = tokenizer.TokenRange();

    // Handle leading whitespace.
    range.DiscardWhitespace();

    auto state = CSSPropertyParserState {.Context = context};
    auto result = ConsumeUnresolvedColorScheme(range, state);

    // Handle trailing whitespace.
    range.DiscardWhitespace();

    if (!range.IsAtEnd())
    {
      return {};
    }

    return result;
  }

  RefPtr<CSSValue> ConsumeColorScheme(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    auto colorScheme = ConsumeUnresolvedColorScheme(tokens, state);
    if (!colorScheme)
    {
      return {};
    }

    return CSSColorSchemeValue::Create(Krys::Move(*colorScheme));
  }
}