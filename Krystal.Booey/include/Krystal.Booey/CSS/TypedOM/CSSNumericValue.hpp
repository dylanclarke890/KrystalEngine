/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSNumericType.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css::calc
{
  struct Child;
  struct ChildOrNone;
  struct Tree;
}

namespace krys::boo::css
{
  class CSSNumericValue;
  class CSSUnitValue;
  class CSSMathSum;

  /// @brief A number, or a numeric value.
  /// @see https://drafts.css-houdini.org/css-typed-om/#typedefdef-cssnumberish
  using CSSNumberish = Variant<double, RefPtr<CSSNumericValue>>;

  /// @brief A numeric CSS value: a number, percentage or dimension, or a math expression of them.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssnumericvalue
  class CSSNumericValue : public CSSStyleValue
  {
  public:
    /// @see https://drafts.css-houdini.org/css-typed-om/#sum-value-value
    using UnitMap = HashMap<UnitType, int32>;
    struct Addend
    {
      double Value;
      UnitMap Units;
    };
    using SumValue = List<Addend>;

  protected:
    CSSNumericType _type;

  public:
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-add
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSNumericValue>> Add(List<CSSNumberish> &&values) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-sub
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSNumericValue>> Sub(List<CSSNumberish> &&values) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-mul
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSNumericValue>> Mul(List<CSSNumberish> &&values) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-div
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSNumericValue>> Div(List<CSSNumberish> &&values) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-min
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSNumericValue>> Min(List<CSSNumberish> &&values) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-max
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSNumericValue>> Max(List<CSSNumberish> &&values) noexcept;

    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-equals
    KRYS_NODISCARD bool Equals(List<CSSNumberish> &&values) noexcept;

    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-to
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSUnitValue>> To(const CSSOMString &unit) noexcept;
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSUnitValue>> To(UnitType unit) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-tosum
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSMathSum>> ToSum(List<CSSOMString> &&units) noexcept;

    KRYS_NODISCARD const CSSNumericType &Type() const noexcept
    {
      return _type;
    }

    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssnumericvalue-parse
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>> Parse(dom::Document &document,
                                                                       CSSOMString &&cssText) noexcept;
    /// @see https://drafts.css-houdini.org/css-typed-om/#rectify-a-numberish-value
    KRYS_NODISCARD static Ref<CSSNumericValue> RectifyNumberish(CSSNumberish &&numberish) noexcept;

    /// @see https://drafts.css-houdini.org/css-typed-om/#create-a-sum-value
    KRYS_NODISCARD virtual Maybe<SumValue> ToSumValue() const noexcept = 0;
    /// @see https://drafts.css-houdini.org/css-typed-om/#equal-numeric-value
    KRYS_NODISCARD virtual bool Equals(const CSSNumericValue &other) const noexcept = 0;

    KRYS_NODISCARD virtual Maybe<calc::Child> ToCalcTreeNode() const noexcept = 0;

    /// @see https://drafts.css-houdini.org/css-typed-om/#reify-a-math-expression
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
      ReifyMathExpression(const calc::Tree &tree) noexcept;
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
      ReifyMathExpression(const calc::Child &root) noexcept;
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSNumericValue>>
      ReifyMathExpression(const calc::ChildOrNone &root) noexcept;

  protected:
    explicit CSSNumericValue(CSSNumericType type = {}) noexcept : _type(krys::move(type))
    {
    }

    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSNumericValue>>
      AddInternal(List<Ref<CSSNumericValue>> &&values) noexcept;
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSNumericValue>>
      MultiplyInternal(List<Ref<CSSNumericValue>> &&values) noexcept;
    template <typename T>
    KRYS_NODISCARD List<Ref<CSSNumericValue>>
      PrependItemsOfTypeOrThis(List<Ref<CSSNumericValue>> &&values) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSNumericValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return krys::boo::css::IsCSSNumericValue(styleValue.StyleValueType());
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
