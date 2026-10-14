#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQuerySerialisation.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryFeatures.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::css::mq
{
  KRYS_NODISCARD static const CSSOMStringAtom &AllAtom() noexcept
  {
    static const CSSOMStringAtom all {u8"all"};
    return all;
  }

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

  void Serialise(CSSOMString &builder, const MediaQueryList &list) noexcept
  {
    for (size_t i = 0; i < list.size(); ++i)
    {
      if (i)
      {
        builder += u8", ";
      }
      Serialise(builder, list[i]);
    }
  }

  void Serialise(CSSOMString &builder, const MediaQuery &query) noexcept
  {
    if (query.Prefix)
    {
      switch (*query.Prefix)
      {
        case Prefix::Not:  builder += u8"not "; break;
        case Prefix::Only: builder += u8"only "; break;
      }
    }

    bool hasMediaType =
      query.MediaType != CSSOMStringAtom::Null() && query.MediaType != CSSOMStringAtom::Empty();
    if (hasMediaType && (!query.Condition || query.Prefix || query.MediaType != AllAtom()))
    {
      SerialiseIdentifier(CSSOMString(query.MediaType.View()), builder);
      if (query.Condition)
      {
        builder += u8" and ";
      }
    }

    if (query.Condition)
    {
      Serialise(builder, *query.Condition);
    }
  }
}
