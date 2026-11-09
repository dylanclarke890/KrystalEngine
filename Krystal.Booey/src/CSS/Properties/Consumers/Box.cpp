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

#include "Krystal.Booey/CSS/Properties/Consumers/Box.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumeMarginTrim(TokenRange &tokens, PropertyParserState &) noexcept
  {
    // <'margin-trim'> = none | [ block || inline ] | [ block-start || inline-start || block-end || inline-end
    // ] https://drafts.csswg.org/css-box/#margin-trim

    auto firstValue = tokens.Peek().ValueId();
    if (firstValue == ValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    // FIXME: Multiple values should be appended in canonical order.
    SmallList<ValueId, 4uz> idents;
    if (firstValue == ValueId::Block || firstValue == ValueId::Inline)
    {
      while (auto ident = ConsumeIdentRaw<ValueId::Block, ValueId::Inline>(tokens))
      {
        if (std::ranges::contains(idents, *ident))
        {
          return nullptr;
        }

        idents.push_back(*ident);
      }
    }
    else
    {
      while (
        auto ident =
          ConsumeIdentRaw<ValueId::BlockStart, ValueId::BlockEnd, ValueId::InlineStart, ValueId::InlineEnd>(
            tokens))
      {
        if (std::ranges::contains(idents, *ident))
        {
          return nullptr;
        }

        idents.push_back(*ident);
      }

      // Try to serialize into either block or inline form
      if (idents.size() == 2uz)
      {
        if (std::ranges::contains(idents, ValueId::BlockStart)
            && std::ranges::contains(idents, ValueId::BlockEnd))
        {
          return PrimitiveValue::Create(ValueId::Block);
        }

        if (std::ranges::contains(idents, ValueId::InlineStart)
            && std::ranges::contains(idents, ValueId::InlineEnd))
        {
          return PrimitiveValue::Create(ValueId::Inline);
        }
      }
      else if (idents.size() == 4uz)
      {
        ValueListBuilder list;
        list.push_back(PrimitiveValue::Create(ValueId::Block));
        list.push_back(PrimitiveValue::Create(ValueId::Inline));

        return ValueList::CreateSpaceSeparated(krys::move(list));
      }
    }

    ValueListBuilder list;
    for (auto ident : idents)
    {
      list.push_back(PrimitiveValue::Create(ident));
    }

    return ValueList::CreateSpaceSeparated(krys::move(list));
  }
}