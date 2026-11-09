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

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTranslate.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD bool IsValidTranslateXY(const CSSNumericValue &value) noexcept
    {
      return value.Type().MatchesTypeOrPercentage<CSSNumericBaseType::Length>();
    }

    KRYS_NODISCARD bool IsValidTranslateZ(const CSSNumericValue &value) noexcept
    {
      return value.Type().Matches<CSSNumericBaseType::Length>();
    }
  }

  dom::ExceptionOr<Ref<CSSTranslate>> CSSTranslate::Create(Ref<CSSNumericValue> x, Ref<CSSNumericValue> y,
                                                           RefPtr<CSSNumericValue> z) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstranslate-csstranslate
    bool is2D = !z;
    if (!z)
    {
      z = CSSNumericFactory::Px(0);
    }
    if (!IsValidTranslateXY(*x) || !IsValidTranslateXY(*y) || !IsValidTranslateZ(*z))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    Ref<CSSNumericValue> nonNullZ = krys::move(z);
    return AdoptRef(*new CSSTranslate(is2D, krys::move(x), krys::move(y), krys::move(nonNullZ)));
  }

  dom::ExceptionOr<Ref<CSSTranslate>> CSSTranslate::Create(const FunctionValue &function,
                                                           dom::Document &document) noexcept
  {
    auto reified = ReifyArguments(function, document);
    if (reified.HasException())
    {
      return reified.ReleaseException();
    }
    auto arguments = reified.ReleaseValue();
    auto count = arguments.size();
    switch (function.Name())
    {
      case ValueId::TranslateX:
      {
        if (count == 1)
        {
          return Create(krys::move(arguments[0]), CSSNumericFactory::Px(0));
        }
        break;
      }
      case ValueId::TranslateY:
      {
        if (count == 1)
        {
          return Create(CSSNumericFactory::Px(0), krys::move(arguments[0]));
        }
        break;
      }
      case ValueId::TranslateZ:
      {
        if (count == 1)
        {
          return Create(CSSNumericFactory::Px(0), CSSNumericFactory::Px(0), krys::move(arguments[0]));
        }
        break;
      }
      case ValueId::Translate:
      {
        if (count == 1)
        {
          return Create(krys::move(arguments[0]), CSSNumericFactory::Px(0));
        }
        if (count == 2)
        {
          return Create(krys::move(arguments[0]), krys::move(arguments[1]));
        }
        break;
      }
      case ValueId::Translate3d:
      {
        if (count == 3)
        {
          return Create(krys::move(arguments[0]), krys::move(arguments[1]), krys::move(arguments[2]));
        }
        break;
      }
      default:
      {
        break;
      }
    }
    return dom::Exception {dom::ExceptionCode::TypeError};
  }

  CSSTranslate::CSSTranslate(bool is2D, Ref<CSSNumericValue> x, Ref<CSSNumericValue> y,
                             Ref<CSSNumericValue> z) noexcept
      : CSSTransformComponent(is2D), _x(krys::move(x)), _y(krys::move(y)), _z(krys::move(z))
  {
  }

  dom::ExceptionOr<void> CSSTranslate::SetX(Ref<CSSNumericValue> x) noexcept
  {
    if (!IsValidTranslateXY(*x))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _x = krys::move(x);
    return {};
  }

  dom::ExceptionOr<void> CSSTranslate::SetY(Ref<CSSNumericValue> y) noexcept
  {
    if (!IsValidTranslateXY(*y))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _y = krys::move(y);
    return {};
  }

  dom::ExceptionOr<void> CSSTranslate::SetZ(Ref<CSSNumericValue> z) noexcept
  {
    if (!IsValidTranslateZ(*z))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _z = krys::move(z);
    return {};
  }

  void CSSTranslate::Serialise(StringBuilder &builder) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#serialize-a-csstranslate
    builder.Append(Is2D() ? u8"translate(" : u8"translate3d(");
    _x->Serialise(builder);
    builder.Append(u8", ");
    _y->Serialise(builder);
    if (!Is2D())
    {
      builder.Append(u8", ");
      _z->Serialise(builder);
    }
    builder.Append(u8')');
  }

  dom::ExceptionOr<Ref<DOMMatrix>> CSSTranslate::ToMatrix() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-tomatrix
    // The matrix's entries are relative to px, so a length that isn't compatible with px (a relative length
    // or a percentage) throws a TypeError.
    const auto *xUnitValue = DynamicDowncast<CSSUnitValue>(*_x);
    const auto *yUnitValue = DynamicDowncast<CSSUnitValue>(*_y);
    const auto *zUnitValue = DynamicDowncast<CSSUnitValue>(*_z);
    if (!xUnitValue || !yUnitValue || !zUnitValue)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    auto x = xUnitValue->ConvertTo(UnitType::px);
    auto y = yUnitValue->ConvertTo(UnitType::px);
    auto z = zUnitValue->ConvertTo(UnitType::px);
    if (!x || !y || !z)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    TransformationMatrix matrix;
    if (Is2D())
    {
      matrix.Translate(x->Value(), y->Value());
    }
    else
    {
      matrix.Translate3d(x->Value(), y->Value(), z->Value());
    }
    return DOMMatrix::Create(matrix, Is2D() ? DOMMatrixReadOnly::Is2D::Yes : DOMMatrixReadOnly::Is2D::No);
  }

  RefPtr<Value> CSSTranslate::ToCSSValue() const noexcept
  {
    auto x = _x->ToCSSValue();
    auto y = _y->ToCSSValue();
    if (!x || !y)
    {
      return nullptr;
    }
    if (Is2D())
    {
      return FunctionValue::Create(ValueId::Translate, Ref<Value>(krys::move(x)), Ref<Value>(krys::move(y)));
    }
    auto z = _z->ToCSSValue();
    if (!z)
    {
      return nullptr;
    }
    return FunctionValue::Create(ValueId::Translate3d, Ref<Value>(krys::move(x)), Ref<Value>(krys::move(y)),
                                 Ref<Value>(krys::move(z)));
  }
}
