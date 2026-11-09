/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Values/FontFeatureValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css
{
  FontFeatureValue::FontFeatureValue(FontTag tag, Ref<Value> value) noexcept
      : Value(ValueType::FontFeature), _tag(tag), _value(krys::move(value))
  {
  }

  CSSOMString FontFeatureValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;
    result.push_back(u8'"');
    for (char c : _tag)
    {
      result.push_back(static_cast<char8_t>(c));
    }
    result.push_back(u8'"');
    // Omit the value if it's `1` as `1` is implied by default.
    if (auto *primitiveValue = DynamicDowncast<PrimitiveValue>(_value.get());
        !(primitiveValue && primitiveValue->ResolveAsIntegerIfNotCalculated<int>() == 1))
    {
      result.push_back(u8' ');
      result.append(_value->CssText(context));
    }
    return result;
  }

  bool FontFeatureValue::Equals(const FontFeatureValue &other) const noexcept
  {
    return _tag == other._tag && CompareValue(_value, other._value);
  }
}
