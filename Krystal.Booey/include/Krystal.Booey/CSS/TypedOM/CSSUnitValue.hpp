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

#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"

namespace krys::boo::css
{
  /// @brief A number with a unit (or none, or a percentage).
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssunitvalue
  class CSSUnitValue final : public CSSNumericValue
  {
  private:
    double _value;
    const UnitType _unit;

  public:
    /// @return A `TypeError` if @p unit isn't a unit.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssunitvalue-cssunitvalue
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSUnitValue>> Create(double value,
                                                                     const CSSOMString &unit) noexcept;
    KRYS_NODISCARD static Ref<CSSUnitValue> Create(double value, UnitType unit) noexcept
    {
      return AdoptRef(*new CSSUnitValue(value, unit));
    }

    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;

    KRYS_NODISCARD double Value() const noexcept
    {
      return _value;
    }
    void SetValue(double value) noexcept
    {
      _value = value;
    }
    /// @brief The unit as `unit` gives it: `number`, `percent`, or the unit's name.
    KRYS_NODISCARD CSSOMStringView Unit() const noexcept;
    /// @brief The unit as it's written after the number.
    KRYS_NODISCARD CSSOMStringView UnitSerialisation() const noexcept;
    KRYS_NODISCARD UnitType UnitEnum() const noexcept
    {
      return _unit;
    }

    /// @return Null if @p unit isn't compatible with this value's unit.
    /// @see https://drafts.css-houdini.org/css-typed-om/#convert-a-cssunitvalue
    KRYS_NODISCARD RefPtr<CSSUnitValue> ConvertTo(UnitType unit) const noexcept;
    /// @return `UnitType::Unknown` if @p unit isn't a unit.
    KRYS_NODISCARD static UnitType ParseUnit(const CSSOMString &unit) noexcept;

    KRYS_NODISCARD RefPtr<css::Value> ToCSSValue() const noexcept final;
    KRYS_NODISCARD RefPtr<css::Value> ToCSSValueWithProperty(PropertyId propertyId) const noexcept final;
    KRYS_NODISCARD Maybe<calc::Child> ToCalcTreeNode() const noexcept final;

  private:
    CSSUnitValue(double value, UnitType unit) noexcept;

    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSUnitValue;
    }
    KRYS_NODISCARD Maybe<SumValue> ToSumValue() const noexcept final;
    KRYS_NODISCARD bool Equals(const CSSNumericValue &other) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSUnitValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSUnitValue;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
