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

#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief An `attr()` value: an attribute name and an optional fallback, e.g. `attr(data-label, "none")`.
  class AttrValue final : public Value
  {
  private:
    CSSOMString _attributeName;
    RefPtr<Value> _fallback;

    AttrValue(CSSOMString &&attributeName, RefPtr<Value> &&fallback) noexcept
        : Value(ValueType::Attr), _attributeName(krys::move(attributeName)), _fallback(krys::move(fallback))
    {
    }

  public:
    KRYS_NODISCARD static Ref<AttrValue> Create(CSSOMString attributeName,
                                                RefPtr<Value> fallback = nullptr) noexcept;

    KRYS_NODISCARD const CSSOMString AttributeName() const noexcept
    {
      return _attributeName;
    }

    KRYS_NODISCARD const Value *Fallback() const noexcept
    {
      return _fallback.get();
    }

    KRYS_NODISCARD bool Equals(const AttrValue &other) const noexcept;
    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::AttrValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsAttrValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
