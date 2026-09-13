#include "Krystal.Booey/CSS/Properties/Consumers/Attr.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSAttrValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeAttr(TokenRange tokens, PropertyParserState &state) noexcept
  {
    // Standard says this should be:
    //
    // <attr()>    = attr( <attr-name> <attr-type>? , <declaration-value>?)
    // <attr-name> = [ <ident-token> '|' ]? <ident-token>
    // <attr-type> = type( <syntax> ) | string | <attr-unit>
    // https://drafts.csswg.org/css-values-5/#funcdef-attr

    // FIXME: Add support for complete <attr-name> syntax, including namespace support.
    // FIXME: Add support for <attr-type> syntax

    if (tokens.Peek().Type() != TokenType::Ident)
    {
      return nullptr;
    }

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    CSSOMStringAtom attrName = CSSOMStringAtom::Null();
    if (state.Context.IsHTMLDocument)
    {
      attrName = krys::text::ToASCIILower(token.IdentCodePoints());
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
    if (tokens.Peek().Type() == TokenType::String)
    {
      fallback = CSSPrimitiveValue::Create(tokens.Consume().IdentCodePoints());
      tokens.DiscardWhitespace();
    }

    if (!tokens.IsAtEnd())
    {
      return nullptr;
    }

    auto attr = CSSAttrValue::Create(krys::move(attrName), krys::move(fallback));

    // FIXME: Consider moving to a CSSFunctionValue with a custom-ident rather than a special CSS_ATTR
    // primitive value.
    return CSSPrimitiveValue::Create(krys::move(attr));
  }
}