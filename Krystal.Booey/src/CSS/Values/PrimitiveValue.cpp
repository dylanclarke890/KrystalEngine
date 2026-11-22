/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Values/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include <algorithm>
#include <charconv>

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD HashMap<const PrimitiveValue *, CSSOMString> &SerialisedPrimitiveValues() noexcept
    {
      static NeverDestroyed<HashMap<const PrimitiveValue *, CSSOMString>> map;
      return *map;
    }

    KRYS_NODISCARD PrimitiveValue *ValueFromPool(Span<AlignedStorage<PrimitiveValue>> pool,
                                                 double value) noexcept
    {
      // Only values that round-trip exactly through an index are pooled. Checking the range first avoids the
      // undefined behaviour of casting an out-of-range double to an integer.
      if (!(value >= 0.0 && value < static_cast<double>(pool.size())))
      {
        return nullptr;
      }

      auto poolIndex = static_cast<size_t>(value);
      double roundTripValue = static_cast<double>(poolIndex);
      if (std::ranges::equal(
            Span<const byte>(reinterpret_cast<const byte *>(&value), sizeof(double)),
            Span<const byte>(reinterpret_cast<const byte *>(&roundTripValue), sizeof(double))))
      {
        return pool[poolIndex].get();
      }

      return nullptr;
    }
  }

#pragma region Constructors

  PrimitiveValue::PrimitiveValue(css::PropertyId property) noexcept : css::Value(ValueType::Primitive)
  {
    _primitiveKind = PrimitiveKind::PropertyId;
    _value.PropertyId = property;
  }

  PrimitiveValue::PrimitiveValue(double value, UnitType unit) noexcept : css::Value(ValueType::Primitive)
  {
    _unit = unit;
    _value.Number = value;
  }

  PrimitiveValue::PrimitiveValue(const CSSOMString &value, PrimitiveKind kind) noexcept
      : css::Value(ValueType::Primitive)
  {
    krys_debug_assert(kind == PrimitiveKind::String || kind == PrimitiveKind::CustomIdent
                      || kind == PrimitiveKind::FontFamily);
    _primitiveKind = kind;
    _value.String = CSSOMStringAtom(value).LeakStorage();
  }

  PrimitiveValue::PrimitiveValue(StaticValueTag, css::ValueId keyword) noexcept
      : css::Value(ValueType::Primitive)
  {
    _primitiveKind = PrimitiveKind::ValueId;
    _value.ValueId = keyword;

    MakeStatic();
  }

  PrimitiveValue::PrimitiveValue(StaticValueTag, double value, UnitType unit) noexcept
      : PrimitiveValue(value, unit)
  {
    MakeStatic();
  }

  PrimitiveValue::PrimitiveValue(StaticValueTag, CreateImplicitInitialValueTag) noexcept
      : PrimitiveValue(StaticValue, css::ValueId::Initial)
  {
    _isImplicitInitialValue = true;
  }

  PrimitiveValue::PrimitiveValue(Ref<calc::Value> value) noexcept : css::Value(ValueType::Primitive)
  {
    _primitiveKind = PrimitiveKind::Calc;
    _value.Calc = value.release();
  }

  PrimitiveValue::~PrimitiveValue() noexcept
  {
    switch (_primitiveKind)
    {
      case PrimitiveKind::String:
      case PrimitiveKind::CustomIdent:
      case PrimitiveKind::FontFamily:  CSSOMStringAtom::DerefStorage(_value.String); break;
      case PrimitiveKind::Calc:        _value.Calc->SubRef(); break;
      case PrimitiveKind::Number:
      case PrimitiveKind::PropertyId:
      case PrimitiveKind::ValueId:     break;
    }

    if (_hasCachedCSSText)
    {
      krys_debug_assert(SerialisedPrimitiveValues().contains(this));
      SerialisedPrimitiveValues().erase(this);
    }
  }

#pragma endregion

#pragma region Static Creation

  Ref<PrimitiveValue> PrimitiveValue::Create(css::PropertyId property) noexcept
  {
    return AdoptRef(*new PrimitiveValue(property));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(double value) noexcept
  {
    if (auto *result = ValueFromPool(CommonValuePool->_numberValues, value))
    {
      return ShareRef(*result);
    }

    return AdoptRef(*new PrimitiveValue(value, UnitType::Number));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(double value, UnitType unit) noexcept
  {
    switch (unit)
    {
      case UnitType::Number:
        if (auto *result = ValueFromPool(CommonValuePool->_numberValues, value))
        {
          return ShareRef(*result);
        }
        break;
      case UnitType::Percentage:
        if (auto *result = ValueFromPool(CommonValuePool->_percentageValues, value))
        {
          return ShareRef(*result);
        }
        break;
      case UnitType::px:
        if (auto *result = ValueFromPool(CommonValuePool->_pixelValues, value))
        {
          return ShareRef(*result);
        }
        break;
      default: break;
    }

    return AdoptRef(*new PrimitiveValue(value, unit));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(const CSSOMString &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value, PrimitiveKind::String));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(const CSSOMStringAtom &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value.ToString(), PrimitiveKind::String));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(Ref<calc::Value> value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(krys::move(value)));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateCustomIdent(const CSSOMString &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value, PrimitiveKind::CustomIdent));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateCustomIdent(const CSSOMStringAtom &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value.ToString(), PrimitiveKind::CustomIdent));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateFontFamily(const CSSOMString &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value, PrimitiveKind::FontFamily));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateInteger(double value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value, UnitType::Integer));
  }

#pragma endregion

  UnitType PrimitiveValue::PrimitiveType() const noexcept
  {
    if (auto *calcValue = CSSCalcValue())
    {
      return calcValue->PrimitiveType();
    }
    return _unit;
  }

  bool PrimitiveValue::IsCalculatedPercentageWithLength() const noexcept
  {
    auto *calcValue = CSSCalcValue();
    return calcValue && calcValue->IsPercentageWithLength();
  }

  RefPtr<const calc::Value> PrimitiveValue::ProtectedCSSCalcValue() const noexcept
  {
    return ShareRefPtr(CSSCalcValue());
  }

  bool PrimitiveValue::ConversionToCanonicalUnitRequiresConversionData() const noexcept
  {
    if (IsCalculated())
    {
      return _value.Calc->RequiresConversionData();
    }

    return css::ConversionToCanonicalUnitRequiresConversionData(PrimitiveType());
  }

#pragma region Length converting

  template <>
  int64 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept
  {
    return RoundForImpreciseConversion<int64>(ResolveAsLengthDouble(conversionData));
  }

  template <>
  uint64 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept
  {
    return RoundForImpreciseConversion<uint64>(ResolveAsLengthDouble(conversionData));
  }

  template <>
  int32 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept
  {
    return RoundForImpreciseConversion<int32>(ResolveAsLengthDouble(conversionData));
  }

  template <>
  uint32 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept
  {
    return RoundForImpreciseConversion<uint32>(ResolveAsLengthDouble(conversionData));
  }

  template <>
  int16 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept
  {
    return RoundForImpreciseConversion<int16>(ResolveAsLengthDouble(conversionData));
  }

  template <>
  uint16 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept
  {
    return RoundForImpreciseConversion<uint16>(ResolveAsLengthDouble(conversionData));
  }

  template <>
  float PrimitiveValue::ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept
  {
    return NarrowPrecisionToFloat(ResolveAsLengthDouble(conversionData));
  }

  template <>
  double PrimitiveValue::ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept
  {
    return ResolveAsLengthDouble(conversionData);
  }

  template <>
  LayoutUnit PrimitiveValue::ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept
  {
    return LayoutUnit(ResolveAsLengthDouble(conversionData));
  }

  double PrimitiveValue::ResolveAsLengthDouble(const ToLengthConversionData &conversionData) const noexcept
  {
    if (auto *calcValue = CSSCalcValue())
    {
      // The multiplier and factor is applied to each value in the calc expression individually.
      return calcValue->ComputeLengthPx(conversionData);
    }

    auto lengthUnit = ToLengthUnit(PrimitiveType());
    if (!lengthUnit)
    {
      krys_debug_assert(false);
      return -1.0;
    }

    return style::ComputeNonCalcLengthDouble(_value.Number, *lengthUnit, conversionData);
  }

#pragma endregion

#pragma region Arbitrarily converting

  double PrimitiveValue::DoubleValue(UnitType targetUnit,
                                     const ToLengthConversionData &conversionData) const noexcept
  {
    return DoubleValueInternal(targetUnit, conversionData).value_or(0);
  }

  double PrimitiveValue::DoubleValueNoConversionDataRequired(UnitType targetUnit) const noexcept
  {
    krys_debug_assert(!IsCalculated());
    return DoubleValueInternalWithoutConversionData(targetUnit).value_or(0);
  }

#pragma endregion

#pragma region Non-converting

  double PrimitiveValue::DoubleValue(const ToLengthConversionData &conversionData) const noexcept
  {
    if (auto *calcValue = CSSCalcValue())
    {
      return calcValue->DoubleValue(conversionData);
    }

    return _value.Number;
  }

  double PrimitiveValue::DoubleValueDividingBy100IfPercentage(
    const ToLengthConversionData &conversionData) const noexcept
  {
    krys_debug_assert(IsNumberOrInteger() || IsPercentage());

    if (auto *calcValue = CSSCalcValue())
    {
      return calcValue->PrimitiveType() == UnitType::Percentage
               ? calcValue->DoubleValue(conversionData) / 100.0
               : calcValue->DoubleValue(conversionData);
    }

    if (IsPercentage())
    {
      return _value.Number / 100.0;
    }

    return _value.Number;
  }

#pragma endregion

  Maybe<bool> PrimitiveValue::IsZero() const noexcept
  {
    if (IsCalculated())
    {
      return null;
    }

    return !_value.Number;
  }

  Maybe<bool> PrimitiveValue::IsOne() const noexcept
  {
    if (IsCalculated())
    {
      return null;
    }

    return _value.Number == 1;
  }

  Maybe<bool> PrimitiveValue::IsPositive() const noexcept
  {
    if (IsCalculated())
    {
      return null;
    }

    return _value.Number > 0;
  }

  Maybe<bool> PrimitiveValue::IsNegative() const noexcept
  {
    if (IsCalculated())
    {
      return null;
    }

    return _value.Number < 0;
  }

  Maybe<double>
    PrimitiveValue::DoubleValueInternal(UnitType targetUnit,
                                        const ToLengthConversionData &conversionData) const noexcept
  {
    if ((_primitiveKind != PrimitiveKind::Number && _primitiveKind != PrimitiveKind::Calc)
        || (_primitiveKind == PrimitiveKind::Number && _unit == UnitType::Unknown)
        || targetUnit == UnitType::Unknown)
    {
      return null;
    }

    UnitType sourceUnitType = PrimitiveType();
    if (targetUnit == sourceUnitType)
    {
      return DoubleValue(conversionData);
    }

    UnitCategory sourceCategory = UnitCategoryForType(sourceUnitType);
    krys_debug_assert(sourceCategory != UnitCategory::Other);

    UnitType targetUnitType = targetUnit;
    UnitCategory targetCategory = UnitCategoryForType(targetUnitType);
    krys_debug_assert(targetCategory != UnitCategory::Other);

    // Cannot convert between unrelated unit categories if one of them is not UnitCategory::Number.
    if (sourceCategory != targetCategory && sourceCategory != UnitCategory::Number
        && targetCategory != UnitCategory::Number)
    {
      return null;
    }

    if (targetCategory == UnitCategory::Number)
    {
      // Cannot convert between numbers and percent.
      if (sourceCategory == UnitCategory::Percent)
      {
        return null;
      }

      // We interpret conversion to UnitType::Number as conversion to a canonical unit in this value's
      // category.
      targetUnitType = CanonicalUnitTypeForCategory(sourceCategory);
      if (targetUnitType == UnitType::Unknown)
      {
        return null;
      }
    }

    if (sourceUnitType == UnitType::Number || sourceUnitType == UnitType::Integer)
    {
      // Cannot convert between numbers and percent.
      if (targetCategory == UnitCategory::Percent)
      {
        return null;
      }

      // We interpret conversion from UnitType::Number in the same way as Parser::ValidUnit() while using
      // non-strict mode.
      sourceUnitType = CanonicalUnitTypeForCategory(targetCategory);
      if (sourceUnitType == UnitType::Unknown)
      {
        return null;
      }
    }

    double convertedValue = DoubleValue(conversionData);

    // If we don't need to scale it, don't worry about if we can scale it.
    if (sourceUnitType == targetUnitType)
    {
      return convertedValue;
    }

    // First convert the value from PrimitiveUnitType() to canonical type.
    auto sourceFactor = ConversionToCanonicalUnitsScaleFactor(sourceUnitType);
    if (!sourceFactor.has_value())
    {
      return null;
    }
    convertedValue *= *sourceFactor;

    // Now convert from canonical type to the target unitType.
    auto targetFactor = ConversionToCanonicalUnitsScaleFactor(targetUnitType);
    if (!targetFactor.has_value())
    {
      return null;
    }
    convertedValue /= *targetFactor;

    return convertedValue;
  }

  Maybe<double> PrimitiveValue::DoubleValueInternalWithoutConversionData(UnitType targetUnit) const noexcept
  {
    if ((_primitiveKind != PrimitiveKind::Number && _primitiveKind != PrimitiveKind::Calc)
        || (_primitiveKind == PrimitiveKind::Number && _unit == UnitType::Unknown)
        || targetUnit == UnitType::Unknown)
    {
      return null;
    }

    UnitType sourceUnitType = PrimitiveType();
    if (targetUnit == sourceUnitType)
    {
      return _value.Number;
    }

    UnitCategory sourceCategory = UnitCategoryForType(sourceUnitType);
    krys_debug_assert(sourceCategory != UnitCategory::Other);

    UnitType targetUnitType = targetUnit;
    UnitCategory targetCategory = UnitCategoryForType(targetUnitType);
    krys_debug_assert(targetCategory != UnitCategory::Other);

    // Cannot convert between unrelated unit categories if one of them is not UnitCategory::Number.
    if (sourceCategory != targetCategory && sourceCategory != UnitCategory::Number
        && targetCategory != UnitCategory::Number)
    {
      return null;
    }

    if (targetCategory == UnitCategory::Number)
    {
      // Cannot convert between numbers and percent.
      if (sourceCategory == UnitCategory::Percent)
      {
        return null;
      }

      // We interpret conversion to UnitType::Number as conversion to a canonical unit in this value's
      // category.
      targetUnitType = CanonicalUnitTypeForCategory(sourceCategory);
      if (targetUnitType == UnitType::Unknown)
      {
        return null;
      }
    }

    if (sourceUnitType == UnitType::Number || sourceUnitType == UnitType::Integer)
    {
      // Cannot convert between numbers and percent.
      if (targetCategory == UnitCategory::Percent)
      {
        return null;
      }

      // We interpret conversion from UnitType::Number in the same way as Parser::ValidUnit() while using
      // non-strict mode.
      sourceUnitType = CanonicalUnitTypeForCategory(targetCategory);
      if (sourceUnitType == UnitType::Unknown)
      {
        return null;
      }
    }

    double convertedValue = _value.Number;

    // If we don't need to scale it, don't worry about if we can scale it.
    if (sourceUnitType == targetUnitType)
    {
      return convertedValue;
    }

    // First convert the value from PrimitiveUnitType() to canonical type.
    auto sourceFactor = ConversionToCanonicalUnitsScaleFactor(sourceUnitType);
    if (!sourceFactor.has_value())
    {
      return null;
    }
    convertedValue *= *sourceFactor;

    // Now convert from canonical type to the target unitType.
    auto targetFactor = ConversionToCanonicalUnitsScaleFactor(targetUnitType);
    if (!targetFactor.has_value())
    {
      return null;
    }
    convertedValue /= *targetFactor;

    return convertedValue;
  }

  CSSOMString PrimitiveValue::StringValue() const noexcept
  {
    switch (_primitiveKind)
    {
      case PrimitiveKind::String:
      case PrimitiveKind::CustomIdent:
      case PrimitiveKind::FontFamily:  return CSSOMString(_value.String->View());
      case PrimitiveKind::ValueId:     return CSSOMString(ToString(_value.ValueId));
      case PrimitiveKind::PropertyId:  return CSSOMString(ToString(_value.PropertyId));
      case PrimitiveKind::Number:
      case PrimitiveKind::Calc:        return {};
    }
    krys_unreachable();
  }

  CSSOMString PrimitiveValue::FormatNumberValue(CSSOMStringView suffix) const noexcept
  {
    return FormatCSSNumberValue(SerialisableNumber {.Value = _value.Number, .Suffix = suffix});
  }

  CSSOMString PrimitiveValue::FormatIntegerValue(CSSOMStringView suffix) const noexcept
  {
    if (!std::isfinite(_value.Number))
    {
      return FormatNonfiniteCSSNumberValue(SerialisableNumber {.Value = _value.Number, .Suffix = suffix});
    }

    StringBuilder builder;
    builder.Append(_value.Number, suffix);
    return krys::move(builder).ToString();
  }

  CSSOMString PrimitiveValue::SerialiseInternal(const SerialisationContext &context) const noexcept
  {
    switch (_primitiveKind)
    {
      case PrimitiveKind::Calc:       return ProtectedCSSCalcValue()->CssText(context);
      case PrimitiveKind::FontFamily: return SerialiseFontFamily(CSSOMString(_value.String->View()));
      case PrimitiveKind::String:     return SerialiseString(CSSOMString(_value.String->View()));
      case PrimitiveKind::CustomIdent:
      {
        StringBuilder builder;
        SerialiseIdentifier(CSSOMString(_value.String->View()), builder);
        return krys::move(builder).ToString();
      }

      case PrimitiveKind::PropertyId:
      case PrimitiveKind::ValueId:    krys_debug_assert(false); return {};
      case PrimitiveKind::Number:     break;
    }

    switch (_unit)
    {
      case UnitType::Integer:  return FormatIntegerValue(u8"");
      case UnitType::QuirkyEm: return FormatNumberValue(u8"em");
      case UnitType::Unknown:  krys_debug_assert(false); return {};
      default:
        // Every remaining unit is a number with a unit suffix (Number has an empty suffix).
        return FormatNumberValue(UnitTypeString(_unit));
    }
  }

  CSSOMString PrimitiveValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    if (_primitiveKind == PrimitiveKind::Number && _unit == UnitType::Unknown)
    {
      return {};
    }
    switch (_primitiveKind)
    {
      case PrimitiveKind::ValueId:    return CSSOMString(ToLower(_value.ValueId));
      case PrimitiveKind::PropertyId: return CSSOMString(ToString(_value.PropertyId));
      default:
      {
        auto &map = SerialisedPrimitiveValues();
        krys_debug_assert(map.contains(this) == _hasCachedCSSText);
        if (_hasCachedCSSText)
        {
          return map.at(this);
        }

        CSSOMString serialisedValue = SerialiseInternal(context);
        _hasCachedCSSText = true;
        map.emplace(this, serialisedValue);
        return serialisedValue;
      }
    }
  }

  bool PrimitiveValue::Equals(const PrimitiveValue &other) const noexcept
  {
    if (_primitiveKind != other._primitiveKind || _unit != other._unit)
    {
      return false;
    }

    switch (_primitiveKind)
    {
      case PrimitiveKind::PropertyId: return _value.PropertyId == other._value.PropertyId;
      case PrimitiveKind::ValueId:    return _value.ValueId == other._value.ValueId;
      case PrimitiveKind::String:
      case PrimitiveKind::CustomIdent:
      case PrimitiveKind::FontFamily:
        return _value.String == other._value.String; // Interned, so equal iff same atom.
      case PrimitiveKind::Calc:   return ProtectedCSSCalcValue()->Equals(*other.ProtectedCSSCalcValue());
      case PrimitiveKind::Number: return _unit != UnitType::Unknown && _value.Number == other._value.Number;
    }
    krys_unreachable();
  }

  bool PrimitiveValue::AddDerivedHash(size_t &seed) const noexcept
  {
    seed = CombineFastHashes(seed, _primitiveKind, _unit);

    switch (_primitiveKind)
    {
      case PrimitiveKind::PropertyId:  seed = CombineFastHashes(seed, _value.PropertyId); break;
      case PrimitiveKind::ValueId:     seed = CombineFastHashes(seed, _value.ValueId); break;
      case PrimitiveKind::String:
      case PrimitiveKind::CustomIdent:
      case PrimitiveKind::FontFamily:  seed = CombineFastHashes(seed, _value.String->Hash()); break;
      case PrimitiveKind::Calc:        seed = CombineFastHashes(seed, _value.Calc); break;
      case PrimitiveKind::Number:
        if (_unit != UnitType::Unknown)
        {
          seed = CombineFastHashes(seed, _value.Number);
        }
        break;
    }

    return true;
  }

  /// @see https://drafts.css-houdini.org/css-properties-values-api/#dependency-cycles
  void PrimitiveValue::CollectComputedStyleDependencies(
    css::ComputedStyleDependencies &dependencies) const noexcept
  {
    if (auto *calcValue = CSSCalcValue())
    {
      calcValue->CollectComputedStyleDependencies(dependencies);
      return;
    }

    if (auto lengthUnit = ToLengthUnit(PrimitiveUnitType()))
    {
      css::CollectComputedStyleDependencies(dependencies, *lengthUnit);
    }
  }
}
