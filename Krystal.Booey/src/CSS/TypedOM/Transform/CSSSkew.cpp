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

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSSkew.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"

namespace krys::boo::css
{
  namespace
  {
    /// @brief Whether @p ay is a unit value of zero, which `skew()` leaves out.
    KRYS_NODISCARD bool IsZeroSkewAngle(const CSSNumericValue &ay) noexcept
    {
      const auto *unitValue = DynamicDowncast<CSSUnitValue>(ay);
      return unitValue && !unitValue->Value();
    }
  }

  dom::ExceptionOr<Ref<CSSSkew>> CSSSkew::Create(Ref<CSSNumericValue> ax, Ref<CSSNumericValue> ay) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssskew-cssskew
    // 1. If ax or ay do not match <angle>, throw a TypeError.
    if (!ax->Type().Matches<CSSNumericBaseType::Angle>() || !ay->Type().Matches<CSSNumericBaseType::Angle>())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    // 2. Return a new CSSSkew object with its ax and ay internal slots set to ax and ay, and its is2D
    // internal
    //    slot set to true.
    return AdoptRef(*new CSSSkew(krys::move(ax), krys::move(ay)));
  }

  dom::ExceptionOr<Ref<CSSSkew>> CSSSkew::Create(const FunctionValue &function,
                                                 dom::Document &document) noexcept
  {
    auto reified = ReifyArguments(function, document);
    if (reified.HasException())
    {
      return reified.ReleaseException();
    }
    auto arguments = reified.ReleaseValue();
    if (function.Name() != ValueId::Skew || arguments.empty() || arguments.size() > 2)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    if (arguments.size() == 2)
    {
      return Create(krys::move(arguments[0]), krys::move(arguments[1]));
    }
    return Create(krys::move(arguments[0]), CSSNumericFactory::Deg(0));
  }

  CSSSkew::CSSSkew(Ref<CSSNumericValue> ax, Ref<CSSNumericValue> ay) noexcept
      : CSSTransformComponent(true), _ax(krys::move(ax)), _ay(krys::move(ay))
  {
  }

  dom::ExceptionOr<void> CSSSkew::SetAx(Ref<CSSNumericValue> ax) noexcept
  {
    if (!ax->Type().Matches<CSSNumericBaseType::Angle>())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _ax = krys::move(ax);
    return {};
  }

  dom::ExceptionOr<void> CSSSkew::SetAy(Ref<CSSNumericValue> ay) noexcept
  {
    if (!ay->Type().Matches<CSSNumericBaseType::Angle>())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _ay = krys::move(ay);
    return {};
  }

  void CSSSkew::Serialise(StringBuilder &builder) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#serialize-a-cssskew
    builder.Append(u8"skew(");
    _ax->Serialise(builder);
    if (!IsZeroSkewAngle(*_ay))
    {
      builder.Append(u8", ");
      _ay->Serialise(builder);
    }
    builder.Append(u8')');
  }

  dom::ExceptionOr<Ref<DOMMatrix>> CSSSkew::ToMatrix() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-tomatrix
    const auto *axUnitValue = DynamicDowncast<CSSUnitValue>(*_ax);
    const auto *ayUnitValue = DynamicDowncast<CSSUnitValue>(*_ay);
    if (!axUnitValue || !ayUnitValue)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    auto ax = axUnitValue->ConvertTo(UnitType::deg);
    auto ay = ayUnitValue->ConvertTo(UnitType::deg);
    if (!ax || !ay)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    TransformationMatrix matrix;
    matrix.Skew(ax->Value(), ay->Value());
    return DOMMatrix::Create(matrix, DOMMatrixReadOnly::Is2D::Yes);
  }

  RefPtr<Value> CSSSkew::ToCSSValue() const noexcept
  {
    auto ax = _ax->ToCSSValue();
    auto ay = _ay->ToCSSValue();
    if (!ax || !ay)
    {
      return nullptr;
    }
    if (IsZeroSkewAngle(*_ay))
    {
      return FunctionValue::Create(ValueId::Skew, Ref<Value>(krys::move(ax)));
    }
    return FunctionValue::Create(ValueId::Skew, Ref<Value>(krys::move(ax)), Ref<Value>(krys::move(ay)));
  }
}
