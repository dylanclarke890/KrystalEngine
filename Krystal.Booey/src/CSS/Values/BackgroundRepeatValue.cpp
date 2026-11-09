/*
 * Copyright (C) 2022-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/BackgroundRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  BackgroundRepeatValue::BackgroundRepeatValue(css::ValueId xValue, css::ValueId yValue) noexcept
      : Value(ValueType::BackgroundRepeat), _xValue(xValue), _yValue(yValue)
  {
  }

  Ref<BackgroundRepeatValue> BackgroundRepeatValue::Create(css::ValueId repeatXValue,
                                                           css::ValueId repeatYValue) noexcept
  {
    return AdoptRef(*new BackgroundRepeatValue(repeatXValue, repeatYValue));
  }

  CSSOMString BackgroundRepeatValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    // background-repeat/mask-repeat behave a little like a shorthand, but `repeat no-repeat` is transformed
    // to `repeat-x`.
    if (_xValue != _yValue)
    {
      if (_xValue == ValueId::Repeat && _yValue == ValueId::NoRepeat)
      {
        return CSSOMString(ToString(ValueId::RepeatX));
      }
      if (_xValue == ValueId::NoRepeat && _yValue == ValueId::Repeat)
      {
        return CSSOMString(ToString(ValueId::RepeatY));
      }

      CSSOMString result {ToString(_xValue)};
      result.push_back(u8' ');
      result.append(ToString(_yValue));
      return result;
    }
    return CSSOMString(ToString(_xValue));
  }

  bool BackgroundRepeatValue::Equals(const BackgroundRepeatValue &other) const noexcept
  {
    return _xValue == other._xValue && _yValue == other._yValue;
  }
}
