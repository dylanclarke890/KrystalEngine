/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSSkewY.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSSkewY>> CSSSkewY::Create(Ref<CSSNumericValue> ay) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssskewy-cssskewy
    // 1. If ay does not match <angle>, throw a TypeError.
    if (!ay->Type().Matches<CSSNumericBaseType::Angle>())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    // 2. Return a new CSSSkewY object with its ay internal slot set to ay, and its is2D internal slot
    //    set to true.
    return AdoptRef(*new CSSSkewY(krys::move(ay)));
  }

  dom::ExceptionOr<Ref<CSSSkewY>> CSSSkewY::Create(const FunctionValue &function,
                                                   dom::Document &document) noexcept
  {
    auto reified = ReifyArguments(function, document);
    if (reified.HasException())
    {
      return reified.ReleaseException();
    }
    auto arguments = reified.ReleaseValue();
    if (function.Name() != ValueId::SkewY || arguments.size() != 1)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    return Create(krys::move(arguments[0]));
  }

  CSSSkewY::CSSSkewY(Ref<CSSNumericValue> ay) noexcept : CSSTransformComponent(true), _ay(krys::move(ay))
  {
  }

  dom::ExceptionOr<void> CSSSkewY::SetAy(Ref<CSSNumericValue> ay) noexcept
  {
    if (!ay->Type().Matches<CSSNumericBaseType::Angle>())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _ay = krys::move(ay);
    return {};
  }

  void CSSSkewY::Serialise(StringBuilder &builder) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#serialize-a-cssskewy
    builder.Append(u8"skewY(");
    _ay->Serialise(builder);
    builder.Append(u8')');
  }

  dom::ExceptionOr<Ref<DOMMatrix>> CSSSkewY::ToMatrix() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-tomatrix
    const auto *unitValue = DynamicDowncast<CSSUnitValue>(*_ay);
    if (!unitValue)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    auto angle = unitValue->ConvertTo(UnitType::deg);
    if (!angle)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    TransformationMatrix matrix;
    matrix.SkewY(angle->Value());
    return DOMMatrix::Create(matrix, DOMMatrixReadOnly::Is2D::Yes);
  }

  RefPtr<Value> CSSSkewY::ToCSSValue() const noexcept
  {
    auto angle = _ay->ToCSSValue();
    if (!angle)
    {
      return nullptr;
    }
    return FunctionValue::Create(ValueId::SkewY, Ref<Value>(krys::move(angle)));
  }
}
