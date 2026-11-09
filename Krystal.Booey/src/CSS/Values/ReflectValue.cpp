/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/ReflectValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  ReflectValue::ReflectValue(css::ValueId direction, Ref<Value> offset, RefPtr<Value> mask) noexcept
      : Value(ValueType::Reflect), _direction(direction), _offset(krys::move(offset)), _mask(krys::move(mask))
  {
  }

  Ref<ReflectValue> ReflectValue::Create(css::ValueId direction, Ref<Value> offset,
                                         RefPtr<Value> mask) noexcept
  {
    return AdoptRef(*new ReflectValue(direction, krys::move(offset), krys::move(mask)));
  }

  CSSOMString ReflectValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result {ToString(_direction)};
    result.push_back(u8' ');
    result.append(_offset->CssText(context));
    if (_mask)
    {
      result.push_back(u8' ');
      result.append(_mask->CssText(context));
    }
    return result;
  }

  bool ReflectValue::Equals(const ReflectValue &other) const noexcept
  {
    return _direction == other._direction && CompareValue(_offset, other._offset)
           && CompareValuePtr(_mask, other._mask);
  }
}
