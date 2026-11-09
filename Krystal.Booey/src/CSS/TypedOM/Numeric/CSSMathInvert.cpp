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

#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathInvert.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"

namespace krys::boo::css
{
  Ref<CSSMathInvert> CSSMathInvert::Create(CSSNumberish &&numberish) noexcept
  {
    return AdoptRef(*new CSSMathInvert(krys::move(numberish)));
  }

  KRYS_NODISCARD static CSSNumericType NegatedType(const CSSNumberish &numberish) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#type-of-a-cssmathvalue
    return krys::SwitchOn(
      numberish, [](double) { return CSSNumericType(); },
      [](const RefPtr<CSSNumericValue> &value)
      {
        if (!value)
        {
          return CSSNumericType();
        }
        CSSNumericType type = value->Type();
        auto negate = [](CSSNumericType::BaseTypeStorage &optional)
        {
          if (optional)
          {
            optional = *optional * -1;
          }
        };
        negate(type.Length);
        negate(type.Angle);
        negate(type.Time);
        negate(type.Frequency);
        negate(type.Resolution);
        negate(type.Flex);
        negate(type.Percent);
        return type;
      });
  }

  CSSMathInvert::CSSMathInvert(CSSNumberish &&numberish) noexcept
      : CSSMathValue(NegatedType(numberish)), _value(RectifyNumberish(krys::move(numberish)))
  {
  }

  void CSSMathInvert::Serialise(StringBuilder &builder,
                                EnumSet<SerialisationArguments> arguments) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#calc-serialization
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(arguments.Contains(SerialisationArguments::Nested) ? u8"(" : u8"calc(");
    }
    builder.Append(u8"1 / ");
    _value->Serialise(builder, arguments);
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(u8')');
    }
  }

  auto CSSMathInvert::ToSumValue() const noexcept -> Maybe<SumValue>
  {
    // https://drafts.css-houdini.org/css-typed-om/#create-a-sum-value
    auto values = _value->ToSumValue();
    if (!values)
    {
      return null;
    }
    if (values->size() != 1)
    {
      return null;
    }
    auto &value = (*values)[0];
    if (!value.Value)
    {
      return null;
    }
    value.Value = 1.0 / value.Value;

    UnitMap negatedExponents;
    for (const auto &[unit, exponent] : value.Units)
    {
      negatedExponents.try_emplace(unit, -1 * exponent);
    }
    value.Units = krys::move(negatedExponents);

    return values;
  }

  bool CSSMathInvert::Equals(const CSSNumericValue &other) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#equal-numeric-value
    auto *otherInvert = DynamicDowncast<CSSMathInvert>(other);
    if (!otherInvert)
    {
      return false;
    }
    return _value->Equals(otherInvert->Value());
  }

  Maybe<calc::Child> CSSMathInvert::ToCalcTreeNode() const noexcept
  {
    auto child = _value->ToCalcTreeNode();
    if (!child)
    {
      return null;
    }

    auto operation = calc::Invert {.A = krys::move(*child)};
    auto type = calc::ToType(operation);
    if (!type)
    {
      return null;
    }

    return calc::CreateChild(krys::move(operation), *type);
  }
}
