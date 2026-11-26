#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Web/Css/Calc/Value.hpp"
#include "Krystal.Web/Css/CSSOM/Markup.hpp"
#include "Krystal.Web/Css/Values/ComputedStyleDependencies.hpp"
#include "Krystal.Web/Css/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Web/Css/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Css/Values/ToLengthConversionData.hpp"
#include "Krystal.Web/Css/Values/ValuePool.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Web/Style/Values/Primitives/LengthResolution.hpp"
#include <cmath>

namespace krys::web::css
{
  namespace
  {
    /// @brief The static pool's value for @p value, if it has one. A pool holds the whole numbers from 0 to
    /// one less than its size; -0 isn't among them, as it serialises differently.
    KRYS_NODISCARD PrimitiveValue *FindPooledPrimitiveValue(Span<AlignedStorage<PrimitiveValue>> pool,
                                                            double value) noexcept
    {
      // Checked before the conversion below, which is undefined for a double out of the index's range. NaN
      // fails it too.
      if (!(value >= 0.0 && value < static_cast<double>(pool.size())) || std::signbit(value))
      {
        return nullptr;
      }

      auto index = static_cast<size_t>(value);
      return static_cast<double>(index) == value ? pool[index].get() : nullptr;
    }
  }

#pragma region Construction

  PrimitiveValue::PrimitiveValue(double value, UnitType unit) noexcept : css::Value(ValueType::Primitive)
  {
    _header.Unit = unit;
    _payload.Number = value;
  }

  PrimitiveValue::PrimitiveValue(css::PropertyId property) noexcept : css::Value(ValueType::Primitive)
  {
    _header.PrimitiveKind = PrimitiveKind::PropertyId;
    _payload.PropertyId = property;
  }

  PrimitiveValue::PrimitiveValue(CSSOMStringAtom value, PrimitiveKind kind) noexcept
      : css::Value(ValueType::Primitive)
  {
    krys_debug_assert(kind == PrimitiveKind::String || kind == PrimitiveKind::CustomIdent
                      || kind == PrimitiveKind::FontFamily);
    _header.PrimitiveKind = kind;
    _payload.String = krys::move(value).LeakStorage();
  }

  PrimitiveValue::PrimitiveValue(Ref<calc::Value> value) noexcept : css::Value(ValueType::Primitive)
  {
    _header.PrimitiveKind = PrimitiveKind::Calc;
    _payload.Calc = value.release();
  }

  PrimitiveValue::PrimitiveValue(StaticValueTag, css::ValueId keyword) noexcept
      : css::Value(ValueType::Primitive)
  {
    _header.PrimitiveKind = PrimitiveKind::ValueId;
    _payload.ValueId = keyword;
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
    _header.IsImplicitInitialValue = true;
  }

  PrimitiveValue::~PrimitiveValue() noexcept
  {
    switch (_header.PrimitiveKind)
    {
      case PrimitiveKind::String:
      case PrimitiveKind::CustomIdent:
      case PrimitiveKind::FontFamily:  CSSOMStringAtom::DerefStorage(_payload.String); break;
      case PrimitiveKind::Calc:        _payload.Calc->SubRef(); break;
      case PrimitiveKind::Number:
      case PrimitiveKind::PropertyId:
      case PrimitiveKind::ValueId:     break;
    }
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(double value, UnitType unit) noexcept
  {
    Span<AlignedStorage<PrimitiveValue>> pool;
    switch (unit)
    {
      case UnitType::Number:     pool = CommonValuePool->_numberValues; break;
      case UnitType::Percentage: pool = CommonValuePool->_percentageValues; break;
      case UnitType::px:         pool = CommonValuePool->_pixelValues; break;
      default:                   break;
    }

    if (auto *pooled = FindPooledPrimitiveValue(pool, value))
    {
      return ShareRef(*pooled);
    }
    return AdoptRef(*new PrimitiveValue(value, unit));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateInteger(double value) noexcept
  {
    return Create(value, UnitType::Integer);
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(Ref<calc::Value> value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(krys::move(value)));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(css::PropertyId property) noexcept
  {
    return AdoptRef(*new PrimitiveValue(property));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(const CSSOMString &value) noexcept
  {
    return Create(CSSOMStringAtom(value));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(CSSOMStringAtom value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(krys::move(value), PrimitiveKind::String));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateCustomIdent(const CSSOMString &value) noexcept
  {
    return CreateCustomIdent(CSSOMStringAtom(value));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateCustomIdent(CSSOMStringAtom value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(krys::move(value), PrimitiveKind::CustomIdent));
  }

  Ref<PrimitiveValue> PrimitiveValue::CreateFontFamily(const CSSOMString &value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(CSSOMStringAtom(value), PrimitiveKind::FontFamily));
  }

#pragma endregion

#pragma region What it holds

  CSSOMString PrimitiveValue::StringValue() const noexcept
  {
    switch (_header.PrimitiveKind)
    {
      case PrimitiveKind::String:
      case PrimitiveKind::CustomIdent:
      case PrimitiveKind::FontFamily:  return CSSOMString(StringView());
      case PrimitiveKind::ValueId:     return CSSOMString(ToString(_payload.ValueId));
      case PrimitiveKind::PropertyId:  return CSSOMString(ToString(_payload.PropertyId));
      case PrimitiveKind::Number:
      case PrimitiveKind::Calc:        return {};
    }
    krys_unreachable();
  }

#pragma endregion

#pragma region Its unit

  UnitType PrimitiveValue::Unit() const noexcept
  {
    return IsCalc() ? _payload.Calc->PrimitiveType() : StoredUnit();
  }

  bool PrimitiveValue::IsCalcPercentageWithLength() const noexcept
  {
    return IsCalc() && _payload.Calc->IsPercentageWithLength();
  }

  bool PrimitiveValue::IsFontIndependentLength() const noexcept
  {
    if (!IsCalc())
    {
      return css::IsFontIndependentLength(StoredUnit());
    }

    auto dependencies = ComputedStyleDependencies();
    return IsLength() && dependencies.Properties.empty() && dependencies.RootProperties.empty()
           && !dependencies.ViewportDimensions && !dependencies.ContainerDimensions && !dependencies.Anchors;
  }

  bool PrimitiveValue::IsFontRelativeLength() const noexcept
  {
    if (!IsCalc())
    {
      return css::IsFontRelativeLength(StoredUnit());
    }

    // Only font-relative units add properties: font-size, and line-height for `lh` and `rlh`.
    auto dependencies = ComputedStyleDependencies();
    return !dependencies.Properties.empty() || !dependencies.RootProperties.empty();
  }

  bool PrimitiveValue::IsParentFontRelativeLength() const noexcept
  {
    if (!IsCalc())
    {
      return IsPercentage() || (IsFontRelativeLength() && !IsRootFontRelativeLength());
    }

    return IsPercentage() || IsCalcPercentageWithLength() || !ComputedStyleDependencies().Properties.empty();
  }

  bool PrimitiveValue::IsRootFontRelativeLength() const noexcept
  {
    if (!IsCalc())
    {
      return css::IsRootFontRelativeLength(StoredUnit());
    }

    return !ComputedStyleDependencies().RootProperties.empty();
  }

#pragma endregion

#pragma region Resolving

  template <typename T>
  T PrimitiveValue::ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept
  {
    double pixels = ResolveAsLengthDouble(conversionData);
    if constexpr (SameType<T, double>)
    {
      return pixels;
    }
    else if constexpr (SameType<T, float>)
    {
      return NarrowPrecisionToFloat(pixels);
    }
    else if constexpr (SameType<T, LayoutUnit>)
    {
      return LayoutUnit(pixels);
    }
    else
    {
      return style::RoundForImpreciseConversion<T>(pixels);
    }
  }

  template int16 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &) const noexcept;
  template uint16 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &) const noexcept;
  template int32 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &) const noexcept;
  template uint32 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &) const noexcept;
  template int64 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &) const noexcept;
  template uint64 PrimitiveValue::ResolveAsLength(const ToLengthConversionData &) const noexcept;
  template float PrimitiveValue::ResolveAsLength(const ToLengthConversionData &) const noexcept;
  template double PrimitiveValue::ResolveAsLength(const ToLengthConversionData &) const noexcept;
  template LayoutUnit PrimitiveValue::ResolveAsLength(const ToLengthConversionData &) const noexcept;

  double PrimitiveValue::ResolveAsLengthDouble(const ToLengthConversionData &conversionData) const noexcept
  {
    if (IsCalc())
    {
      // A calc() applies the conversion to each of its lengths.
      return _payload.Calc->ComputeLengthPx(conversionData);
    }

    auto lengthUnit = ToUnit<LengthUnit>(StoredUnit());
    if (!lengthUnit)
    {
      krys_debug_assert(false);
      return -1.0;
    }
    return style::ComputeNonCalcLengthDouble(_payload.Number, *lengthUnit, conversionData);
  }

  double PrimitiveValue::ResolveDouble(const ToLengthConversionData &conversionData) const noexcept
  {
    return IsCalc() ? _payload.Calc->DoubleValue(conversionData) : _payload.Number;
  }

  double PrimitiveValue::ResolveDouble(UnitType targetUnit,
                                       const ToLengthConversionData &conversionData) const noexcept
  {
    auto unit = Unit();
    if (unit == UnitType::Unknown)
    {
      return 0.0;
    }
    return ConvertUnitOrNumber(ResolveDouble(conversionData), unit, targetUnit).value_or(0.0);
  }

  double PrimitiveValue::ResolveDoubleWithoutConversionData(UnitType targetUnit) const noexcept
  {
    krys_debug_assert(!IsCalc());
    return ConvertUnitOrNumber(_payload.Number, StoredUnit(), targetUnit).value_or(0.0);
  }

  double PrimitiveValue::ResolveDoubleDividingBy100IfPercentage(
    const ToLengthConversionData &conversionData) const noexcept
  {
    krys_debug_assert(IsNumberOrInteger() || IsPercentage());
    double value = ResolveDouble(conversionData);
    return IsPercentage() ? value / 100.0 : value;
  }

#pragma endregion

#pragma region The Value operations

  CSSOMString PrimitiveValue::SerialiseNumber() const noexcept
  {
    double number = _payload.Number;
    switch (StoredUnit())
    {
      case UnitType::Unknown: return {};
      case UnitType::Integer:
      {
        // An integer is written without a fraction or exponent, unless it isn't finite.
        if (!std::isfinite(number))
        {
          return FormatNonfiniteCSSNumberValue(SerialisableNumber {.Value = number, .Suffix = u8""});
        }

        StringBuilder builder;
        builder.Append(number, u8"");
        return krys::move(builder).ToString();
      }
      default:
        return FormatCSSNumberValue(
          SerialisableNumber {.Value = number, .Suffix = UnitTypeString(StoredUnit())});
    }
  }

  /// @see https://www.w3.org/TR/cssom-1/#serialize-a-css-component-value
  CSSOMString PrimitiveValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    switch (_header.PrimitiveKind)
    {
      case PrimitiveKind::Number:     return SerialiseNumber();
      case PrimitiveKind::ValueId:    return CSSOMString(ToLower(_payload.ValueId));
      case PrimitiveKind::PropertyId: return CSSOMString(ToString(_payload.PropertyId));
      case PrimitiveKind::String:     return SerialiseString(CSSOMString(StringView()));
      case PrimitiveKind::FontFamily: return SerialiseFontFamily(CSSOMString(StringView()));
      case PrimitiveKind::Calc:       return _payload.Calc->CssText(context);
      case PrimitiveKind::CustomIdent:
      {
        StringBuilder builder;
        SerialiseIdentifier(CSSOMString(StringView()), builder);
        return krys::move(builder).ToString();
      }
    }
    krys_unreachable();
  }

  bool PrimitiveValue::Equals(const PrimitiveValue &other) const noexcept
  {
    if (_header.PrimitiveKind != other._header.PrimitiveKind || StoredUnit() != other.StoredUnit())
    {
      return false;
    }

    switch (_header.PrimitiveKind)
    {
      case PrimitiveKind::Number:
        // A number without a unit is no value, and equals nothing.
        return StoredUnit() != UnitType::Unknown && _payload.Number == other._payload.Number;
      case PrimitiveKind::ValueId:    return _payload.ValueId == other._payload.ValueId;
      case PrimitiveKind::PropertyId: return _payload.PropertyId == other._payload.PropertyId;
      case PrimitiveKind::String:
      case PrimitiveKind::CustomIdent:
      case PrimitiveKind::FontFamily:
        // Atoms are interned, so equal text means the same storage.
        return _payload.String == other._payload.String;
      case PrimitiveKind::Calc: return _payload.Calc->Equals(*other._payload.Calc);
    }
    krys_unreachable();
  }

  bool PrimitiveValue::AddDerivedHash(size_t &seed) const noexcept
  {
    seed = CombineFastHashes(seed, _header.PrimitiveKind, StoredUnit());
    switch (_header.PrimitiveKind)
    {
      case PrimitiveKind::Number:
        if (StoredUnit() != UnitType::Unknown)
        {
          seed = CombineFastHashes(seed, _payload.Number);
        }
        break;
      case PrimitiveKind::ValueId:     seed = CombineFastHashes(seed, _payload.ValueId); break;
      case PrimitiveKind::PropertyId:  seed = CombineFastHashes(seed, _payload.PropertyId); break;
      case PrimitiveKind::String:
      case PrimitiveKind::CustomIdent:
      case PrimitiveKind::FontFamily:  seed = CombineFastHashes(seed, _payload.String->Hash()); break;
      // By identity: equal calc()s may hash differently, which only costs a missed deduplication.
      case PrimitiveKind::Calc:        seed = CombineFastHashes(seed, _payload.Calc); break;
    }
    return true;
  }

  /// @see https://drafts.css-houdini.org/css-properties-values-api/#dependency-cycles
  void PrimitiveValue::CollectComputedStyleDependencies(
    css::ComputedStyleDependencies &dependencies) const noexcept
  {
    if (IsCalc())
    {
      _payload.Calc->CollectComputedStyleDependencies(dependencies);
    }
    else if (auto lengthUnit = ToUnit<LengthUnit>(StoredUnit()))
    {
      css::CollectComputedStyleDependencies(dependencies, *lengthUnit);
    }
  }

#pragma endregion
}
