/*
 * Copyright (C) 2017-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/FontStyleWithAngleValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  FontStyleWithAngleValue::FontStyleWithAngleValue(ObliqueAngle &&obliqueAngle) noexcept
      : Value(ValueType::FontStyleWithAngle), _obliqueAngle(krys::move(obliqueAngle))
  {
  }

  Ref<FontStyleWithAngleValue> FontStyleWithAngleValue::Create(ObliqueAngle obliqueAngle) noexcept
  {
    return AdoptRef(*new FontStyleWithAngleValue(krys::move(obliqueAngle)));
  }

  CSSOMString FontStyleWithAngleValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    if (_obliqueAngle.IsKnownZero())
    {
      return CSSOMString(ToLower(ValueId::Normal));
    }

    CSSOMString result {ToLower(ValueId::Oblique)};
    result.push_back(u8' ');
    result.append(Serialise(context, _obliqueAngle));
    return result;
  }

  bool FontStyleWithAngleValue::Equals(const FontStyleWithAngleValue &other) const noexcept
  {
    return _obliqueAngle == other._obliqueAngle;
  }

  IterationStatus FontStyleWithAngleValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _obliqueAngle);
  }
}
