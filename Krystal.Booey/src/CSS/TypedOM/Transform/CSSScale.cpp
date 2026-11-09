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

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSScale.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSScale>> CSSScale::Create(CSSNumberish x, CSSNumberish y,
                                                   Maybe<CSSNumberish> z) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssscale-cssscale
    // 1. Let x, y, and z (if passed) be replaced by the result of rectifying a numberish value.
    bool is2D = !z;
    auto rectifiedX = CSSNumericValue::RectifyNumberish(krys::move(x));
    auto rectifiedY = CSSNumericValue::RectifyNumberish(krys::move(y));
    Ref<CSSNumericValue> rectifiedZ = z ? CSSNumericValue::RectifyNumberish(krys::move(*z))
                                        : Ref<CSSNumericValue>(CSSNumericFactory::Number(1));
    // 2. If x, y, or z (if passed) don't match <number>, throw a TypeError.
    if (!rectifiedX->Type().MatchesNumber() || !rectifiedY->Type().MatchesNumber()
        || !rectifiedZ->Type().MatchesNumber())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    // 3. Let this be a new CSSScale object, with its x and y internal slots set to x and y.
    // 4. If z was passed, set this's z internal slot to z, and set this's is2D internal slot to false.
    // 5. If z was not passed, set this's z internal slot to a new unit value of (1, "number"), and set this's
    //    is2D internal slot to true.
    // 6. Return this.
    return AdoptRef(
      *new CSSScale(is2D, krys::move(rectifiedX), krys::move(rectifiedY), krys::move(rectifiedZ)));
  }

  dom::ExceptionOr<Ref<CSSScale>> CSSScale::Create(const FunctionValue &function,
                                                   dom::Document &document) noexcept
  {
    auto reified = ReifyArguments(function, document);
    if (reified.HasException())
    {
      return reified.ReleaseException();
    }
    auto arguments = reified.ReleaseValue();
    auto count = arguments.size();
    auto numberish = [&](size_t index)
    {
      return CSSNumberish {RefPtr<CSSNumericValue>(arguments[index])};
    };
    switch (function.Name())
    {
      case ValueId::ScaleX:
      {
        if (count == 1)
        {
          return Create(numberish(0), 1.0);
        }
        break;
      }
      case ValueId::ScaleY:
      {
        if (count == 1)
        {
          return Create(1.0, numberish(0));
        }
        break;
      }
      case ValueId::ScaleZ:
      {
        if (count == 1)
        {
          return Create(1.0, 1.0, numberish(0));
        }
        break;
      }
      case ValueId::Scale:
      {
        if (count == 1 || count == 2)
        {
          return Create(numberish(0), numberish(count - 1));
        }
        break;
      }
      case ValueId::Scale3d:
      {
        if (count == 3)
        {
          return Create(numberish(0), numberish(1), numberish(2));
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

  CSSScale::CSSScale(bool is2D, Ref<CSSNumericValue> x, Ref<CSSNumericValue> y,
                     Ref<CSSNumericValue> z) noexcept
      : CSSTransformComponent(is2D), _x(krys::move(x)), _y(krys::move(y)), _z(krys::move(z))
  {
  }

  dom::ExceptionOr<void> CSSScale::SetX(CSSNumberish x) noexcept
  {
    auto rectified = CSSNumericValue::RectifyNumberish(krys::move(x));
    if (!rectified->Type().MatchesNumber())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _x = krys::move(rectified);
    return {};
  }

  dom::ExceptionOr<void> CSSScale::SetY(CSSNumberish y) noexcept
  {
    auto rectified = CSSNumericValue::RectifyNumberish(krys::move(y));
    if (!rectified->Type().MatchesNumber())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _y = krys::move(rectified);
    return {};
  }

  dom::ExceptionOr<void> CSSScale::SetZ(CSSNumberish z) noexcept
  {
    auto rectified = CSSNumericValue::RectifyNumberish(krys::move(z));
    if (!rectified->Type().MatchesNumber())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _z = krys::move(rectified);
    return {};
  }

  void CSSScale::Serialise(StringBuilder &builder) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#serialize-a-cssscale
    builder.Append(Is2D() ? u8"scale(" : u8"scale3d(");
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

  dom::ExceptionOr<Ref<DOMMatrix>> CSSScale::ToMatrix() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-tomatrix
    const auto *xUnitValue = DynamicDowncast<CSSUnitValue>(*_x);
    const auto *yUnitValue = DynamicDowncast<CSSUnitValue>(*_y);
    const auto *zUnitValue = DynamicDowncast<CSSUnitValue>(*_z);
    if (!xUnitValue || !yUnitValue || !zUnitValue)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    TransformationMatrix matrix;
    if (Is2D())
    {
      matrix.ScaleNonUniform(xUnitValue->Value(), yUnitValue->Value());
    }
    else
    {
      matrix.Scale3d(xUnitValue->Value(), yUnitValue->Value(), zUnitValue->Value());
    }
    return DOMMatrix::Create(matrix, Is2D() ? DOMMatrixReadOnly::Is2D::Yes : DOMMatrixReadOnly::Is2D::No);
  }

  RefPtr<Value> CSSScale::ToCSSValue() const noexcept
  {
    auto x = _x->ToCSSValue();
    auto y = _y->ToCSSValue();
    if (!x || !y)
    {
      return nullptr;
    }
    if (Is2D())
    {
      return FunctionValue::Create(ValueId::Scale, Ref<Value>(krys::move(x)), Ref<Value>(krys::move(y)));
    }
    auto z = _z->ToCSSValue();
    if (!z)
    {
      return nullptr;
    }
    return FunctionValue::Create(ValueId::Scale3d, Ref<Value>(krys::move(x)), Ref<Value>(krys::move(y)),
                                 Ref<Value>(krys::move(z)));
  }
}
