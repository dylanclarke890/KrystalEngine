#include "Krystal.Booey/CSS/Properties/Consumers/Content.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Attr.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CounterStyles.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/String.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CounterValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumeQuotes(TokenRange &tokens, PropertyParserState &) noexcept
  {
    // <'quotes'> = auto | none | match-parent | [ <string> <string> ]+
    // https://drafts.csswg.org/css-content-3/#propdef-quotes

    // FIXME: Support `match-parent`.

    auto id = tokens.Peek().ValueId();
    if (id == ValueId::None || id == ValueId::Auto)
    {
      return ConsumeIdent(tokens);
    }

    ValueListBuilder values;
    while (!tokens.IsAtEnd())
    {
      auto parsedValue = ConsumeString(tokens);
      if (!parsedValue)
      {
        return nullptr;
      }

      values.push_back(krys::move(parsedValue));
    }

    if (values.size() && !(values.size() % 2))
    {
      return ValueList::CreateSpaceSeparated(krys::move(values));
    }

    return nullptr;
  }

  static RefPtr<Value> ConsumeCounterContent(TokenRange args, PropertyParserState &state) noexcept
  {
    // counter()  =  counter( <counter-name>, <counter-style>? )
    // https://www.w3.org/TR/css-lists-3/#funcdef-counter

    auto maybeIdent = ConsumeCustomIdentRaw(args);
    if (maybeIdent == null)
    {
      return nullptr;
    }

    CSSOMStringAtom identifier {*maybeIdent};

    RefPtr<Value> counterStyle;
    if (ConsumeComma(args))
    {
      counterStyle = ConsumeCounterStyle(args, state);
      if (!counterStyle)
      {
        return nullptr;
      }
    }

    if (!counterStyle)
    {
      counterStyle = PrimitiveValue::Create(ValueId::Decimal);
    }

    if (!args.IsAtEnd())
    {
      return nullptr;
    }

    return CounterValue::Create(krys::move(identifier), CSSOMStringAtom::Null(), krys::move(counterStyle));
  }

  KRYS_NODISCARD static RefPtr<Value> ConsumeCountersContent(TokenRange args,
                                                                PropertyParserState &state) noexcept
  {
    // counters() = counters( <counter-name>, <string>, <counter-style>? )
    // https://www.w3.org/TR/css-lists-3/#funcdef-counters

    auto maybeIdent = ConsumeCustomIdentRaw(args);
    if (maybeIdent == null)
    {
      return nullptr;
    }

    CSSOMStringAtom identifier {*maybeIdent};
    if (!ConsumeComma(args) || args.Peek().Type() != TokenType::String)
    {
      return nullptr;
    }
    CSSOMStringAtom separator = args.Consume().IdentCodePoints();
    args.DiscardWhitespace();

    RefPtr<Value> counterStyle;
    if (ConsumeComma(args))
    {
      counterStyle = ConsumeCounterStyle(args, state);
      if (!counterStyle)
      {
        return nullptr;
      }
    }

    if (!counterStyle)
    {
      counterStyle = PrimitiveValue::Create(ValueId::Decimal);
    }

    if (!args.IsAtEnd())
    {
      return nullptr;
    }

    return CounterValue::Create(krys::move(identifier), krys::move(separator), krys::move(counterStyle));
  }

  RefPtr<Value> ConsumeContent(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // Standard says this should be:
    //
    // <'content'> = normal | none | [ <content-replacement> | <content-list> ] [/ [ <string> | <counter> |
    // <attr()> ]+ ]? https://drafts.csswg.org/css-content-3/#propdef-content

    if (IdentMatches<ValueId::None, ValueId::Normal>(tokens.Peek().ValueId()))
    {
      return ConsumeIdent(tokens);
    }

    enum class ContentListType : bool
    {
      VisibleContent,
      AltText
    };

    auto ConsumeContentList = [&](ValueListBuilder &values, ContentListType type) -> bool
    {
      bool shouldEnd = false;
      do
      {
        RefPtr<Value> parsedValue = ConsumeString(tokens);
        if (type == ContentListType::VisibleContent)
        {
          if (!parsedValue)
          {
            parsedValue = ConsumeImage(tokens, state);
          }

          if (!parsedValue)
          {
            parsedValue = ConsumeIdent<ValueId::OpenQuote, ValueId::CloseQuote, ValueId::NoOpenQuote,
                                       ValueId::NoCloseQuote>(tokens);
          }
        }
        if (!parsedValue)
        {
          if (tokens.Peek().FunctionId() == ValueId::Attr)
          {
            parsedValue = ConsumeAttr(ConsumeFunction(tokens), state);
          }
          // FIXME: Alt-text should support counters.
          else if (type == ContentListType::VisibleContent)
          {
            if (tokens.Peek().FunctionId() == ValueId::Counter)
            {
              parsedValue = ConsumeCounterContent(ConsumeFunction(tokens), state);
            }
            else if (tokens.Peek().FunctionId() == ValueId::Counters)
            {
              parsedValue = ConsumeCountersContent(ConsumeFunction(tokens), state);
            }
          }

          if (!parsedValue)
          {
            return false;
          }
        }
        values.push_back(krys::move(parsedValue));

        // Visible content parsing ends at '/' or end of tokens.
        if (type == ContentListType::VisibleContent && !tokens.IsAtEnd())
        {
          auto &value = tokens.Peek();
          if (value.Type() == TokenType::Delim && value.IdentCodePoints() == u8"/")
          {
            shouldEnd = true;
          }
        }

        shouldEnd = shouldEnd || tokens.IsAtEnd();
      } while (!shouldEnd);
      return true;
    };

    ValueListBuilder visibleContent;
    if (!ConsumeContentList(visibleContent, ContentListType::VisibleContent))
    {
      return nullptr;
    }

    // Consume alt-text content if there is any.
    if (ConsumeSlash(tokens))
    {
      ValueListBuilder altText;
      if (!ConsumeContentList(altText, ContentListType::AltText))
      {
        return nullptr;
      }

      return ValuePair::CreateSlashSeparated(
        ValueList::CreateSpaceSeparated(krys::move(visibleContent)),
        ValueList::CreateSpaceSeparated(krys::move(altText)));
    }

    return ValueList::CreateSpaceSeparated(krys::move(visibleContent));
  }
}