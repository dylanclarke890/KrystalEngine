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
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Strings/StringAtom.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <cmath>
#include <limits>

namespace krys::boo::css::calc
{
  class Value;
}

namespace krys::boo::css
{
  class ToLengthConversionData;

  // Max/min values for CSS, needs to slightly smaller/larger than the true max/min values to allow for
  // rounding without overflowing. Subtract two (rather than one) to allow for values to be converted to float
  // and back without exceeding the LayoutUnit::max.
  constexpr float MaxValueForCssLength = static_cast<float>(IntMaxForLayoutUnit - 2);
  constexpr float MinValueForCssLength = static_cast<float>(IntMinForLayoutUnit + 2);

  /// @brief Dimension calculations are imprecise, often resulting in values of e.g. 44.99998. We need to
  /// round if we're really close to the next integer value.
  template <typename T>
  KRYS_NODISCARD inline T RoundForImpreciseConversion(double value) noexcept
  {
    value += (value < 0) ? -0.01 : +0.01;
    return ((value > std::numeric_limits<T>::max()) || (value < std::numeric_limits<T>::min()))
             ? 0
             : static_cast<T>(value);
  }

  template <>
  KRYS_NODISCARD inline float RoundForImpreciseConversion(double value) noexcept
  {
    double ceiledValue = std::ceil(value);
    double proximityToNextInt = ceiledValue - value;
    if (proximityToNextInt <= 0.01 && value > 0)
    {
      return static_cast<float>(ceiledValue);
    }
    if (proximityToNextInt >= 0.99 && value < 0)
    {
      return static_cast<float>(std::floor(value));
    }
    return static_cast<float>(value);
  }

  class PrimitiveValue final : public Value
  {
    friend class StaticValuePool;
    friend class ValuePool;
    friend LazyNeverDestroyed<PrimitiveValue>;
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

    // FIXME: Some of these use PrimitiveUnitType() and some use PrimitiveType(). Many that use
    // PrimitiveUnitType() are likely broken with calc().

    KRYS_NODISCARD bool IsAngle() const noexcept
    {
      return UnitCategoryForType(PrimitiveType()) == UnitCategory::Angle;
    }

    KRYS_NODISCARD bool IsFontIndependentLength() const noexcept
    {
      return css::IsFontIndependentLength(PrimitiveUnitType());
    }

    KRYS_NODISCARD bool IsFontRelativeLength() const noexcept
    {
      return css::IsFontRelativeLength(PrimitiveUnitType());
    }

    KRYS_NODISCARD bool IsParentFontRelativeLength() const noexcept
    {
      return IsPercentage() || (IsFontRelativeLength() && !IsRootFontRelativeLength());
    }

    KRYS_NODISCARD bool IsRootFontRelativeLength() const noexcept
    {
      return css::IsRootFontRelativeLength(PrimitiveUnitType());
    }

    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return css::IsLength(PrimitiveType());
    }

    KRYS_NODISCARD bool IsNumber() const noexcept
    {
      return PrimitiveType() == UnitType::Number;
    }

    KRYS_NODISCARD bool IsInteger() const noexcept
    {
      return PrimitiveType() == UnitType::Integer;
    }

    KRYS_NODISCARD bool IsNumberOrInteger() const noexcept
    {
      return IsNumber() || IsInteger();
    }

    KRYS_NODISCARD bool IsPercentage() const noexcept
    {
      return PrimitiveType() == UnitType::Percentage;
    }

    KRYS_NODISCARD bool IsPx() const noexcept
    {
      return PrimitiveType() == UnitType::px;
    }

    KRYS_NODISCARD bool IsTime() const noexcept
    {
      return UnitCategoryForType(PrimitiveType()) == UnitCategory::Time;
    }

    KRYS_NODISCARD bool IsCalculated() const noexcept
    {
      return _primitiveKind == PrimitiveKind::Calc;
    }

    KRYS_NODISCARD bool IsCalculatedPercentageWithLength() const noexcept;

    KRYS_NODISCARD bool IsResolution() const noexcept
    {
      return UnitCategoryForType(PrimitiveType()) == UnitCategory::Resolution;
    }

    KRYS_NODISCARD bool IsViewportPercentageLength() const noexcept
    {
      return css::IsViewportPercentageLength(PrimitiveUnitType());
    }

    KRYS_NODISCARD bool IsContainerPercentageLength() const noexcept
    {
      return css::IsContainerPercentageLength(PrimitiveUnitType());
    }

    KRYS_NODISCARD bool IsFlex() const noexcept
    {
      return PrimitiveType() == UnitType::fr;
    }

    KRYS_NODISCARD bool ConversionToCanonicalUnitRequiresConversionData() const noexcept;

#pragma endregion

#pragma region Keywords, Properties and Strings

    KRYS_NODISCARD bool IsValueId() const noexcept
    {
      return _primitiveKind == PrimitiveKind::ValueId;
    }

    KRYS_NODISCARD css::ValueId ValueId() const noexcept
    {
      return IsValueId() ? _value.ValueId : css::ValueId::Invalid;
    }

    KRYS_NODISCARD bool IsPropertyId() const noexcept
    {
      return _primitiveKind == PrimitiveKind::PropertyId;
    }

    KRYS_NODISCARD css::PropertyId PropertyId() const noexcept
    {
      return IsPropertyId() ? _value.PropertyId : css::PropertyId::Invalid;
    }

    KRYS_NODISCARD bool IsString() const noexcept
    {
      return _primitiveKind == PrimitiveKind::String;
    }

    KRYS_NODISCARD bool IsCustomIdent() const noexcept
    {
      return _primitiveKind == PrimitiveKind::CustomIdent;
    }

    KRYS_NODISCARD CSSOMString CustomIdent() const noexcept
    {
      krys_debug_assert(IsCustomIdent());
      return StringValue();
    }

    KRYS_NODISCARD bool IsFontFamily() const noexcept
    {
      return _primitiveKind == PrimitiveKind::FontFamily;
    }

    KRYS_NODISCARD CSSOMString StringValue() const noexcept;

#pragma endregion

    /// @brief The unit of a number, or of the number a calc() resolves to; `UnitType::Unknown` for the other
    /// kinds of value, and for a calc() mixing a percentage with a length or angle.
    KRYS_NODISCARD UnitType PrimitiveType() const noexcept;

#pragma region Integer (requires IsInteger())

    template <typename T = int64>
    KRYS_NODISCARD T ResolveAsInteger(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsInteger());
      return Value<T>(conversionData);
    }

    template <typename T = int64>
    KRYS_NODISCARD Maybe<T> ResolveAsIntegerIfNotCalculated() const noexcept
    {
      krys_debug_assert(IsInteger());
      return ValueIfNotCalculated<T>();
    }

#pragma endregion

#pragma region Number (requires IsNumberOrInteger())

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsNumber(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsNumberOrInteger());
      return Value<T>(UnitType::Number, conversionData);
    }

    template <typename T = double>
    KRYS_NODISCARD Maybe<T> ResolveAsNumberIfNotCalculated() const noexcept
    {
      krys_debug_assert(IsNumberOrInteger());
      return ValueIfNotCalculated<T>();
    }

#pragma endregion

#pragma region Percentage (requires IsPercentage())

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsPercentage(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsPercentage());
      return Value<T>(conversionData);
    }

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsPercentageNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(IsPercentage());
      return ValueNoConversionDataRequired<T>();
    }

#pragma endregion

#pragma region Angle (requires IsAngle())

    template <typename T = double, AngleUnit Unit = UnitTraits<AngleUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsAngle(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsAngle());
      return ClampTo<T>(
        ConvertUnit(DoubleValue(conversionData), PrimitiveType(), ToUnitType(Unit)).value_or(0));
    }

#pragma endregion

#pragma region Time (requires IsTime())

    template <typename T = double, TimeUnit Unit = UnitTraits<TimeUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsTime(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsTime());
      return ClampTo<T>(
        ConvertUnit(DoubleValue(conversionData), PrimitiveType(), ToUnitType(Unit)).value_or(0));
    }

#pragma endregion

#pragma region Frequency

#pragma endregion

#pragma region Resolution (requires IsResolution())

    template <typename T = double, ResolutionUnit Unit = UnitTraits<ResolutionUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsResolution(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsResolution());
      return ClampTo<T>(
        ConvertUnit(DoubleValue(conversionData), PrimitiveType(), ToUnitType(Unit)).value_or(0));
    }

#pragma endregion

#pragma region Flex (requires IsFlex())

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsFlex(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsFlex());
      return Value<T>(conversionData);
    }

#pragma endregion

#pragma region Length (requires IsLength())

    /// @note Specialised for int64, uint64, int32, uint32, int16, uint16, float, double and LayoutUnit in
    /// PrimitiveValue.cpp.
    template <typename T = double>
    KRYS_NODISCARD T ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept;

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsLengthNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(IsLength());
      return ValueNoConversionDataRequired<T>(UnitType::px);
    }

#pragma endregion

#pragma region Non-converting

    template <typename T = double>
    KRYS_NODISCARD T Value(const ToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(DoubleValue(conversionData));
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueNoConversionDataRequired() const noexcept
    {
      return ClampTo<T>(DoubleValueNoConversionDataRequired());
    }

    template <typename T = double>
    KRYS_NODISCARD Maybe<T> ValueIfNotCalculated() const noexcept
    {
      if (IsCalculated())
      {
        return null;
      }

      return static_cast<T>(_value.Number);
    }

#pragma endregion

#pragma region Divides value by 100 if percentage

    template <typename T = double>
    KRYS_NODISCARD
    T ValueDividingBy100IfPercentage(const ToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(DoubleValueDividingBy100IfPercentage(conversionData));
    }

#pragma endregion

    // These return null for calc, for which range checking is not done at parse time:
    // <https://www.w3.org/TR/css3-values/#calc-range>.
    KRYS_NODISCARD Maybe<bool> IsZero() const noexcept;

    KRYS_NODISCARD Maybe<bool> IsOne() const noexcept;

    KRYS_NODISCARD Maybe<bool> IsPositive() const noexcept;

    KRYS_NODISCARD Maybe<bool> IsNegative() const noexcept;

    KRYS_NODISCARD const calc::Value *CSSCalcValue() const noexcept
    {
      return IsCalculated() ? _value.Calc : nullptr;
    }

    KRYS_NODISCARD RefPtr<const calc::Value> ProtectedCSSCalcValue() const noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const PrimitiveValue &other) const noexcept;

    void CollectComputedStyleDependencies(css::ComputedStyleDependencies &dependencies) const noexcept;

  private:
    // The stored unit: `Unknown` for every kind but a number, calc() included.
    KRYS_NODISCARD UnitType PrimitiveUnitType() const noexcept
    {
      return _unit;
    }

#pragma region Length converting

    KRYS_NODISCARD double ResolveAsLengthDouble(const ToLengthConversionData &conversionData) const noexcept;

#pragma endregion

#pragma region Arbitrarily converting

    KRYS_NODISCARD double DoubleValue(UnitType targetUnit, const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD double DoubleValueNoConversionDataRequired(UnitType targetUnit) const noexcept;

    template <typename T = double>
    KRYS_NODISCARD T Value(UnitType targetUnit, const ToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(DoubleValue(targetUnit, conversionData));
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueNoConversionDataRequired(UnitType targetUnit) const noexcept
    {
      return ClampTo<T>(DoubleValueNoConversionDataRequired(targetUnit));
    }

#pragma endregion

#pragma region Non-converting

    KRYS_NODISCARD double DoubleValue(const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD double DoubleValueNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(!IsCalculated());
      return _value.Number;
    }

    KRYS_NODISCARD double DoubleValueDividingBy100IfPercentage(const ToLengthConversionData &) const noexcept;

#pragma endregion

    KRYS_NODISCARD Maybe<double> DoubleValueInternal(UnitType targetUnit,
                                                     const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD Maybe<double> DoubleValueInternalWithoutConversionData(UnitType targetUnit) const noexcept;

    KRYS_NODISCARD bool AddDerivedHash(size_t &seed) const noexcept;

    KRYS_NODISCARD CSSOMString SerialiseInternal(const SerialisationContext &) const noexcept;

    KRYS_NODISCARD CSSOMString FormatNumberValue(CSSOMStringView suffix) const noexcept;

    KRYS_NODISCARD CSSOMString FormatIntegerValue(CSSOMStringView suffix) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PrimitiveValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsPrimitiveValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

namespace krys::boo::css
{
#pragma region ValueId helpers

  KRYS_NODISCARD inline ValueId GetValueId(const PrimitiveValue &value) noexcept
  {
    return value.ValueId();
  }

  KRYS_NODISCARD inline ValueId GetValueId(const PrimitiveValue *value) noexcept
  {
    return value ? GetValueId(*value) : ValueId::Invalid;
  }

  KRYS_NODISCARD inline ValueId GetValueId(const Value &value) noexcept
  {
    auto *primitiveValue = DynamicDowncast<PrimitiveValue>(value);
    return primitiveValue ? GetValueId(*primitiveValue) : ValueId::Invalid;
  }

  KRYS_NODISCARD inline ValueId GetValueId(const Value *value) noexcept
  {
    return value ? GetValueId(*value) : ValueId::Invalid;
  }

  KRYS_NODISCARD inline bool IsValueId(const PrimitiveValue &value, ValueId id) noexcept
  {
    return GetValueId(value) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const PrimitiveValue *value, ValueId id) noexcept
  {
    return GetValueId(value) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const RefPtr<PrimitiveValue> &value, ValueId id) noexcept
  {
    return GetValueId(value.get()) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const Ref<PrimitiveValue> &value, ValueId id) noexcept
  {
    return GetValueId(value.get()) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const Value &value, ValueId id) noexcept
  {
    return GetValueId(value) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const Value *value, ValueId id) noexcept
  {
    return GetValueId(value) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const RefPtr<Value> &value, ValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsValueId(const Ref<Value> &value, ValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsCustomIdentValue(const Value &value) noexcept
  {
    auto *primitiveValue = DynamicDowncast<PrimitiveValue>(value);
    return primitiveValue && primitiveValue->IsCustomIdent();
  }

#pragma endregion

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

  inline CSSOMString Value::CustomIdent() const noexcept
  {
    krys_debug_assert(IsCustomIdent());
    return Downcast<PrimitiveValue>(*this).StringValue();
  }

  inline bool Value::IsString() const noexcept
  {
    auto *value = DynamicDowncast<PrimitiveValue>(*this);
    return value && value->IsString();
  }

  inline CSSOMString Value::String() const noexcept
  {
    krys_debug_assert(IsString());
    return Downcast<PrimitiveValue>(*this).StringValue();
  }

  inline bool Value::IsInteger() const noexcept
  {
    auto *value = DynamicDowncast<PrimitiveValue>(*this);
    return value && value->IsInteger();
  }

  inline int64 Value::Integer(const ToLengthConversionData &conversionData) const noexcept
  {
    krys_debug_assert(IsInteger());
    return Downcast<PrimitiveValue>(*this).ResolveAsInteger(conversionData);
  }

#pragma endregion
}
