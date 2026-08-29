#include "Krystal.HTML/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRangeGuard.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/KeywordDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/String.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSURLValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSURL.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  // MARK: <url>
  // https://drafts.csswg.org/css-values/#urls

  // <url> = <url()> | <src()>
  //
  // <url()> = url( <string> <url-modifier>* ) | <url-token>
  // <src()> = src( <string> <url-modifier>* )

  // <url-modifier> = <cross-origin-modifier> | <integrity-modifier> | <referrer-policy-modifier>
  //
  // <cross-origin-modifier> = cross-origin( anonymous | use-credentials )
  // <integrity-modifier> = integrity( <string> )
  // <referrer-policy-modifier> = referrer-policy( no-referrer | no-referrer-when-downgrade | same-origin |
  // origin | strict-origin | origin-when-cross-origin | strict-origin-when-cross-origin | unsafe-url)

  Maybe<CSSURL> ConsumeURLRaw(CSSTokenRange &tokens, CSSPropertyParserState &state,
                              AllowedURLModifiers allowedURLModifiers) noexcept
  {
    auto &token = tokens.Peek();
    if (token.Type() == CSSTokenType::Url)
    {
      auto result = CompleteURL(CSSOMString(token.IdentCodePoints()), state.Context);
      if (!result)
      {
        return {};
      }

      tokens.Discard();
      tokens.DiscardWhitespace();

      return result;
    }

    switch (token.FunctionId())
    {
      case CSSValueId::Url:
      {
        CSSTokenRangeGuard guard {tokens};

        auto args = ConsumeFunction(tokens);

        auto string = ConsumeStringRaw(args);
        if (string.empty())
        {
          return {};
        }

        auto result = CompleteURL(CSSOMString(string), state.Context);
        if (!result)
        {
          return {};
        }

        if (!state.Context.cssURLModifiersEnabled)
        {
          if (!args.IsAtEnd())
          {
            return {};
          }
        }
        else
        {
          while (!args.IsAtEnd())
          {
            switch (args.Peek().FunctionId())
            {
              case CSSValueId::CrossOrigin:
              {
                if (!HasFlag(allowedURLModifiers, AllowedURLModifiers::CrossOrigin))
                {
                  return {};
                }

                if (result->modifiers.crossOrigin)
                {
                  return {};
                }

                auto crossOriginArgs = ConsumeFunction(args);
                auto crossOriginValue = MetaConsumer<Keywords::Anonymous, Keywords::UseCredentials>::Consume(
                  crossOriginArgs, state);

                if (!crossOriginValue || !crossOriginArgs.IsAtEnd())
                {
                  return {};
                }

                result->modifiers.crossOrigin = URLCrossOriginFunction {.parameters = {*crossOriginValue}};
                break;
              }
              case CSSValueId::Integrity:
              {
                if (!state.Context.cssURLIntegrityModifierEnabled)
                {
                  return {};
                }

                if (!HasFlag(allowedURLModifiers, AllowedURLModifiers::Integrity))
                {
                  return {};
                }

                if (result->modifiers.integrity)
                {
                  return {};
                }

                auto integrityArgs = ConsumeFunction(args);
                auto integrityValue = ConsumeStringRaw(integrityArgs);

                if (integrityValue.empty() || !integrityArgs.IsAtEnd())
                {
                  return {};
                }

                result->modifiers.integrity =
                  URLIntegrityFunction {.parameters = {CSSOMString(integrityValue)}};

                break;
              }
              case CSSValueId::ReferrerPolicy:
              {
                if (!HasFlag(allowedURLModifiers, AllowedURLModifiers::ReferrerPolicy))
                {
                  return {};
                }

                if (result->modifiers.referrerPolicy)
                {
                  return {};
                }

                auto referrerPolicyArgs = ConsumeFunction(args);
                auto referrerPolicyValue =
                  MetaConsumer<Keywords::NoReferrer, Keywords::NoReferrerWhenDowngrade, Keywords::SameOrigin,
                               Keywords::Origin, Keywords::StrictOrigin, Keywords::OriginWhenCrossOrigin,
                               Keywords::StrictOriginWhenCrossOrigin,
                               Keywords::UnsafeUrl>::Consume(referrerPolicyArgs, state);

                if (!referrerPolicyValue || !referrerPolicyArgs.IsAtEnd())
                {
                  return {};
                }

                result->modifiers.referrerPolicy =
                  URLReferrerPolicyFunction {.parameters = {*referrerPolicyValue}};

                break;
              }
              default:
              {
                return {};
              }
            }
          }
        }

        guard.Commit();

        return result;
      }

      default: break;
    }

    return {};
  }

  RefPtr<CSSValue> ConsumeURL(CSSTokenRange &tokens, CSSPropertyParserState &state,
                              AllowedURLModifiers allowedURLModifiers) noexcept
  {
    if (auto rawURL = ConsumeURLRaw(tokens, state, allowedURLModifiers))
    {
      return CSSURLValue::Create(Krys::Move(*rawURL));
    }

    return nullptr;
  }
}