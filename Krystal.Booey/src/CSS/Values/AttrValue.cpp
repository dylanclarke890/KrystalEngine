/*
 * Copyright (C) 2024 Alexsander Borges Damaceno <alexbdamac@gmail.com>. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Values/AttrValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css
{
  Ref<AttrValue> AttrValue::Create(CSSOMString attributeName, RefPtr<Value> fallback) noexcept
  {
    return AdoptRef(*new AttrValue(krys::move(attributeName), krys::move(fallback)));
  }

  bool AttrValue::Equals(const AttrValue &other) const noexcept
  {
    auto *fallback = DynamicDowncast<PrimitiveValue>(_fallback.get());
    auto *otherFallback = DynamicDowncast<PrimitiveValue>(other._fallback.get());

    if (fallback && otherFallback)
    {
      return _attributeName == other._attributeName
             && fallback->StringValue() == otherFallback->StringValue();
    }
    if (fallback || otherFallback)
    {
      return false;
    }
    return _attributeName == other._attributeName;
  }

  CSSOMString AttrValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    auto *fallback = DynamicDowncast<PrimitiveValue>(_fallback.get());

    CSSOMString result {u8"attr("};
    result.append(_attributeName);
    if (fallback && !fallback->StringValue().empty())
    {
      result.append(u8", ");
      result.append(fallback->CssText(context));
    }
    result.push_back(u8')');
    return result;
  }
}
