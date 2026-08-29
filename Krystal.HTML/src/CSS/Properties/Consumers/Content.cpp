#include "Krystal.HTML/CSS/Properties/Consumers/Content.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Attr.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CounterStyles.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/String.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSCounterValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePair.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeQuotes(CSSTokenRange &tokens, CSSPropertyParserState &) noexcept
  {
    // <'quotes'> = auto | none | match-parent | [ <string> <string> ]+
    // https://drafts.csswg.org/css-content-3/#propdef-quotes

    // FIXME: Support `match-parent`.

    auto id = tokens.Peek().ValueId();
    if (id == CSSValueId::None || id == CSSValueId::Auto)
    {
      return ConsumeIdent(tokens);
    }

    CSSValueListBuilder values;
    while (!tokens.IsAtEnd())
    {
      auto parsedValue = ConsumeString(tokens);
      if (!parsedValue)
      {
        return nullptr;
      }

      values.push_back(Krys::Move(parsedValue));
    }

    if (values.size() && !(values.size() % 2))
    {
      return CSSValueList::CreateSpaceSeparated(Krys::Move(values));
    }

    return nullptr;
  }

  static RefPtr<CSSValue> ConsumeCounterContent(CSSTokenRange args, CSSPropertyParserState &state) noexcept
  {
    // counter()  =  counter( <counter-name>, <counter-style>? )
    // https://www.w3.org/TR/css-lists-3/#funcdef-counter

    auto maybeIdent = ConsumeCustomIdentRaw(args);
    if (maybeIdent == Null)
    {
      return nullptr;
    }

    CSSOMStringAtom identifier {*maybeIdent};

    RefPtr<CSSValue> counterStyle;
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
      counterStyle = CSSPrimitiveValue::Create(CSSValueId::Decimal);
    }

    if (!args.IsAtEnd())
    {
      return nullptr;
    }

    return CSSCounterValue::Create(Krys::Move(identifier), CSSOMStringAtom::Null(), Krys::Move(counterStyle));
  }

  KRYS_NODISCARD static RefPtr<CSSValue> ConsumeCountersContent(CSSTokenRange args,
                                                                CSSPropertyParserState &state) noexcept
  {
    // counters() = counters( <counter-name>, <string>, <counter-style>? )
    // https://www.w3.org/TR/css-lists-3/#funcdef-counters

    auto maybeIdent = ConsumeCustomIdentRaw(args);
    if (maybeIdent == Null)
    {
      return nullptr;
    }

    CSSOMStringAtom identifier {*maybeIdent};
    if (!ConsumeComma(args) || args.Peek().Type() != CSSTokenType::String)
    {
      return nullptr;
    }
    CSSOMStringAtom separator = args.Consume().IdentCodePoints();
    args.DiscardWhitespace();

    RefPtr<CSSValue> counterStyle;
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
      counterStyle = CSSPrimitiveValue::Create(CSSValueId::Decimal);
    }

    if (!args.IsAtEnd())
    {
      return nullptr;
    }

    return CSSCounterValue::Create(Krys::Move(identifier), Krys::Move(separator), Krys::Move(counterStyle));
  }

  RefPtr<CSSValue> ConsumeContent(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // Standard says this should be:
    //
    // <'content'> = normal | none | [ <content-replacement> | <content-list> ] [/ [ <string> | <counter> |
    // <attr()> ]+ ]? https://drafts.csswg.org/css-content-3/#propdef-content

    if (IdentMatches<CSSValueId::None, CSSValueId::Normal>(tokens.Peek().ValueId()))
    {
      return ConsumeIdent(tokens);
    }

    enum class ContentListType : bool
    {
      VisibleContent,
      AltText
    };

    auto ConsumeContentList = [&](CSSValueListBuilder &values, ContentListType type) -> bool
    {
      bool shouldEnd = false;
      do
      {
        RefPtr<CSSValue> parsedValue = ConsumeString(tokens);
        if (type == ContentListType::VisibleContent)
        {
          if (!parsedValue)
          {
            parsedValue = ConsumeImage(tokens, state);
          }

          if (!parsedValue)
          {
            parsedValue = ConsumeIdent<CSSValueId::OpenQuote, CSSValueId::CloseQuote, CSSValueId::NoOpenQuote,
                                       CSSValueId::NoCloseQuote>(tokens);
          }
        }
        if (!parsedValue)
        {
          if (tokens.Peek().FunctionId() == CSSValueId::Attr)
          {
            parsedValue = ConsumeAttr(ConsumeFunction(tokens), state);
          }
          // FIXME: Alt-text should support counters.
          else if (type == ContentListType::VisibleContent)
          {
            if (tokens.Peek().FunctionId() == CSSValueId::Counter)
            {
              parsedValue = ConsumeCounterContent(ConsumeFunction(tokens), state);
            }
            else if (tokens.Peek().FunctionId() == CSSValueId::Counters)
            {
              parsedValue = ConsumeCountersContent(ConsumeFunction(tokens), state);
            }
          }

          if (!parsedValue)
          {
            return false;
          }
        }
        values.push_back(Krys::Move(parsedValue));

        // Visible content parsing ends at '/' or end of tokens.
        if (type == ContentListType::VisibleContent && !tokens.IsAtEnd())
        {
          auto &value = tokens.Peek();
          if (value.Type() == CSSTokenType::Delim && value.IdentCodePoints() == u8"/")
          {
            shouldEnd = true;
          }
        }

        shouldEnd = shouldEnd || tokens.IsAtEnd();
      } while (!shouldEnd);
      return true;
    };

    CSSValueListBuilder visibleContent;
    if (!ConsumeContentList(visibleContent, ContentListType::VisibleContent))
    {
      return nullptr;
    }

    // Consume alt-text content if there is any.
    if (ConsumeSlash(tokens))
    {
      CSSValueListBuilder altText;
      if (!ConsumeContentList(altText, ContentListType::AltText))
      {
        return nullptr;
      }

      return CSSValuePair::CreateSlashSeparated(
        CSSValueList::CreateSpaceSeparated(Krys::Move(visibleContent)),
        CSSValueList::CreateSpaceSeparated(Krys::Move(altText)));
    }

    return CSSValueList::CreateSpaceSeparated(Krys::Move(visibleContent));
  }
}