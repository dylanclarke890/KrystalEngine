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

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSPerspective.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSStyleValueFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::css
{
  namespace
  {
    /// @brief @p length if it's a length or `none`, with a string as a keyword value.
    KRYS_NODISCARD dom::ExceptionOr<CSSPerspectiveValue>
      CheckPerspectiveLength(CSSPerspectiveValue length) noexcept
    {
      // https://drafts.css-houdini.org/css-typed-om/#dom-cssperspective-cssperspective
      auto checkKeyword = [](Ref<CSSKeywordValue> keyword) -> dom::ExceptionOr<CSSPerspectiveValue>
      {
        // Otherwise (that is, if length is not a CSSNumericValue), rectify a keywordish value from length,
        // then if the result's value is an ASCII case-insensitive match for the keyword none, set length to
        // the result. Otherwise, throw a TypeError.
        if (!infra::StringAlgorithms::ASCIICaseInsensitiveMatch(keyword->Value(), u8"none"))
        {
          return dom::Exception {dom::ExceptionCode::TypeError};
        }
        return CSSPerspectiveValue {RefPtr<CSSKeywordValue>(krys::move(keyword))};
      };
      return krys::SwitchOn(
        krys::move(length),
        [](RefPtr<CSSNumericValue> &&value) -> dom::ExceptionOr<CSSPerspectiveValue>
        {
          // If length is a CSSNumericValue: if length does not match <length>, throw a TypeError.
          if (!value || !value->Type().Matches<CSSNumericBaseType::Length>())
          {
            return dom::Exception {dom::ExceptionCode::TypeError};
          }
          return CSSPerspectiveValue {krys::move(value)};
        },
        [&](CSSOMString &&value)
        { return checkKeyword(CSSKeywordValue::RectifyKeywordish(krys::move(value))); },
        [&](RefPtr<CSSKeywordValue> &&value) -> dom::ExceptionOr<CSSPerspectiveValue>
        {
          if (!value)
          {
            return dom::Exception {dom::ExceptionCode::TypeError};
          }
          return checkKeyword(Ref<CSSKeywordValue>(krys::move(value)));
        });
    }
  }

  dom::ExceptionOr<Ref<CSSPerspective>> CSSPerspective::Create(CSSPerspectiveValue length) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssperspective-cssperspective
    auto checkedLength = CheckPerspectiveLength(krys::move(length));
    if (checkedLength.HasException())
    {
      return checkedLength.ReleaseException();
    }
    // Return a new CSSPerspective object with its length internal slot set to length, and its is2D internal
    // slot set to false.
    return AdoptRef(*new CSSPerspective(checkedLength.ReleaseValue()));
  }

  dom::ExceptionOr<Ref<CSSPerspective>> CSSPerspective::Create(const FunctionValue &function,
                                                               dom::Document &document) noexcept
  {
    if (function.Name() != ValueId::Perspective || function.Length() != 1)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    auto reified = CSSStyleValueFactory::ReifyValue(document, function[0], {});
    if (reified.HasException())
    {
      return reified.ReleaseException();
    }
    auto value = reified.ReleaseValue();
    if (auto *keyword = DynamicDowncast<CSSKeywordValue>(*value))
    {
      return Create(ShareRefPtr(keyword));
    }
    if (auto *numeric = DynamicDowncast<CSSNumericValue>(*value))
    {
      return Create(ShareRefPtr(numeric));
    }
    return dom::Exception {dom::ExceptionCode::TypeError};
  }

  CSSPerspective::CSSPerspective(CSSPerspectiveValue length) noexcept
      : CSSTransformComponent(false), _length(krys::move(length))
  {
  }

  CSSPerspective::~CSSPerspective() noexcept = default;

  dom::ExceptionOr<void> CSSPerspective::SetLength(CSSPerspectiveValue length) noexcept
  {
    auto checkedLength = CheckPerspectiveLength(krys::move(length));
    if (checkedLength.HasException())
    {
      return checkedLength.ReleaseException();
    }
    _length = checkedLength.ReleaseValue();
    return {};
  }

  void CSSPerspective::Serialise(StringBuilder &builder) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#serialize-a-cssperspective
    builder.Append(u8"perspective(");
    krys::SwitchOn(
      _length,
      [&](const RefPtr<CSSNumericValue> &value)
      {
        // A negative length is serialised in a calc(), as perspective() doesn't accept one.
        const auto *unitValue = DynamicDowncast<CSSUnitValue>(value.get());
        if (unitValue && unitValue->Value() < 0.0)
        {
          builder.Append(u8"calc(");
          value->Serialise(builder);
          builder.Append(u8')');
          return;
        }
        value->Serialise(builder);
      },
      [&](const CSSOMString &value) { builder.Append(value); },
      [&](const RefPtr<CSSKeywordValue> &value) { builder.Append(value->ToString()); });
    builder.Append(u8')');
  }

  dom::ExceptionOr<Ref<DOMMatrix>> CSSPerspective::ToMatrix() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-tomatrix
    // `perspective(none)` is the identity.
    const auto *numeric = std::get_if<RefPtr<CSSNumericValue>>(&_length);
    if (!numeric)
    {
      return DOMMatrix::Create(TransformationMatrix(), DOMMatrixReadOnly::Is2D::Yes);
    }
    const auto *unitValue = DynamicDowncast<CSSUnitValue>(numeric->get());
    if (!unitValue)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    auto length = unitValue->ConvertTo(UnitType::px);
    if (!length)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    TransformationMatrix matrix;
    matrix.ApplyPerspective(length->Value());
    return DOMMatrix::Create(matrix, DOMMatrixReadOnly::Is2D::No);
  }

  RefPtr<Value> CSSPerspective::ToCSSValue() const noexcept
  {
    auto length = krys::SwitchOn(
      _length, [](const RefPtr<CSSNumericValue> &value) { return value->ToCSSValue(); },
      [](const CSSOMString &) { return RefPtr<Value>(); },
      [](const RefPtr<CSSKeywordValue> &value) { return value->ToCSSValue(); });
    if (!length)
    {
      return nullptr;
    }
    return FunctionValue::Create(ValueId::Perspective, Ref<Value>(krys::move(length)));
  }
}
