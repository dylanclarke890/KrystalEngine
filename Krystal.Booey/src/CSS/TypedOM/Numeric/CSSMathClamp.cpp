/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathClamp.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include <algorithm>

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSMathClamp>> CSSMathClamp::Create(CSSNumberish &&lower, CSSNumberish &&value,
                                                           CSSNumberish &&upper) noexcept
  {
    auto rectifiedLower = RectifyNumberish(krys::move(lower));
    auto rectifiedValue = RectifyNumberish(krys::move(value));
    auto rectifiedUpper = RectifyNumberish(krys::move(upper));

    auto addedType = CSSNumericType::AddTypes(rectifiedLower->Type(), rectifiedValue->Type());
    if (!addedType)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    addedType = CSSNumericType::AddTypes(*addedType, rectifiedUpper->Type());
    if (!addedType)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    return AdoptRef(*new CSSMathClamp(krys::move(*addedType), krys::move(rectifiedLower),
                                      krys::move(rectifiedValue), krys::move(rectifiedUpper)));
  }

  CSSMathClamp::CSSMathClamp(CSSNumericType &&type, Ref<CSSNumericValue> &&lower,
                             Ref<CSSNumericValue> &&value, Ref<CSSNumericValue> &&upper) noexcept
      : CSSMathValue(krys::move(type)), _lower(krys::move(lower)), _value(krys::move(value)),
        _upper(krys::move(upper))
  {
  }

  void CSSMathClamp::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments>) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#calc-serialization
    EnumSet<SerialisationArguments> serialisationArguments {SerialisationArguments::Nested,
                                                            SerialisationArguments::WithoutParentheses};
    builder.Append(u8"clamp(");
    _lower->Serialise(builder, serialisationArguments);
    builder.Append(u8", ");
    _value->Serialise(builder, serialisationArguments);
    builder.Append(u8", ");
    _upper->Serialise(builder, serialisationArguments);
    builder.Append(u8')');
  }

  auto CSSMathClamp::ToSumValue() const noexcept -> Maybe<SumValue>
  {
    auto validateSumValue = [](const Maybe<SumValue> &sumValue, const UnitMap *expectedUnits)
    {
      return sumValue && sumValue->size() == 1
             && (!expectedUnits || *expectedUnits == sumValue->front().Units);
    };

    auto lower = _lower->ToSumValue();
    if (!validateSumValue(lower, nullptr))
    {
      return null;
    }
    auto value = _value->ToSumValue();
    if (!validateSumValue(value, &lower->front().Units))
    {
      return null;
    }
    auto upper = _upper->ToSumValue();
    if (!validateSumValue(upper, &lower->front().Units))
    {
      return null;
    }

    value->front().Value =
      std::max(lower->front().Value, std::min(value->front().Value, upper->front().Value));
    return value;
  }

  bool CSSMathClamp::Equals(const CSSNumericValue &other) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#equal-numeric-value
    auto *otherClamp = DynamicDowncast<CSSMathClamp>(other);
    if (!otherClamp)
    {
      return false;
    }
    return _lower->Equals(*otherClamp->_lower) && _value->Equals(*otherClamp->_value)
           && _upper->Equals(*otherClamp->_upper);
  }

  Maybe<calc::Child> CSSMathClamp::ToCalcTreeNode() const noexcept
  {
    auto lower = _lower->ToCalcTreeNode();
    if (!lower)
    {
      return null;
    }
    auto value = _value->ToCalcTreeNode();
    if (!value)
    {
      return null;
    }
    auto upper = _upper->ToCalcTreeNode();
    if (!upper)
    {
      return null;
    }

    auto clamp =
      calc::Clamp {.Min = krys::move(*lower), .Val = krys::move(*value), .Max = krys::move(*upper)};
    auto type = calc::ToType(clamp);
    if (!type)
    {
      return null;
    }

    return calc::CreateChild(krys::move(clamp), *type);
  }
}
