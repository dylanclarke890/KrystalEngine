/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/BorderImageSliceValue.hpp"

namespace krys::boo::css
{
  BorderImageSliceValue::BorderImageSliceValue(Quad slices, bool fill) noexcept
      : Value(ValueType::BorderImageSlice), _slices(krys::move(slices)), _fill(fill)
  {
  }

  Ref<BorderImageSliceValue> BorderImageSliceValue::Create(Quad slices, bool fill) noexcept
  {
    return AdoptRef(*new BorderImageSliceValue(krys::move(slices), fill));
  }

  CSSOMString BorderImageSliceValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    if (_fill)
    {
      return _slices.CssText(context) + u8" fill";
    }
    return _slices.CssText(context);
  }

  bool BorderImageSliceValue::Equals(const BorderImageSliceValue &other) const noexcept
  {
    return _fill == other._fill && _slices.Equals(other._slices);
  }
}
