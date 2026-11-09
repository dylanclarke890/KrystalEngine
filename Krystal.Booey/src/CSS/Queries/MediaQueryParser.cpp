/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQuerySerialisation.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryFeatures.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css::mq
{
  List<const FeatureSchema *> MediaQueryParser::FeatureSchemas() noexcept
  {
    return Features::AllSchemas();
  }

  MediaQueryList MediaQueryParser::Parse(CSSOMStringView string, const ParserContext &context) noexcept
  {
    InputStream inputStream {CSSOMString(string)};
    Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    return Parse(tokenizer.Tokens(), context);
  }

  MediaQueryList MediaQueryParser::Parse(TokenRange tokens, const ParserContext &context) noexcept
  {
    return ConsumeMediaQueryList(tokens, context);
  }

  Maybe<MediaQuery> MediaQueryParser::ParseCondition(TokenRange tokens, const ParserContext &context) noexcept
  {
    tokens.DiscardWhitespace();

    if (tokens.IsAtEnd())
    {
      return MediaQuery {{}, AllAtom()};
    }

    auto condition = ConsumeCondition(tokens, context);
    if (!condition)
    {
      return {};
    }

    return MediaQuery {{}, CSSOMStringAtom::Null(), krys::move(condition)};
  }

  MediaQueryList MediaQueryParser::ConsumeMediaQueryList(TokenRange &tokens,
                                                         const ParserContext &context) noexcept
  {
    tokens.DiscardWhitespace();

    if (tokens.IsAtEnd())
    {
      return {};
    }

    MediaQueryList list;

    while (true)
    {
      auto begin = tokens;
      while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma)
      {
        tokens.DiscardComponentValue();
      }

      auto subrange = begin.RangeUntil(tokens);

      auto consumeMediaQueryOrNotAll = [&]
      {
        if (auto query = ConsumeMediaQuery(subrange, context))
        {
          return krys::move(*query);
        }
        // "A media query that does not match the grammar in the previous section must be replaced by not all
        // during parsing."
        return MediaQuery {Prefix::Not, AllAtom()};
      };

      list.push_back(consumeMediaQueryOrNotAll());

      if (tokens.IsAtEnd())
      {
        break;
      }
      tokens.Discard();
      tokens.DiscardWhitespace();
    }

    return list;
  }

  Maybe<MediaQuery> MediaQueryParser::ConsumeMediaQuery(TokenRange &tokens,
                                                        const ParserContext &context) noexcept
  {
    // <media-condition>

    auto rangeCopy = tokens;
    if (auto condition = ConsumeCondition(tokens, context))
    {
      if (!tokens.IsAtEnd())
      {
        return {};
      }
      return MediaQuery {{}, CSSOMStringAtom::Null(), krys::move(condition)};
    }

    tokens = rangeCopy;

    // [ not | only ]? <media-type> [ and <media-condition-without-or> ]

    auto consumePrefix = [&]() -> Maybe<Prefix>
    {
      if (tokens.Peek().Type() != TokenType::Ident)
      {
        return {};
      }

      if (tokens.Peek().ValueId() == ValueId::Not)
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return Prefix::Not;
      }
      if (tokens.Peek().ValueId() == ValueId::Only)
      {
        // 'only' doesn't do anything. It exists to hide the rule from legacy agents.
        tokens.Discard();
        tokens.DiscardWhitespace();
        return Prefix::Only;
      }
      return {};
    };

    auto consumeMediaType = [&]() -> CSSOMStringAtom
    {
      if (tokens.Peek().Type() != TokenType::Ident)
      {
        return CSSOMStringAtom::Null();
      }

      auto identifier = tokens.Peek().ValueId();
      if (identifier == ValueId::Only || identifier == ValueId::Not || identifier == ValueId::And
          || identifier == ValueId::Or)
      {
        return CSSOMStringAtom::Null();
      }

      auto mediaType = text::ToASCIILower(tokens.ConsumeIncludingWhitespace().IdentCodePoints());
      if (mediaType == u8"layer")
      {
        return CSSOMStringAtom::Null();
      }

      return CSSOMStringAtom(krys::move(mediaType));
    };

    auto prefix = consumePrefix();
    auto mediaType = consumeMediaType();

    if (mediaType == CSSOMStringAtom::Null())
    {
      return {};
    }

    if (tokens.IsAtEnd())
    {
      return MediaQuery {prefix, mediaType, {}};
    }

    if (tokens.Peek().Type() != TokenType::Ident || tokens.Peek().ValueId() != ValueId::And)
    {
      return {};
    }

    tokens.Discard();
    tokens.DiscardWhitespace();

    auto condition = ConsumeCondition(tokens, context);
    if (!condition)
    {
      return {};
    }

    if (!tokens.IsAtEnd())
    {
      return {};
    }

    if (condition->LogicalOperator == LogicalOperator::Or)
    {
      return {};
    }

    return MediaQuery {prefix, mediaType, krys::move(condition)};
  }

  void Serialise(StringBuilder &builder, const MediaQueryList &list) noexcept
  {
    for (size_t i = 0; i < list.size(); ++i)
    {
      if (i)
      {
        builder.Append(u8", ");
      }
      Serialise(builder, list[i]);
    }
  }

  void Serialise(StringBuilder &builder, const MediaQuery &query) noexcept
  {
    if (query.Prefix)
    {
      switch (*query.Prefix)
      {
        case Prefix::Not:  builder.Append(u8"not "); break;
        case Prefix::Only: builder.Append(u8"only "); break;
      }
    }

    bool hasMediaType =
      query.MediaType != CSSOMStringAtom::Null() && query.MediaType != CSSOMStringAtom::Empty();
    if (hasMediaType && (!query.Condition || query.Prefix || query.MediaType != AllAtom()))
    {
      SerialiseIdentifier(CSSOMString(query.MediaType.View()), builder);
      if (query.Condition)
      {
        builder.Append(u8" and ");
      }
    }

    if (query.Condition)
    {
      Serialise(builder, *query.Condition);
    }
  }
}
