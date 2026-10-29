#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Core/FloatConversion.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <algorithm>
#include <charconv>

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD bool IsValidUnitTypeForDoubleConversion(UnitType unitType) noexcept
    {
      switch (unitType)
      {
        case UnitType::Attr:
        case UnitType::FontFamily:
        case UnitType::CustomIdent:
        case UnitType::PropertyId:
        case UnitType::String:
        case UnitType::Unknown:
        case UnitType::ValueId:     return false;
        case UnitType::Ident:       krys_debug_assert(false); return false;
        default:                    return true;
      }
    }

    KRYS_NODISCARD bool IsStringType(UnitType type) noexcept
    {
      return type == UnitType::String || type == UnitType::CustomIdent || type == UnitType::Attr
             || type == UnitType::FontFamily;
    }

    KRYS_NODISCARD HashMap<const PrimitiveValue *, CSSOMString> &SerialisedPrimitiveValues() noexcept
    {
      static NeverDestroyed<HashMap<const PrimitiveValue *, CSSOMString>> map;
      return map.get();
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
    SetPrimitiveUnitType(UnitType::PropertyId);
    _value.PropertyId = property;
  }

  PrimitiveValue::PrimitiveValue(double number, UnitType type) noexcept : css::Value(ValueType::Primitive)
  {
    SetPrimitiveUnitType(type);
    _value.Number = number;
  }

  PrimitiveValue::PrimitiveValue(const CSSOMString &string, UnitType type) noexcept
      : css::Value(ValueType::Primitive)
  {
    krys_debug_assert(IsStringType(type));
    SetPrimitiveUnitType(type);
    _value.String = CSSOMStringAtom(string).Storage();
  }

  PrimitiveValue::PrimitiveValue(StaticValueTag, css::ValueId valueId) noexcept
      : css::Value(ValueType::Primitive)
  {
    SetPrimitiveUnitType(UnitType::ValueId);
    _value.ValueId = valueId;

    // FIXME: We should handle this like webkit (by having ref count increment by 2 so a value of 1 is
    // reserved for static values that will not be destroyed) . For now, we will just increase the ref count
    // by 1.
    AddRef();
  }

  PrimitiveValue::PrimitiveValue(StaticValueTag, double number, UnitType type) noexcept
      : PrimitiveValue(number, type)
  {
    // FIXME: We should handle this like webkit (by having ref count increment by 2 so a value of 1 is
    // reserved for static values that will not be destroyed) . For now, we will just increase the ref count
    // by 1.
    AddRef();
  }

  PrimitiveValue::PrimitiveValue(StaticValueTag, CreateImplicitInitialValueTag) noexcept
      : PrimitiveValue(StaticValue, css::ValueId::Initial)
  {
    _isImplicitInitialValue = true;
  }

  PrimitiveValue::PrimitiveValue(Ref<calc::Value> value) noexcept : css::Value(ValueType::Primitive)
  {
    SetPrimitiveUnitType(UnitType::Calc);
    _value.Calc = value.release();
  }

  PrimitiveValue::PrimitiveValue(Ref<AttrValue> value) noexcept : css::Value(ValueType::Primitive)
  {
    SetPrimitiveUnitType(UnitType::Attr);
    _value.Attr = value.release();
  }

  PrimitiveValue::~PrimitiveValue() noexcept
  {
    // Strings are interned atoms that live for the whole program, so only the reference-counted members need
    // releasing.
    switch (PrimitiveUnitType())
    {
      case UnitType::Attr:                     _value.Attr->SubRef(); break;
      case UnitType::Calc:                     _value.Calc->SubRef(); break;
      case UnitType::CalcPercentageWithAngle:
      case UnitType::CalcPercentageWithLength: krys_debug_assert(false); break;
      default:                                 break;
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

  Ref<PrimitiveValue> PrimitiveValue::Create(double value, UnitType type) noexcept
  {
    switch (type)
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

    return AdoptRef(*new PrimitiveValue(value, type));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(const CSSOMString &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value, UnitType::String));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(const CSSOMStringAtom &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(CSSOMString(value.View()), UnitType::String));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(Ref<calc::Value> value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(krys::move(value)));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(Ref<AttrValue> value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(krys::move(value)));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateCustomIdent(const CSSOMString &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value, UnitType::CustomIdent));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateCustomIdent(const CSSOMStringAtom &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(CSSOMString(value.View()), UnitType::CustomIdent));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateFontFamily(const CSSOMString &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value, UnitType::FontFamily));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateInteger(double value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value, UnitType::Integer));
  }

#pragma endregion

  UnitType PrimitiveValue::PrimitiveType() const noexcept
  {
    auto type = PrimitiveUnitType();
    switch (type)
    {
      case UnitType::PropertyId:
      case UnitType::ValueId:
      case UnitType::CustomIdent: return UnitType::Ident;
      case UnitType::FontFamily:
        // Web-exposed content expects font family values to have UnitType::String primitive type so we need
        // to map our internal UnitType::FontFamily type here.
        return UnitType::String;
      default:
        if (auto *calcValue = CSSCalcValue())
        {
          return calcValue->PrimitiveType();
        }

        return type;
    }
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

  dom::ExceptionOr<float> PrimitiveValue::FloatValueDeprecated(UnitType targetUnit) const noexcept
  {
    auto result = DoubleValueInternalDeprecated(targetUnit);
    if (!result)
    {
      return dom::Exception(dom::ExceptionCode::InvalidAccessError);
    }

    return ClampTo<float>(*result);
  }

#pragma region Arbitrarily converting

  double PrimitiveValue::DoubleValue(UnitType targetUnit,
                                     const ToLengthConversionData &conversionData) const noexcept
  {
    return DoubleValueInternal(targetUnit, conversionData).value_or(0);
  }

  double PrimitiveValue::DoubleValueNoConversionDataRequired(UnitType targetUnit) const noexcept
  {
    krys_debug_assert(!IsCalculated());
    return DoubleValueInternalDeprecated(targetUnit).value_or(0);
  }

  double PrimitiveValue::DoubleValueDeprecated(UnitType targetUnit) const noexcept
  {
    return DoubleValueInternalDeprecated(targetUnit).value_or(0);
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

  double PrimitiveValue::DoubleValueDeprecated() const noexcept
  {
    if (auto *calcValue = CSSCalcValue())
    {
      return calcValue->DoubleValueDeprecated();
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

  double PrimitiveValue::DoubleValueDividingBy100IfPercentageNoConversionDataRequired() const noexcept
  {
    krys_debug_assert(IsNumberOrInteger() || IsPercentage());
    krys_debug_assert(!IsCalculated());

    if (IsPercentage())
    {
      return _value.Number / 100.0;
    }

    return _value.Number;
  }

  double PrimitiveValue::DoubleValueDividingBy100IfPercentageDeprecated() const noexcept
  {
    krys_debug_assert(IsNumberOrInteger() || IsPercentage());

    if (auto *calcValue = CSSCalcValue())
    {
      return calcValue->PrimitiveType() == UnitType::Percentage ? calcValue->DoubleValueDeprecated() / 100.0
                                                                : calcValue->DoubleValueDeprecated();
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
    PrimitiveValue::DoubleValueInternal(UnitType requestedUnitType,
                                        const ToLengthConversionData &conversionData) const noexcept
  {
    if (!IsValidUnitTypeForDoubleConversion(PrimitiveUnitType())
        || !IsValidUnitTypeForDoubleConversion(requestedUnitType))
    {
      return null;
    }

    UnitType sourceUnitType = PrimitiveType();
    if (requestedUnitType == sourceUnitType || requestedUnitType == UnitType::Dimension)
    {
      return DoubleValue(conversionData);
    }

    UnitCategory sourceCategory = UnitCategoryForType(sourceUnitType);
    krys_debug_assert(sourceCategory != UnitCategory::Other);

    UnitType targetUnitType = requestedUnitType;
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

  Maybe<double> PrimitiveValue::DoubleValueInternalDeprecated(UnitType requestedUnitType) const noexcept
  {
    if (!IsValidUnitTypeForDoubleConversion(PrimitiveUnitType())
        || !IsValidUnitTypeForDoubleConversion(requestedUnitType))
    {
      return null;
    }

    UnitType sourceUnitType = PrimitiveType();
    if (requestedUnitType == sourceUnitType || requestedUnitType == UnitType::Dimension)
    {
      return DoubleValueDeprecated();
    }

    UnitCategory sourceCategory = UnitCategoryForType(sourceUnitType);
    krys_debug_assert(sourceCategory != UnitCategory::Other);

    UnitType targetUnitType = requestedUnitType;
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

    double convertedValue = DoubleValueDeprecated();

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
    switch (PrimitiveUnitType())
    {
      case UnitType::String:
      case UnitType::CustomIdent:
      case UnitType::FontFamily:  return *_value.String;
      case UnitType::ValueId:     return CSSOMString(ToString(_value.ValueId));
      case UnitType::PropertyId:  return CSSOMString(ToString(_value.PropertyId));
      case UnitType::Attr:        return ProtectedCSSAttrValue()->CssText(DefaultSerialisationContext());
      default:                    return {};
    }
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
    auto type = PrimitiveUnitType();
    switch (type)
    {
      case UnitType::Attr:       return ProtectedCSSAttrValue()->CssText(context);
      case UnitType::Calc:       return ProtectedCSSCalcValue()->CssText(context);
      // FIXME: This isn't correct.
      case UnitType::Dimension:  return FormatNumberValue(u8"");
      case UnitType::FontFamily: return SerialiseFontFamily(*_value.String);
      case UnitType::Integer:    return FormatIntegerValue(u8"");
      case UnitType::QuirkyEm:   return FormatNumberValue(u8"em");
      case UnitType::String:     return SerialiseString(*_value.String);
      case UnitType::CustomIdent:
      {
        StringBuilder builder;
        SerialiseIdentifier(*_value.String, builder);
        return krys::move(builder).ToString();
      }

      case UnitType::CalcPercentageWithAngle:
      case UnitType::CalcPercentageWithLength:
      case UnitType::Ident:
      case UnitType::PropertyId:
      case UnitType::Unknown:
      case UnitType::ValueId:                  krys_debug_assert(false); return {};

      default:
        // Every remaining unit is a number with a unit suffix (Number has an empty suffix).
        return FormatNumberValue(UnitTypeString(type));
    }
  }

  CSSOMString PrimitiveValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    switch (PrimitiveUnitType())
    {
      case UnitType::Unknown:    return {};
      case UnitType::ValueId:    return CSSOMString(ToLower(_value.ValueId));
      case UnitType::PropertyId: return CSSOMString(ToString(_value.PropertyId));
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
    if (PrimitiveUnitType() != other.PrimitiveUnitType())
    {
      return false;
    }

    switch (PrimitiveUnitType())
    {
      case UnitType::Unknown:    return false;
      case UnitType::PropertyId: return _value.PropertyId == other._value.PropertyId;
      case UnitType::ValueId:    return _value.ValueId == other._value.ValueId;
      case UnitType::String:
      case UnitType::CustomIdent:
      case UnitType::FontFamily:
        return _value.String == other._value.String; // Interned, so equal iff same atom.
      case UnitType::Attr: return ProtectedCSSAttrValue()->Equals(*other.ProtectedCSSAttrValue());
      case UnitType::Calc: return ProtectedCSSCalcValue()->Equals(*other.ProtectedCSSCalcValue());
      case UnitType::Ident:
      case UnitType::CalcPercentageWithAngle:
      case UnitType::CalcPercentageWithLength:
        // FIXME: seems like these should be handled.
        krys_debug_assert(false);
        return false;
      default: return _value.Number == other._value.Number;
    }
  }

  bool PrimitiveValue::AddDerivedHash(size_t &seed) const noexcept
  {
    seed = Hash::Combine(seed, PrimitiveUnitType());

    switch (PrimitiveUnitType())
    {
      case UnitType::Unknown:                  break;
      case UnitType::PropertyId:               seed = Hash::Combine(seed, _value.PropertyId); break;
      case UnitType::ValueId:                  seed = Hash::Combine(seed, _value.ValueId); break;
      case UnitType::String:
      case UnitType::CustomIdent:
      case UnitType::FontFamily:               seed = Hash::Combine(seed, _value.String); break;
      case UnitType::Attr:                     seed = Hash::Combine(seed, _value.Attr); break;
      case UnitType::Calc:                     seed = Hash::Combine(seed, _value.Calc); break;
      case UnitType::Ident:
      case UnitType::CalcPercentageWithAngle:
      case UnitType::CalcPercentageWithLength: krys_debug_assert(false); return false;
      default:                                 seed = Hash::Combine(seed, _value.Number); break;
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
