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

#include "Krystal.Booey/Css/TypedOM/Numeric/CSSMathMin.hpp"
#include "Krystal.Booey/Css/Calc/Tree.hpp"

namespace krys::boo
{
  ExceptionOr<Ref<CSSMathMin>> CSSMathMin::Create(List<CSSNumberish> numberishes) noexcept
  {
    List<Ref<CSSNumericValue>> values;
    values.reserve(numberishes.size());
    for (auto &numberish : numberishes)
    {
      values.push_back(RectifyNumberish(krys::move(numberish)));
    }
    return Create(krys::move(values));
  }

  ExceptionOr<Ref<CSSMathMin>> CSSMathMin::Create(List<Ref<CSSNumericValue>> &&values) noexcept
  {
    if (values.empty())
    {
      return Exception {ExceptionCode::SyntaxError};
    }

    auto type = CSSNumericType::AddTypes(values);
    if (!type)
    {
      return Exception {ExceptionCode::TypeError};
    }

    return AdoptRef(*new CSSMathMin(krys::move(values), krys::move(*type)));
  }

  CSSMathMin::CSSMathMin(List<Ref<CSSNumericValue>> &&values, CSSNumericType &&type) noexcept
      : CSSMathValue(krys::move(type)), _values(CSSNumericArray::Create(krys::move(values)))
  {
  }

  void CSSMathMin::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#calc-serialization
    if (!arguments.contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(u8"min(");
    }
    _values->ForEach(
      [&](const CSSNumericValue &numericValue, bool first)
      {
        if (!first)
        {
          builder.Append(u8", ");
        }
        numericValue.Serialise(builder,
                               {SerialisationArguments::Nested, SerialisationArguments::WithoutParentheses});
      });
    if (!arguments.contains(SerialisationArguments::WithoutParentheses))
    {
      builder.Append(u8')');
    }
  }

  auto CSSMathMin::ToSumValue() const noexcept -> Maybe<SumValue>
  {
    // https://drafts.css-houdini.org/css-typed-om/#create-a-sum-value
    const auto &valuesArray = _values->Array();
    Maybe<SumValue> currentMax = valuesArray[0]->ToSumValue();
    if (!currentMax || currentMax->size() != 1)
    {
      return null;
    }
    for (size_t i = 1; i < valuesArray.size(); ++i)
    {
      auto currentValue = valuesArray[i]->ToSumValue();
      if (!currentValue || currentValue->size() != 1 || (*currentValue)[0].Units != (*currentMax)[0].Units)
      {
        return null;
      }
      if ((*currentValue)[0].Value < (*currentMax)[0].Value)
      {
        currentMax = krys::move(currentValue);
      }
    }
    return currentMax;
  }

  Maybe<css::calc::Child> CSSMathMin::ToCalcTreeNode() const noexcept
  {
    List<css::calc::Child> children;
    for (const auto &child : _values->Array())
    {
      if (auto node = child->ToCalcTreeNode())
      {
        children.push_back(krys::move(*node));
      }
    }
    if (children.empty())
    {
      return null;
    }

    auto operation = css::calc::Min {.Children = krys::move(children)};
    auto type = css::calc::ToType(operation);
    if (!type)
    {
      return null;
    }

    return css::calc::CreateChild(krys::move(operation), *type);
  }
}
