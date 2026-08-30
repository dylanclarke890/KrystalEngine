#include "Krystal.HTML/CSS/Properties/Consumers/Animations.hpp"
#include "Krystal.HTML/CSS/Parser/CSSParserIdioms.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Timeline.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{

  SmallList<Pair<CSSValueId, double>> ConsumeKeyframeKeyList(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept
  {
    // <keyframe-selector> = from | to | <percentage [0,100]> | <timeline-tokens-name> <percentage>
    // https://drafts.csswg.org/css-animations-1/#typedef-keyframe-selector

    enum class RestrictedToZeroToHundredRange : bool
    {
      No,
      Yes
    };

    auto ConsumeAndConvertPercentage = [&](CSSTokenRange &tokens,
                                           RestrictedToZeroToHundredRange restricted) -> Maybe<double>
    {
      // FIXME: We use resolveAsPercentageDeprecated() to deal with calc() and % values.
      // We will eventually want to return a CSS value that can be kept as-is on a
      // BlendingKeyframe so that resolution happens when we have the necessary context
      // when the keyframes are associated with a target element.
      if (auto percentageValue = CSSPrimitiveValueResolver<Percentage<>>::ConsumeAndResolve(tokens, state))
      {
        // TODO: I'm using ResolveAsPercentageNoConversionDataRequired() here instead of
        // ResolveAsPercentageDeprecated()
        auto resolvedPercentage = percentageValue->ResolveAsPercentageNoConversionDataRequired();
        if (restricted == RestrictedToZeroToHundredRange::No)
        {
          return resolvedPercentage / 100;
        }

        if (resolvedPercentage >= 0 && resolvedPercentage <= 100)
        {
          return resolvedPercentage / 100;
        }
      }
      return {};
    };

    auto TimelineRange = [&](CSSTokenRange &tokens, CSSValueId id) -> Maybe<Pair<CSSValueId, double>>
    {
      if (IsAnimationRangeKeyword(id))
      {
        // "normal" will be considered valid by isAnimationRangeKeyword() but is not valid for a @keyframes
        // rule.
        if (id == CSSValueId::Normal)
        {
          return {};
        }

        if (auto convertedPercentage =
              ConsumeAndConvertPercentage(tokens, RestrictedToZeroToHundredRange::No))
        {
          return {{id, *convertedPercentage}};
        }
      }

      return {};
    };

    SmallList<Pair<CSSValueId, double>> result;
    while (true)
    {
      tokens.DiscardWhitespace();

      if (auto tokenValue = ConsumeIdent(tokens))
      {
        auto valueId = tokenValue->ValueId();
        if (valueId == CSSValueId::From)
        {
          result.push_back({CSSValueId::Normal, 0});
        }
        else if (valueId == CSSValueId::To)
        {
          result.push_back({CSSValueId::Normal, 1});
        }
        else if (auto pair = TimelineRange(tokens, valueId))
        {
          result.push_back(*pair);
        }
        else
        {
          return {}; // Parser error, invalid value in keyframe selector
        }
      }
      else if (auto convertedPercentage =
                 ConsumeAndConvertPercentage(tokens, RestrictedToZeroToHundredRange::Yes))
      {
        result.push_back({CSSValueId::Normal, *convertedPercentage});
      }
      else
      {
        return {}; // Parser error, invalid value in keyframe selector
      }

      if (tokens.IsAtEnd())
      {
        return result;
      }

      if (tokens.Consume().Type() != CSSTokenType::Comma)
      {
        return {}; // Parser error
      }
    }
  }

  SmallList<Pair<CSSValueId, double>> ParseKeyframeKeyList(const CSSOMString &string,
                                                           const CSSParserContext &context) noexcept
  {
    auto inputStream = CSSInputStream(CSSOMString(string));
    auto tokenizer = CSSTokenizer(inputStream);
    auto tokens = tokenizer.TokenRange();

    // Handle leading whitespace.
    tokens.DiscardWhitespace();

    auto state = CSSPropertyParserState {.Context = context};
    auto result = ConsumeKeyframeKeyList(tokens, state);

    // Handle trailing whitespace.
    tokens.DiscardWhitespace();

    if (!tokens.IsAtEnd())
    {
      return {};
    }

    return result;
  }

  RefPtr<CSSValue> ConsumeKeyframesName(CSSTokenRange &tokens, CSSPropertyParserState &) noexcept
  {
    // <keyframes-name> = <custom-ident> | <string>
    // https://drafts.csswg.org/css-animations/#typedef-keyframes-name

    if (tokens.Peek().Type() == CSSTokenType::String)
    {
      auto &token = tokens.Consume();
      tokens.DiscardWhitespace();

      auto valueId = FindCSSValueKeyword(token.IdentCodePoints());
      if (CSSParserIdioms::IsValidCustomIdentifier(valueId) && valueId != CSSValueId::None)
      {
        return CSSPrimitiveValue::CreateCustomIdent(token.IdentCodePoints());
      }

      return CSSPrimitiveValue::Create(token.IdentCodePoints());
    }

    return ConsumeCustomIdent(tokens);
  }
}