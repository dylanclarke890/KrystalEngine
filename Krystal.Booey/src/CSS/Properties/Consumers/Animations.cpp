/*
 * Copyright (C) 2016-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Properties/Consumers/Animations.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Timeline.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  SmallList<Pair<ValueId, double>> ConsumeKeyframeKeyList(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept
  {
    // <keyframe-selector> = from | to | <percentage [0,100]> | <timeline-range-name> <percentage>
    // https://drafts.csswg.org/css-animations-1/#typedef-keyframe-selector

    enum class RestrictedToZeroToHundredRange : bool
    {
      No,
      Yes
    };

    auto ConsumeAndConvertPercentage = [&](TokenRange &tokens,
                                           RestrictedToZeroToHundredRange restricted) -> Maybe<double>
    {
      // FIXME: We use resolveAsPercentageDeprecated() to deal with calc() and % values.
      // We will eventually want to return a CSS value that can be kept as-is on a
      // BlendingKeyframe so that resolution happens when we have the necessary context
      // when the keyframes are associated with a target element.
      if (auto percentageValue = PrimitiveValueResolver<Percentage<>>::ConsumeAndResolve(tokens, state))
      {
        auto resolvedPercentage = percentageValue->ResolveAsPercentageDeprecated();
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

    auto TimelineRange = [&](TokenRange &tokens, ValueId id) -> Maybe<Pair<ValueId, double>>
    {
      if (IsAnimationRangeKeyword(id))
      {
        // "normal" will be considered valid by isAnimationRangeKeyword() but is not valid for a @keyframes
        // rule.
        if (id == ValueId::Normal)
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

    SmallList<Pair<ValueId, double>> result;
    while (true)
    {
      tokens.DiscardWhitespace();

      if (auto tokenValue = ConsumeIdent(tokens))
      {
        auto valueId = tokenValue->ValueId();
        if (valueId == ValueId::From)
        {
          result.push_back({ValueId::Normal, 0});
        }
        else if (valueId == ValueId::To)
        {
          result.push_back({ValueId::Normal, 1});
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
        result.push_back({ValueId::Normal, *convertedPercentage});
      }
      else
      {
        return {}; // Parser error, invalid value in keyframe selector
      }

      if (tokens.IsAtEnd())
      {
        return result;
      }

      if (tokens.Consume().Type() != TokenType::Comma)
      {
        return {}; // Parser error
      }
    }
  }

  SmallList<Pair<ValueId, double>> ParseKeyframeKeyList(const CSSOMString &string,
                                                        const ParserContext &context) noexcept
  {
    auto inputStream = InputStream(CSSOMString(string));
    auto tokenizer = Tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    auto tokens = tokenizer.Tokens();

    // Handle leading whitespace.
    tokens.DiscardWhitespace();

    auto state = PropertyParserState {.Context = context};
    auto result = ConsumeKeyframeKeyList(tokens, state);

    // Handle trailing whitespace.
    tokens.DiscardWhitespace();

    if (!tokens.IsAtEnd())
    {
      return {};
    }

    return result;
  }

  RefPtr<Value> ConsumeKeyframesName(TokenRange &tokens, PropertyParserState &) noexcept
  {
    // <keyframes-name> = <custom-ident> | <string>
    // https://drafts.csswg.org/css-animations/#typedef-keyframes-name

    if (tokens.Peek().Type() == TokenType::String)
    {
      auto &token = tokens.Consume();
      tokens.DiscardWhitespace();

      auto valueId = ValueIdFromName(token.IdentCodePoints());
      if (IsValidCustomIdentifier(valueId) && valueId != ValueId::None)
      {
        return PrimitiveValue::CreateCustomIdent(token.IdentCodePoints());
      }

      return PrimitiveValue::Create(token.IdentCodePoints());
    }

    return ConsumeCustomIdent(tokens);
  }
}