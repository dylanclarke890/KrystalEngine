/*
 * Copyright (C) 2011-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Adobe Systems Incorporated. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/CrossFadeValue.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCrossfadeImage.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::css
{
  CrossFadeValue::CrossFadeValue(Ref<Value> &&fromValueOrNone, Ref<Value> &&toValueOrNone,
                                 Ref<PrimitiveValue> &&percentageValue) noexcept
      : Value(ValueType::CrossFade), _fromValueOrNone(krys::move(fromValueOrNone)),
        _toValueOrNone(krys::move(toValueOrNone)), _percentageValue(krys::move(percentageValue))
  {
  }

  Ref<CrossFadeValue> CrossFadeValue::Create(Ref<Value> &&fromValueOrNone, Ref<Value> &&toValueOrNone,
                                             Ref<PrimitiveValue> &&percentageValue) noexcept
  {
    return AdoptRef(*new CrossFadeValue(krys::move(fromValueOrNone), krys::move(toValueOrNone),
                                        krys::move(percentageValue)));
  }

  bool CrossFadeValue::Equals(const CrossFadeValue &other) const noexcept
  {
    return EqualInputImages(other) && CompareValue(_percentageValue, other._percentageValue);
  }

  bool CrossFadeValue::EqualInputImages(const CrossFadeValue &other) const noexcept
  {
    return CompareValue(_fromValueOrNone, other._fromValueOrNone)
           && CompareValue(_toValueOrNone, other._toValueOrNone);
  }

  CSSOMString CrossFadeValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result {u8"cross-fade("};
    result.append(_fromValueOrNone->CssText(context));
    result.append(u8", ");
    result.append(_toValueOrNone->CssText(context));
    result.append(u8", ");
    result.append(_percentageValue->CssText(context));
    result.push_back(u8')');
    return result;
  }

  RefPtr<StyleImage> CrossFadeValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    return StyleCrossfadeImage::Create(
      state.CreateStyleImage(*_fromValueOrNone), state.CreateStyleImage(*_toValueOrNone),
      std::clamp(_percentageValue->ValueDividingBy100IfPercentage<double>(state.CssToLengthConversionData()),
                 0.0, 1.0));
  }
}
