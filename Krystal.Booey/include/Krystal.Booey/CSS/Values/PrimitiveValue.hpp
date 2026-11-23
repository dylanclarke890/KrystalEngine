/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Strings/StringAtom.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css::calc
{
  class Value;
}

namespace krys::boo::css
{
  class ToLengthConversionData;

  class PrimitiveValue final : public Value
  {
    friend class StaticValuePool;
    friend class ValuePool;
    friend class Value;

    using PrimitiveKind = detail::PrimitiveKind;

  private:
    union ValueUnion
    {
      css::PropertyId PropertyId;
      css::ValueId ValueId;
      double Number;
      StringAtomStorage String;
      const calc::Value *Calc;
    } _value;

    struct CreateImplicitInitialValueTag
    {
    };

    constexpr static CreateImplicitInitialValueTag CreateImplicitInitialValue {};

#pragma region Constructors

    explicit PrimitiveValue(css::PropertyId property) noexcept;

    PrimitiveValue(const CSSOMString &value, PrimitiveKind kind) noexcept;

    PrimitiveValue(double value, UnitType unit) noexcept;

    explicit PrimitiveValue(Ref<calc::Value> value) noexcept;

    PrimitiveValue(StaticValueTag, css::ValueId keyword) noexcept;

    PrimitiveValue(StaticValueTag, double value, UnitType unit) noexcept;

    PrimitiveValue(StaticValueTag, CreateImplicitInitialValueTag) noexcept;

#pragma endregion

  public:
    ~PrimitiveValue() noexcept;

#pragma region Static Creation

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(double value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(double value, UnitType unit) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> CreateInteger(double value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(Ref<calc::Value> value) noexcept;

    /// @note Defined in ValuePool.hpp.
    KRYS_NODISCARD static inline Ref<PrimitiveValue> Create(css::ValueId identifier) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(css::PropertyId property) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(const CSSOMString &value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(const CSSOMStringAtom &value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> CreateCustomIdent(const CSSOMString &value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> CreateCustomIdent(const CSSOMStringAtom &value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> CreateFontFamily(const CSSOMString &value) noexcept;

    /// @note Defined in ValuePool.hpp.
    KRYS_NODISCARD static inline PrimitiveValue &ImplicitInitialValue() noexcept;

#pragma endregion

#pragma region Unit Checks

    // The quantity checks (IsLength, IsAngle, ...) see a calc() as the quantity it resolves to. The font
    // checks look at the lengths inside it, and IsPx at the written unit, which a calc() never has.

    KRYS_NODISCARD bool IsAngle() const noexcept
    {
      return CategoryForUnitType(Unit()) == Category::Angle;
    }

    /// @brief Whether this is an absolute length, which nothing but zoom affects; a calc() is if all its
    /// lengths are.
    KRYS_NODISCARD bool IsFontIndependentLength() const noexcept;

    /// @brief Whether this is a length relative to a font, the root's included; a calc() is if any of its
    /// lengths is.
    KRYS_NODISCARD bool IsFontRelativeLength() const noexcept;

    /// @brief Whether this depends on the parent's font size: a percentage, or a length relative to the
    /// element's own font; a calc() is if any of its parts is.
    KRYS_NODISCARD bool IsParentFontRelativeLength() const noexcept;

    /// @brief Whether this is a length relative to the root's font; a calc() is if any of its lengths is.
    KRYS_NODISCARD bool IsRootFontRelativeLength() const noexcept;

    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return css::IsLength(Unit());
    }

    KRYS_NODISCARD bool IsNumber() const noexcept
    {
      return Unit() == UnitType::Number;
    }

    KRYS_NODISCARD bool IsInteger() const noexcept
    {
      return Unit() == UnitType::Integer;
    }

    KRYS_NODISCARD bool IsNumberOrInteger() const noexcept
    {
      return IsNumber() || IsInteger();
    }

    KRYS_NODISCARD bool IsPercentage() const noexcept
    {
      return Unit() == UnitType::Percentage;
    }

    /// @brief Whether this is a number written in `px`; never a calc().
    KRYS_NODISCARD bool IsPx() const noexcept
    {
      return StoredUnit() == UnitType::px;
    }

    KRYS_NODISCARD bool IsTime() const noexcept
    {
      return CategoryForUnitType(Unit()) == Category::Time;
    }

    KRYS_NODISCARD bool IsCalc() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::Calc;
    }

    KRYS_NODISCARD bool IsCalcPercentageWithLength() const noexcept;

    KRYS_NODISCARD bool IsResolution() const noexcept
    {
      return CategoryForUnitType(Unit()) == Category::Resolution;
    }

    KRYS_NODISCARD bool IsFlex() const noexcept
    {
      return Unit() == UnitType::fr;
    }

#pragma endregion

#pragma region Keywords, Properties and Strings

    KRYS_NODISCARD bool IsValueId() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::ValueId;
    }

    KRYS_NODISCARD css::ValueId ValueId() const noexcept
    {
      return IsValueId() ? _value.ValueId : css::ValueId::Invalid;
    }

    KRYS_NODISCARD bool IsPropertyId() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::PropertyId;
    }

    KRYS_NODISCARD css::PropertyId PropertyId() const noexcept
    {
      return IsPropertyId() ? _value.PropertyId : css::PropertyId::Invalid;
    }

    KRYS_NODISCARD bool IsString() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::String;
    }

    KRYS_NODISCARD bool IsCustomIdent() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::CustomIdent;
    }

    KRYS_NODISCARD bool IsFontFamily() const noexcept
    {
      return _header.PrimitiveKind == PrimitiveKind::FontFamily;
    }

    KRYS_NODISCARD CSSOMString StringValue() const noexcept;

#pragma endregion

    /// @brief The unit of a number, or of the number a calc() resolves to; `UnitType::Unknown` for the other
    /// kinds of value, and for a calc() mixing a percentage with a length or angle.
    KRYS_NODISCARD UnitType Unit() const noexcept;

#pragma region Integer (requires IsInteger())

    template <typename T = int64>
    KRYS_NODISCARD T ResolveAsInteger(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsInteger());
      return Resolve<T>(conversionData);
    }

    template <typename T = int64>
    KRYS_NODISCARD Maybe<T> ResolveAsIntegerIfNotCalc() const noexcept
    {
      krys_debug_assert(IsInteger());
      return ValueIfNotCalc<T>();
    }

#pragma endregion

#pragma region Number (requires IsNumberOrInteger())

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsNumber(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsNumberOrInteger());
      return Resolve<T>(UnitType::Number, conversionData);
    }

    template <typename T = double>
    KRYS_NODISCARD Maybe<T> ResolveAsNumberIfNotCalc() const noexcept
    {
      krys_debug_assert(IsNumberOrInteger());
      return ValueIfNotCalc<T>();
    }

#pragma endregion

#pragma region Percentage (requires IsPercentage())

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsPercentage(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsPercentage());
      return Resolve<T>(conversionData);
    }

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsPercentageWithoutConversionData() const noexcept
    {
      krys_debug_assert(IsPercentage());
      return ResolveWithoutConversionData<T>();
    }

#pragma endregion

#pragma region Angle (requires IsAngle())

    template <typename T = double, AngleUnit TUnit = UnitTraits<AngleUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsAngle(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsAngle());
      return ClampTo<T>(ConvertUnit(ResolveDouble(conversionData), Unit(), ToUnitType(TUnit)).value_or(0));
    }

#pragma endregion

#pragma region Time (requires IsTime())

    template <typename T = double, TimeUnit TUnit = UnitTraits<TimeUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsTime(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsTime());
      return ClampTo<T>(ConvertUnit(ResolveDouble(conversionData), Unit(), ToUnitType(TUnit)).value_or(0));
    }

#pragma endregion

#pragma region Resolution (requires IsResolution())

    template <typename T = double, ResolutionUnit TUnit = UnitTraits<ResolutionUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsResolution(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsResolution());
      return ClampTo<T>(ConvertUnit(ResolveDouble(conversionData), Unit(), ToUnitType(TUnit)).value_or(0));
    }

#pragma endregion

#pragma region Flex (requires IsFlex())

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsFlex(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsFlex());
      return Resolve<T>(conversionData);
    }

#pragma endregion

#pragma region Length (requires IsLength())

    /// @note Specialised for int64, uint64, int32, uint32, int16, uint16, float, double and LayoutUnit in
    /// PrimitiveValue.cpp.
    template <typename T = double>
    KRYS_NODISCARD T ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept;

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsLengthWithoutConversionData() const noexcept
    {
      krys_debug_assert(IsLength());
      return ResolveWithoutConversionData<T>(UnitType::px);
    }

#pragma endregion

#pragma region Non-converting

    template <typename T = double>
    KRYS_NODISCARD T Resolve(const ToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(ResolveDouble(conversionData));
    }

    template <typename T = double>
    KRYS_NODISCARD T ResolveWithoutConversionData() const noexcept
    {
      return ClampTo<T>(ResolveDoubleWithoutConversionData());
    }

#pragma endregion

#pragma region Divides value by 100 if percentage

    template <typename T = double>
    KRYS_NODISCARD
    T ResolveDividingBy100IfPercentage(const ToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(ResolveDoubleDividingBy100IfPercentage(conversionData));
    }

#pragma endregion

    // These return null for calc, for which range checking is not done at parse time:
    // <https://www.w3.org/TR/css3-values/#calc-range>.
    KRYS_NODISCARD Maybe<bool> IsZero() const noexcept;

    KRYS_NODISCARD Maybe<bool> IsOne() const noexcept;

    KRYS_NODISCARD Maybe<bool> IsPositive() const noexcept;

    KRYS_NODISCARD Maybe<bool> IsNegative() const noexcept;

    KRYS_NODISCARD const calc::Value *CalcValue() const noexcept
    {
      return IsCalc() ? _value.Calc : nullptr;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const PrimitiveValue &other) const noexcept;

    void CollectComputedStyleDependencies(css::ComputedStyleDependencies &dependencies) const noexcept;

  private:
    // The stored unit: `Unknown` for every kind but a number, calc() included.
    KRYS_NODISCARD UnitType StoredUnit() const noexcept
    {
      return _header.Unit;
    }

#pragma region Length converting

    KRYS_NODISCARD double ResolveAsLengthDouble(const ToLengthConversionData &conversionData) const noexcept;

#pragma endregion

#pragma region Arbitrarily converting

    KRYS_NODISCARD double ResolveDouble(UnitType targetUnit, const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD double ResolveDoubleWithoutConversionData(UnitType targetUnit) const noexcept;

    template <typename T = double>
    KRYS_NODISCARD T Resolve(UnitType targetUnit, const ToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(ResolveDouble(targetUnit, conversionData));
    }

    template <typename T = double>
    KRYS_NODISCARD T ResolveWithoutConversionData(UnitType targetUnit) const noexcept
    {
      return ClampTo<T>(ResolveDoubleWithoutConversionData(targetUnit));
    }

#pragma endregion

#pragma region Non-converting

    KRYS_NODISCARD double ResolveDouble(const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD double ResolveDoubleWithoutConversionData() const noexcept
    {
      krys_debug_assert(!IsCalc());
      return _value.Number;
    }

    KRYS_NODISCARD double
      ResolveDoubleDividingBy100IfPercentage(const ToLengthConversionData &) const noexcept;

    template <typename T = double>
    KRYS_NODISCARD Maybe<T> ValueIfNotCalc() const noexcept
    {
      if (IsCalc())
      {
        return null;
      }

      return static_cast<T>(_value.Number);
    }

#pragma endregion

    KRYS_NODISCARD bool AddDerivedHash(size_t &seed) const noexcept;

    KRYS_NODISCARD CSSOMString SerialiseInternal(const SerialisationContext &) const noexcept;

    KRYS_NODISCARD CSSOMString FormatNumberValue(CSSOMStringView suffix) const noexcept;

    KRYS_NODISCARD CSSOMString FormatIntegerValue(CSSOMStringView suffix) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PrimitiveValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.Type() == krys::boo::css::ValueType::Primitive;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

namespace krys::boo::css
{
#pragma region Value inline definitions

  inline bool Value::IsValueId() const noexcept
  {
    auto *value = DynamicDowncast<PrimitiveValue>(*this);
    return value && value->IsValueId();
  }

  inline css::ValueId Value::ValueId() const noexcept
  {
    auto *value = DynamicDowncast<PrimitiveValue>(*this);
    return value ? value->ValueId() : css::ValueId::Invalid;
  }

  inline bool Value::IsCustomIdent() const noexcept
  {
    auto *value = DynamicDowncast<PrimitiveValue>(*this);
    return value && value->IsCustomIdent();
  }

#pragma endregion
}
