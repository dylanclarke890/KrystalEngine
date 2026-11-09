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

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSSkewX.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSSkewX>> CSSSkewX::Create(Ref<CSSNumericValue> ax) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssskewx-cssskewx
    // 1. If ax does not match <angle>, throw a TypeError.
    if (!ax->Type().Matches<CSSNumericBaseType::Angle>())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    // 2. Return a new CSSSkewX object with its ax internal slot set to ax, and its is2D internal slot
    //    set to true.
    return AdoptRef(*new CSSSkewX(krys::move(ax)));
  }

  dom::ExceptionOr<Ref<CSSSkewX>> CSSSkewX::Create(const FunctionValue &function,
                                                   dom::Document &document) noexcept
  {
    auto reified = ReifyArguments(function, document);
    if (reified.HasException())
    {
      return reified.ReleaseException();
    }
    auto arguments = reified.ReleaseValue();
    if (function.Name() != ValueId::SkewX || arguments.size() != 1)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    return Create(krys::move(arguments[0]));
  }

  CSSSkewX::CSSSkewX(Ref<CSSNumericValue> ax) noexcept : CSSTransformComponent(true), _ax(krys::move(ax))
  {
  }

  dom::ExceptionOr<void> CSSSkewX::SetAx(Ref<CSSNumericValue> ax) noexcept
  {
    if (!ax->Type().Matches<CSSNumericBaseType::Angle>())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _ax = krys::move(ax);
    return {};
  }

  void CSSSkewX::Serialise(StringBuilder &builder) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#serialize-a-cssskewx
    builder.Append(u8"skewX(");
    _ax->Serialise(builder);
    builder.Append(u8')');
  }

  dom::ExceptionOr<Ref<DOMMatrix>> CSSSkewX::ToMatrix() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-tomatrix
    const auto *unitValue = DynamicDowncast<CSSUnitValue>(*_ax);
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
    matrix.SkewX(angle->Value());
    return DOMMatrix::Create(matrix, DOMMatrixReadOnly::Is2D::Yes);
  }

  RefPtr<Value> CSSSkewX::ToCSSValue() const noexcept
  {
    auto angle = _ax->ToCSSValue();
    if (!angle)
    {
      return nullptr;
    }
    return FunctionValue::Create(ValueId::SkewX, Ref<Value>(krys::move(angle)));
  }
}
