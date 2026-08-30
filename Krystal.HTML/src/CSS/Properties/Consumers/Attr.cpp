#include "Krystal.HTML/CSS/Properties/Consumers/Attr.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSAttrValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeAttr(CSSTokenRange tokens, CSSPropertyParserState &state) noexcept
  {
    // Standard says this should be:
    //
    // <attr()>    = attr( <attr-name> <attr-type>? , <declaration-value>?)
    // <attr-name> = [ <ident-token> '|' ]? <ident-token>
    // <attr-type> = type( <syntax> ) | string | <attr-unit>
    // https://drafts.csswg.org/css-values-5/#funcdef-attr

    // FIXME: Add support for complete <attr-name> syntax, including namespace support.
    // FIXME: Add support for <attr-type> syntax

    if (tokens.Peek().Type() != CSSTokenType::Ident)
    {
      return nullptr;
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    CSSOMStringAtom attrName = CSSOMStringAtom::Null();
    if (state.Context.IsHTMLDocument)
    {
      attrName = Krys::Text::ToASCIILowercase(token.IdentCodePoints());
    }
    else
    {
      attrName = token.IdentCodePoints();
    }

    if (!tokens.IsAtEnd() && !ConsumeComma(tokens))
    {
      return nullptr;
    }

    RefPtr<CSSValue> fallback;
    if (tokens.Peek().Type() == CSSTokenType::String)
    {
      fallback = CSSPrimitiveValue::Create(tokens.Consume().IdentCodePoints());
      tokens.DiscardWhitespace();
    }

    if (!tokens.IsAtEnd())
    {
      return nullptr;
    }

    auto attr = CSSAttrValue::Create(Krys::Move(attrName), Krys::Move(fallback));

    // FIXME: Consider moving to a CSSFunctionValue with a custom-ident rather than a special CSS_ATTR
    // primitive value.
    return CSSPrimitiveValue::Create(Krys::Move(attr));
  }
}