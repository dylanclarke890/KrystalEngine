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

#include "Krystal.Web/Css/Properties/Consumers/WillChange.hpp"
#include "Krystal.Web/Css/Parser/KeywordLookup.hpp"
#include "Krystal.Web/Css/Properties/Consumers/Ident.hpp"
#include "Krystal.Web/Css/Properties/Consumers/Primitives.hpp"
#include "Krystal.Web/Css/Properties/PropertyParserState.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Css/Values/ValueList.hpp"
#include "Krystal.Web/Css/Values/ValueListBuilder.hpp"

namespace krys::web::css::parse_property
{
  KRYS_NODISCARD RefPtr<Value> ConsumeWillChange(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'will-change'> = auto | <animateable-feature>#
    // https://drafts.csswg.org/css-will-change/#propdef-will-change

    if (tokens.Peek().ValueId() == ValueId::Auto)
    {
      return ConsumeIdent(tokens);
    }

    // Every comma-separated list of identifiers is a valid will-change value, unless the list includes an
    // explicitly disallowed identifier.
    ValueListBuilder values;
    while (!tokens.IsAtEnd())
    {
      switch (tokens.Peek().ValueId())
      {
        case ValueId::Contents:
        case ValueId::ScrollPosition:
        {
          values.push_back(ConsumeIdent(tokens));
          break;
        }
        case ValueId::None:
        case ValueId::All:
        case ValueId::Auto:
        {
          return nullptr;
        }
        default:
        {
          if (tokens.Peek().Type() != TokenType::Ident)
          {
            return nullptr;
          }

          PropertyId propertyId = PropertyIdFromName(tokens.Peek().IdentCodePoints());
          if (propertyId == PropertyId::WillChange)
          {
            return nullptr;
          }

          if (!IsExposed(propertyId, &state.Context.PropertySettings))
          {
            propertyId = PropertyId::Invalid;
          }

          if (propertyId != PropertyId::Invalid)
          {
            values.push_back(PrimitiveValue::Create(propertyId));

            tokens.Discard();
            tokens.DiscardWhitespace();

            break;
          }

          if (auto customIdent = ConsumeCustomIdent(tokens))
          {
            // Append properties we don't recognize, but that are legal.
            values.push_back(krys::move(customIdent));
            break;
          }

          return nullptr;
        }
      }

      // This is a comma separated list
      if (!tokens.IsAtEnd() && !ConsumeComma(tokens))
      {
        return nullptr;
      }
    }

    return ValueList::CreateCommaSeparated(krys::move(values));
  }
}
