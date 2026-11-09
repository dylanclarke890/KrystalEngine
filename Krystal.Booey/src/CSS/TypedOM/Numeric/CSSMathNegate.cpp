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

#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathNegate.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"

namespace krys::boo::css
{
  Ref<CSSMathNegate> CSSMathNegate::Create(CSSNumberish &&numberish) noexcept
  {
    return AdoptRef(*new CSSMathNegate(krys::move(numberish)));
  }

  KRYS_NODISCARD static CSSNumericType CopyType(const CSSNumberish &numberish) noexcept
  {
    return krys::SwitchOn(
      numberish, [](double) { return CSSNumericType(); },
      [](const RefPtr<CSSNumericValue> &value)
      {
        if (!value)
        {
          return CSSNumericType();
        }
        return value->Type();
      });
  }

  CSSMathNegate::CSSMathNegate(CSSNumberish &&numberish) noexcept
      : CSSMathValue(CopyType(numberish)), _value(RectifyNumberish(krys::move(numberish)))
  {
  }

  void CSSMathNegate::Serialise(StringBuilder &builder,
                                EnumSet<SerialisationArguments> arguments) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#calc-serialization
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(arguments.Contains(SerialisationArguments::Nested) ? u8"(" : u8"calc(");
    }
    builder.Append(u8"-");
    _value->Serialise(builder, arguments);
    if (!arguments.Contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(u8')');
    }
  }

  auto CSSMathNegate::ToSumValue() const noexcept -> Maybe<SumValue>
  {
    // https://drafts.css-houdini.org/css-typed-om/#create-a-sum-value
    auto values = _value->ToSumValue();
    if (!values)
    {
      return null;
    }
    for (auto &value : *values)
    {
      value.Value = value.Value * -1;
    }
    return values;
  }

  bool CSSMathNegate::Equals(const CSSNumericValue &other) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#equal-numeric-value
    auto *otherNegate = DynamicDowncast<CSSMathNegate>(other);
    if (!otherNegate)
    {
      return false;
    }
    return _value->Equals(otherNegate->Value());
  }

  Maybe<calc::Child> CSSMathNegate::ToCalcTreeNode() const noexcept
  {
    auto child = _value->ToCalcTreeNode();
    if (!child)
    {
      return null;
    }

    auto operation = calc::Negate {.A = krys::move(*child)};
    auto type = calc::ToType(operation);
    if (!type)
    {
      return null;
    }

    return calc::CreateChild(krys::move(operation), *type);
  }
}
