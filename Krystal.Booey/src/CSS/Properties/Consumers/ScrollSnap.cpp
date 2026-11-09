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

#include "Krystal.Booey/CSS/Properties/Consumers/ScrollSnap.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumeScrollSnapType(TokenRange &tokens, PropertyParserState &) noexcept
  {
    // <'scroll-snap-type'> = none | [ x | y | block | inline | both ] [ mandatory | proximity
    // ]?@(default=proximity) https://drafts.csswg.org/css-scroll-snap-1/#scroll-snap-type

    auto firstValue =
      ConsumeIdentRaw<ValueId::None, ValueId::X, ValueId::Y, ValueId::Block, ValueId::Inline, ValueId::Both>(
        tokens);
    if (!firstValue)
    {
      return nullptr;
    }

    if (*firstValue == ValueId::None)
    {
      return PrimitiveValue::Create(ValueId::None);
    }

    // We only add the second value if it is not the initial value as described in specification
    // so that serialization of this ValueList produces the canonical serialization.

    auto secondValue = ConsumeIdentRaw<ValueId::Proximity, ValueId::Mandatory>(tokens);
    if (secondValue.value_or(ValueId::Proximity) == ValueId::Proximity)
    {
      return PrimitiveValue::Create(*firstValue);
    }

    return ValueList::CreateSpaceSeparated(PrimitiveValue::Create(*firstValue),
                                           PrimitiveValue::Create(ValueId::Mandatory));
  }
}