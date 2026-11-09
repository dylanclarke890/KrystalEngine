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

#include "Krystal.Booey/CSS/Properties/Consumers/Attr.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/AttrValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumeAttr(TokenRange tokens, PropertyParserState &state) noexcept
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

    CSSOMString attrName;
    if (state.Context.IsHTMLDocument)
    {
      attrName = krys::text::ToASCIILower(token.IdentCodePoints());
    }
    else
    {
      attrName = CSSOMString(token.IdentCodePoints());
    }

    if (!tokens.IsAtEnd() && !ConsumeComma(tokens))
    {
      return nullptr;
    }

    RefPtr<Value> fallback;
    if (tokens.Peek().Type() == TokenType::String)
    {
      fallback = PrimitiveValue::Create(CSSOMString(tokens.Consume().IdentCodePoints()));
      tokens.DiscardWhitespace();
    }

    if (!tokens.IsAtEnd())
    {
      return nullptr;
    }

    auto attr = AttrValue::Create(krys::move(attrName), krys::move(fallback));

    // FIXME: Consider moving to a FunctionValue with a custom-ident rather than a special CSS_ATTR
    // primitive value.
    return PrimitiveValue::Create(krys::move(attr));
  }
}