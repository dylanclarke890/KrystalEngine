/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/OffsetRotateValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  OffsetRotateValue::OffsetRotateValue(RefPtr<PrimitiveValue> &&modifier,
                                       RefPtr<PrimitiveValue> &&angle) noexcept
      : Value(ValueType::OffsetRotate), _modifier(krys::move(modifier)), _angle(krys::move(angle))
  {
    krys_debug_assert(_modifier || _angle);

    if (_modifier)
    {
      krys_debug_assert(_modifier->IsValueId());
    }

    if (_angle)
    {
      krys_debug_assert(_angle->IsAngle());
    }
  }

  Ref<OffsetRotateValue> OffsetRotateValue::Create(RefPtr<Value> modifier, RefPtr<Value> angle) noexcept
  {
    return AdoptRef(*new OffsetRotateValue(Downcast<PrimitiveValue>(krys::move(modifier)),
                                           Downcast<PrimitiveValue>(krys::move(angle))));
  }

  CSSOMString OffsetRotateValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;

    if (_modifier)
    {
      result.append(_modifier->CssText(context));
    }

    if (_angle)
    {
      if (!result.empty())
      {
        result.push_back(u8' ');
      }

      result.append(_angle->CssText(context));
    }

    return result;
  }

  bool OffsetRotateValue::IsInitialValue() const noexcept
  {
    return _modifier && _modifier->ValueId() == css::ValueId::Auto
           && (!_angle || _angle->IsZero().value_or(false));
  }

  bool OffsetRotateValue::Equals(const OffsetRotateValue &other) const noexcept
  {
    return CompareValuePtr(_modifier, other._modifier) && CompareValuePtr(_angle, other._angle);
  }
}
