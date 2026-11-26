/*
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

#include "Krystal.Web/Css/Properties/Consumers/Url.hpp"
#include "Krystal.Web/Css/Parser/TokenRange.hpp"
#include "Krystal.Web/Css/Parser/TokenRangeGuard.hpp"
#include "Krystal.Web/Css/Properties/Consumers/KeywordDefinitions.hpp"
#include "Krystal.Web/Css/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Web/Css/Properties/Consumers/Primitives.hpp"
#include "Krystal.Web/Css/Properties/Consumers/String.hpp"
#include "Krystal.Web/Css/Properties/PropertyParserState.hpp"
#include "Krystal.Web/Css/Values/Primitives/Url.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Css/Values/UrlValue.hpp"

namespace krys::web::css::parse_property
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

  Maybe<Url> ConsumeUrlRaw(TokenRange &tokens, PropertyParserState &state,
                           AllowedUrlModifiers allowedUrlModifiers) noexcept
  {
    auto &token = tokens.Peek();
    if (token.Type() == TokenType::Url)
    {
      auto result = CompleteUrl(CSSOMString(token.IdentCodePoints()), state.Context);
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

        // An empty string is a valid URL, so check for the token rather than an empty result.
        if (args.Peek().Type() != TokenType::String)
        {
          return {};
        }

        auto string = ConsumeStringRaw(args);

        auto result = CompleteUrl(CSSOMString(string), state.Context);
        if (!result)
        {
          return {};
        }

        while (!args.IsAtEnd())
        {
          switch (args.Peek().FunctionId())
          {
            case ValueId::CrossOrigin:
            {
              if (!allowedUrlModifiers.contains(AllowedUrlModifier::CrossOrigin))
              {
                return {};
              }

              if (result->Modifiers.CrossOrigin)
              {
                return {};
              }

              auto crossOriginArgs = ConsumeFunction(args);
              auto crossOriginValue =
                MetaConsumer<keywords::Anonymous, keywords::UseCredentials>::Consume(crossOriginArgs, state);

              if (!crossOriginValue || !crossOriginArgs.IsAtEnd())
              {
                return {};
              }

              result->Modifiers.CrossOrigin = UrlCrossOriginFunction {.Parameters = {*crossOriginValue}};
              break;
            }
            case ValueId::Integrity:
            {
              if (!allowedUrlModifiers.contains(AllowedUrlModifier::Integrity))
              {
                return {};
              }

              if (result->Modifiers.Integrity)
              {
                return {};
              }

              auto integrityArgs = ConsumeFunction(args);
              if (integrityArgs.Peek().Type() != TokenType::String)
              {
                return {};
              }

              auto integrityValue = ConsumeStringRaw(integrityArgs);

              if (!integrityArgs.IsAtEnd())
              {
                return {};
              }

              result->Modifiers.Integrity =
                UrlIntegrityFunction {.Parameters = {CSSOMString(integrityValue)}};

              break;
            }
            case ValueId::ReferrerPolicy:
            {
              if (!allowedUrlModifiers.contains(AllowedUrlModifier::ReferrerPolicy))
              {
                return {};
              }

              if (result->Modifiers.ReferrerPolicy)
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

              result->Modifiers.ReferrerPolicy =
                UrlReferrerPolicyFunction {.Parameters = {*referrerPolicyValue}};

              break;
            }
            default:
            {
              return {};
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

  RefPtr<Value> ConsumeUrl(TokenRange &tokens, PropertyParserState &state,
                           AllowedUrlModifiers allowedUrlModifiers) noexcept
  {
    if (auto rawUrl = ConsumeUrlRaw(tokens, state, allowedUrlModifiers))
    {
      return UrlValue::Create(krys::move(*rawUrl));
    }

    return nullptr;
  }
}
