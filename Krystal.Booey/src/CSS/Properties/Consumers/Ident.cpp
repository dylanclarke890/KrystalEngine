/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<PrimitiveValue> ConsumeIdent(TokenRange &tokens) noexcept
  {
    if (auto result = ConsumeIdentRaw(tokens))
    {
      return PrimitiveValue::Create(*result);
    }

    return nullptr;
  }

  RefPtr<PrimitiveValue> ConsumeIdentRange(TokenRange &tokens, ValueId lower, ValueId upper) noexcept
  {
    auto value = ConsumeIdentRangeRaw(tokens, lower, upper);
    if (!value)
    {
      return nullptr;
    }

    return PrimitiveValue::Create(*value);
  }

  RefPtr<PrimitiveValue> ConsumeCustomIdent(TokenRange &tokens, bool shouldLowercase) noexcept
  {
    auto identifier = ConsumeCustomIdentRaw(tokens, shouldLowercase);
    if (!identifier)
    {
      return nullptr;
    }

    return PrimitiveValue::CreateCustomIdent(*identifier);
  }

  RefPtr<PrimitiveValue> ConsumeDashedIdent(TokenRange &tokens, bool shouldLowercase) noexcept
  {
    auto identifier = ConsumeDashedIdentRaw(tokens, shouldLowercase);
    if (!identifier)
    {
      return nullptr;
    }

    return PrimitiveValue::CreateCustomIdent(*identifier);
  }
}
