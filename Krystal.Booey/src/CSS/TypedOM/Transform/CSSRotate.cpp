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

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSRotate.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD bool IsValidRotateAngle(const CSSNumericValue &value) noexcept
    {
      return value.Type().Matches<CSSNumericBaseType::Angle>();
    }
  }

  dom::ExceptionOr<Ref<CSSRotate>> CSSRotate::Create(CSSNumberish x, CSSNumberish y, CSSNumberish z,
                                                     Ref<CSSNumericValue> angle) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssrotate-cssrotate-x-y-z-angle
    // 1. Let x, y, and z be replaced by the result of rectifying a numberish value.
    auto rectifiedX = CSSNumericValue::RectifyNumberish(krys::move(x));
    auto rectifiedY = CSSNumericValue::RectifyNumberish(krys::move(y));
    auto rectifiedZ = CSSNumericValue::RectifyNumberish(krys::move(z));
    // 2. If angle doesn't match <angle>, throw a TypeError.
    // 3. If any of x, y, or z don't match <number>, throw a TypeError.
    if (!IsValidRotateAngle(*angle) || !rectifiedX->Type().MatchesNumber()
        || !rectifiedY->Type().MatchesNumber() || !rectifiedZ->Type().MatchesNumber())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    // 4. Return a new CSSRotate with its x, y, z, and angle internal slots set to x, y, z, and angle, and its
    //    is2D internal slot set to false.
    return AdoptRef(*new CSSRotate(false, krys::move(rectifiedX), krys::move(rectifiedY),
                                   krys::move(rectifiedZ), krys::move(angle)));
  }

  dom::ExceptionOr<Ref<CSSRotate>> CSSRotate::Create(Ref<CSSNumericValue> angle) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssrotate-cssrotate-angle
    // 1. If angle doesn't match <angle>, throw a TypeError.
    if (!IsValidRotateAngle(*angle))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    // 2. Return a new CSSRotate with its angle internal slot set to angle, its x and y internal slots set to
    // new
    //    unit values of (0, "number"), its z internal slot set to a new unit value of (1, "number"), and its
    //    is2D internal slot set to true.
    return AdoptRef(*new CSSRotate(true, CSSNumericFactory::Number(0), CSSNumericFactory::Number(0),
                                   CSSNumericFactory::Number(1), krys::move(angle)));
  }

  dom::ExceptionOr<Ref<CSSRotate>> CSSRotate::Create(const FunctionValue &function,
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
      case ValueId::RotateX:
      {
        if (count == 1)
        {
          return Create(1.0, 0.0, 0.0, krys::move(arguments[0]));
        }
        break;
      }
      case ValueId::RotateY:
      {
        if (count == 1)
        {
          return Create(0.0, 1.0, 0.0, krys::move(arguments[0]));
        }
        break;
      }
      case ValueId::RotateZ:
      {
        if (count == 1)
        {
          return Create(0.0, 0.0, 1.0, krys::move(arguments[0]));
        }
        break;
      }
      case ValueId::Rotate:
      {
        if (count == 1)
        {
          return Create(krys::move(arguments[0]));
        }
        break;
      }
      case ValueId::Rotate3d:
      {
        if (count == 4)
        {
          return Create(RefPtr<CSSNumericValue>(krys::move(arguments[0])),
                        RefPtr<CSSNumericValue>(krys::move(arguments[1])),
                        RefPtr<CSSNumericValue>(krys::move(arguments[2])), krys::move(arguments[3]));
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

  CSSRotate::CSSRotate(bool is2D, Ref<CSSNumericValue> x, Ref<CSSNumericValue> y, Ref<CSSNumericValue> z,
                       Ref<CSSNumericValue> angle) noexcept
      : CSSTransformComponent(is2D), _x(krys::move(x)), _y(krys::move(y)), _z(krys::move(z)),
        _angle(krys::move(angle))
  {
  }

  dom::ExceptionOr<void> CSSRotate::SetX(CSSNumberish x) noexcept
  {
    auto rectified = CSSNumericValue::RectifyNumberish(krys::move(x));
    if (!rectified->Type().MatchesNumber())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _x = krys::move(rectified);
    return {};
  }

  dom::ExceptionOr<void> CSSRotate::SetY(CSSNumberish y) noexcept
  {
    auto rectified = CSSNumericValue::RectifyNumberish(krys::move(y));
    if (!rectified->Type().MatchesNumber())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _y = krys::move(rectified);
    return {};
  }

  dom::ExceptionOr<void> CSSRotate::SetZ(CSSNumberish z) noexcept
  {
    auto rectified = CSSNumericValue::RectifyNumberish(krys::move(z));
    if (!rectified->Type().MatchesNumber())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _z = krys::move(rectified);
    return {};
  }

  dom::ExceptionOr<void> CSSRotate::SetAngle(Ref<CSSNumericValue> angle) noexcept
  {
    if (!IsValidRotateAngle(*angle))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    _angle = krys::move(angle);
    return {};
  }

  void CSSRotate::Serialise(StringBuilder &builder) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#serialize-a-cssrotate
    builder.Append(Is2D() ? u8"rotate(" : u8"rotate3d(");
    if (!Is2D())
    {
      _x->Serialise(builder);
      builder.Append(u8", ");
      _y->Serialise(builder);
      builder.Append(u8", ");
      _z->Serialise(builder);
      builder.Append(u8", ");
    }
    _angle->Serialise(builder);
    builder.Append(u8')');
  }

  dom::ExceptionOr<Ref<DOMMatrix>> CSSRotate::ToMatrix() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-tomatrix
    const auto *angleUnitValue = DynamicDowncast<CSSUnitValue>(*_angle);
    const auto *xUnitValue = DynamicDowncast<CSSUnitValue>(*_x);
    const auto *yUnitValue = DynamicDowncast<CSSUnitValue>(*_y);
    const auto *zUnitValue = DynamicDowncast<CSSUnitValue>(*_z);
    if (!angleUnitValue || !xUnitValue || !yUnitValue || !zUnitValue)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    auto angle = angleUnitValue->ConvertTo(UnitType::deg);
    if (!angle)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    TransformationMatrix matrix;
    if (Is2D())
    {
      matrix.Rotate(angle->Value());
    }
    else
    {
      matrix.Rotate3d(xUnitValue->Value(), yUnitValue->Value(), zUnitValue->Value(), angle->Value());
    }
    return DOMMatrix::Create(matrix, Is2D() ? DOMMatrixReadOnly::Is2D::Yes : DOMMatrixReadOnly::Is2D::No);
  }

  RefPtr<Value> CSSRotate::ToCSSValue() const noexcept
  {
    auto angle = _angle->ToCSSValue();
    if (!angle)
    {
      return nullptr;
    }
    if (Is2D())
    {
      return FunctionValue::Create(ValueId::Rotate, Ref<Value>(krys::move(angle)));
    }
    auto x = _x->ToCSSValue();
    auto y = _y->ToCSSValue();
    auto z = _z->ToCSSValue();
    if (!x || !y || !z)
    {
      return nullptr;
    }
    ValueListBuilder arguments;
    arguments.push_back(Ref<Value>(krys::move(x)));
    arguments.push_back(Ref<Value>(krys::move(y)));
    arguments.push_back(Ref<Value>(krys::move(z)));
    arguments.push_back(Ref<Value>(krys::move(angle)));
    return FunctionValue::Create(ValueId::Rotate3d, krys::move(arguments));
  }
}
