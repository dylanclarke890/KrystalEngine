/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Values/ViewValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  ViewValue::ViewValue(RefPtr<Value> &&axis, RefPtr<Value> &&startInset, RefPtr<Value> &&endInset) noexcept
      : Value(ValueType::View), _axis(krys::move(axis)), _startInset(krys::move(startInset)),
        _endInset(krys::move(endInset))
  {
  }

  Ref<ViewValue> ViewValue::Create() noexcept
  {
    return AdoptRef(*new ViewValue(nullptr, nullptr, nullptr));
  }

  Ref<ViewValue> ViewValue::Create(RefPtr<Value> &&axis, RefPtr<Value> &&startInset,
                                   RefPtr<Value> &&endInset) noexcept
  {
    return AdoptRef(*new ViewValue(krys::move(axis), krys::move(startInset), krys::move(endInset)));
  }

  CSSOMString ViewValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    bool hasAxis = _axis && _axis->ValueId() != ValueId::Block;
    bool hasEndInset = _endInset && _endInset != _startInset;
    bool hasStartInset = (_startInset && _startInset->ValueId() != ValueId::Auto)
                         || (_startInset && _startInset->ValueId() == ValueId::Auto && hasEndInset);

    CSSOMString result {u8"view("};
    if (hasAxis)
    {
      result.append(_axis->CssText(context));
    }
    if (hasAxis && hasStartInset)
    {
      result.push_back(u8' ');
    }
    if (hasStartInset)
    {
      result.append(_startInset->CssText(context));
    }
    if (hasStartInset && hasEndInset)
    {
      result.push_back(u8' ');
    }
    if (hasEndInset)
    {
      result.append(_endInset->CssText(context));
    }
    result.push_back(u8')');
    return result;
  }

  bool ViewValue::Equals(const ViewValue &other) const noexcept
  {
    return CompareValuePtr(_axis, other._axis) && CompareValuePtr(_startInset, other._startInset)
           && CompareValuePtr(_endInset, other._endInset);
  }
}
