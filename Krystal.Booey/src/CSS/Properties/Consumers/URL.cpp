#include "Krystal.Booey/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRangeGuard.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/KeywordDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/String.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/UrlValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"

namespace krys::boo::css::PropertyParserHelpers
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

  Maybe<CSSURL> ConsumeURLRaw(TokenRange &tokens, PropertyParserState &state,
                              AllowedURLModifiers allowedURLModifiers) noexcept
  {
    auto &token = tokens.Peek();
    if (token.Type() == TokenType::Url)
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
      case ValueId::Url:
      {
        TokenRangeGuard guard {tokens};

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
              case ValueId::CrossOrigin:
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
                auto crossOriginValue = MetaConsumer<keywords::Anonymous, keywords::UseCredentials>::Consume(
                  crossOriginArgs, state);

                if (!crossOriginValue || !crossOriginArgs.IsAtEnd())
                {
                  return {};
                }

                result->modifiers.crossOrigin = URLCrossOriginFunction {.parameters = {*crossOriginValue}};
                break;
              }
              case ValueId::Integrity:
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
              case ValueId::ReferrerPolicy:
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
                  MetaConsumer<keywords::NoReferrer, keywords::NoReferrerWhenDowngrade, keywords::SameOrigin,
                               keywords::Origin, keywords::StrictOrigin, keywords::OriginWhenCrossOrigin,
                               keywords::StrictOriginWhenCrossOrigin,
                               keywords::UnsafeUrl>::Consume(referrerPolicyArgs, state);

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

  RefPtr<Value> ConsumeURL(TokenRange &tokens, PropertyParserState &state,
                              AllowedURLModifiers allowedURLModifiers) noexcept
  {
    if (auto rawURL = ConsumeURLRaw(tokens, state, allowedURLModifiers))
    {
      return UrlValue::Create(krys::move(*rawURL));
    }

    return nullptr;
  }
}