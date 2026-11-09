/*
 * Copyright (C) 2019 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Values/GridIntegerRepeatValue.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  GridIntegerRepeatValue::GridIntegerRepeatValue(Ref<PrimitiveValue> repetitions,
                                                 ValueListBuilder builder) noexcept
      : ValueContainingList(ValueType::GridIntegerRepeat, ValueSeparator::SpaceSeparator,
                            krys::move(builder)),
        _repetitions(krys::move(repetitions))
  {
  }

  Ref<GridIntegerRepeatValue> GridIntegerRepeatValue::Create(Ref<PrimitiveValue> repetitions,
                                                             ValueListBuilder builder) noexcept
  {
    return AdoptRef(*new GridIntegerRepeatValue(krys::move(repetitions), krys::move(builder)));
  }

  CSSOMString GridIntegerRepeatValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    StringBuilder result;
    result.Append(u8"repeat(");
    result.Append(_repetitions->CssText(context));
    result.Append(u8", ");
    SerialiseItems(result, context);
    result.Append(u8')');
    return krys::move(result).ToString();
  }

  bool GridIntegerRepeatValue::Equals(const GridIntegerRepeatValue &other) const noexcept
  {
    return CompareValue(_repetitions, other._repetitions) && ItemsEqual(other);
  }
}
