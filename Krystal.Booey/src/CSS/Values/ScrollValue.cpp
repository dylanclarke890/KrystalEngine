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

#include "Krystal.Booey/CSS/Values/ScrollValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  ScrollValue::ScrollValue(RefPtr<Value> &&scroller, RefPtr<Value> &&axis) noexcept
      : Value(ValueType::Scroll), _scroller(krys::move(scroller)), _axis(krys::move(axis))
  {
  }

  Ref<ScrollValue> ScrollValue::Create() noexcept
  {
    return AdoptRef(*new ScrollValue(nullptr, nullptr));
  }

  Ref<ScrollValue> ScrollValue::Create(RefPtr<Value> &&scroller, RefPtr<Value> &&axis) noexcept
  {
    return AdoptRef(*new ScrollValue(krys::move(scroller), krys::move(axis)));
  }

  CSSOMString ScrollValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    bool hasScroller = _scroller && _scroller->ValueId() != ValueId::Nearest;
    bool hasAxis = _axis && _axis->ValueId() != ValueId::Block;

    CSSOMString result {u8"scroll("};
    if (hasScroller)
    {
      result.append(_scroller->CssText(context));
    }
    if (hasScroller && hasAxis)
    {
      result.push_back(u8' ');
    }
    if (hasAxis)
    {
      result.append(_axis->CssText(context));
    }
    result.push_back(u8')');
    return result;
  }

  bool ScrollValue::Equals(const ScrollValue &other) const noexcept
  {
    return CompareValuePtr(_scroller, other._scroller) && CompareValuePtr(_axis, other._axis);
  }
}
