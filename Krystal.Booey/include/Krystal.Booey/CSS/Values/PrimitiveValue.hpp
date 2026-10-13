#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/AttrValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
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

  private:
    union ValueUnion
    {
      css::PropertyId PropertyId;
      css::ValueId ValueId;
      double Number;
      StringAtomStorage String;
      const calc::Value *Calc;
      const AttrValue *Attr;
    } _value;

    struct CreateImplicitInitialValueTag
    {
    };

    constexpr static CreateImplicitInitialValueTag CreateImplicitInitialValue {};

#pragma region Constructors

    explicit PrimitiveValue(css::PropertyId property) noexcept;

    PrimitiveValue(const CSSOMString &value, UnitType unit) noexcept;

    PrimitiveValue(double value, UnitType unit) noexcept;

    explicit PrimitiveValue(Ref<calc::Value> calc) noexcept;

    explicit PrimitiveValue(Ref<AttrValue> attr) noexcept;

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

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(Ref<AttrValue> value) noexcept;

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

    KRYS_NODISCARD bool IsAttr() const noexcept
    {
      return PrimitiveUnitType() == UnitType::Attr;
    }

    KRYS_NODISCARD bool IsFontIndependentLength() const noexcept
    {
      return IsFontIndependentLength(PrimitiveUnitType());
    }

    KRYS_NODISCARD bool IsFontRelativeLength() const noexcept
    {
      return IsFontRelativeLength(PrimitiveUnitType());
    }

    KRYS_NODISCARD bool IsParentFontRelativeLength() const noexcept
    {
      return IsPercentage() || (IsFontRelativeLength() && !IsRootFontRelativeLength());
    }

    KRYS_NODISCARD bool IsRootFontRelativeLength() const noexcept
    {
      return IsRootFontRelativeLength(PrimitiveUnitType());
    }

    KRYS_NODISCARD bool IsQuirkyEms() const noexcept
    {
      return PrimitiveType() == UnitType::QuirkyEm;
    }

    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return IsLength(PrimitiveType());
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

    KRYS_NODISCARD bool IsFrequency() const noexcept
    {
      return UnitCategoryForType(PrimitiveType()) == UnitCategory::Frequency;
    }

    KRYS_NODISCARD bool IsCalculated() const noexcept
    {
      return PrimitiveUnitType() == UnitType::Calc;
    }

    KRYS_NODISCARD bool IsCalculatedPercentageWithLength() const noexcept
    {
      return PrimitiveType() == UnitType::CalcPercentageWithLength;
    }

    KRYS_NODISCARD bool IsDotsPerInch() const noexcept
    {
      return PrimitiveType() == UnitType::dpi;
    }

    KRYS_NODISCARD bool IsDotsPerPixel() const noexcept
    {
      return PrimitiveType() == UnitType::dppx;
    }

    KRYS_NODISCARD bool IsDotsPerCentimeter() const noexcept
    {
      return PrimitiveType() == UnitType::dpcm;
    }

    KRYS_NODISCARD bool IsX() const noexcept
    {
      return PrimitiveType() == UnitType::x;
    }

    KRYS_NODISCARD bool IsResolution() const noexcept
    {
      return UnitCategoryForType(PrimitiveType()) == UnitCategory::Resolution;
    }

    KRYS_NODISCARD bool IsViewportPercentageLength() const noexcept
    {
      return IsViewportPercentageLength(PrimitiveUnitType());
    }

    KRYS_NODISCARD bool IsContainerPercentageLength() const noexcept
    {
      return IsContainerPercentageLength(PrimitiveUnitType());
    }

    KRYS_NODISCARD bool IsFlex() const noexcept
    {
      return PrimitiveType() == UnitType::fr;
    }

    KRYS_NODISCARD bool ConversionToCanonicalUnitRequiresConversionData() const noexcept;

    KRYS_NODISCARD constexpr static bool IsLength(UnitType type) noexcept
    {
      return type == UnitType::em || type == UnitType::ex || type == UnitType::px || type == UnitType::cm
             || type == UnitType::mm || type == UnitType::in || type == UnitType::pt || type == UnitType::pc
             || type == UnitType::Q || IsFontRelativeLength(type) || IsViewportPercentageLength(type)
             || IsContainerPercentageLength(type) || type == UnitType::QuirkyEm;
    }

#pragma endregion

#pragma region Keywords, Properties and Strings

    KRYS_NODISCARD bool IsValueId() const noexcept
    {
      return PrimitiveUnitType() == UnitType::ValueId;
    }

    KRYS_NODISCARD css::ValueId ValueId() const noexcept
    {
      return IsValueId() ? _value.ValueId : css::ValueId::Invalid;
    }

    KRYS_NODISCARD bool IsPropertyId() const noexcept
    {
      return PrimitiveUnitType() == UnitType::PropertyId;
    }

    KRYS_NODISCARD css::PropertyId PropertyId() const noexcept
    {
      return IsPropertyId() ? _value.PropertyId : css::PropertyId::Invalid;
    }

    KRYS_NODISCARD bool IsString() const noexcept
    {
      return PrimitiveUnitType() == UnitType::String;
    }

    KRYS_NODISCARD bool IsCustomIdent() const noexcept
    {
      return PrimitiveUnitType() == UnitType::CustomIdent;
    }

    KRYS_NODISCARD CSSOMString CustomIdent() const noexcept
    {
      krys_debug_assert(IsCustomIdent());
      return StringValue();
    }

    KRYS_NODISCARD bool IsFontFamily() const noexcept
    {
      return PrimitiveUnitType() == UnitType::FontFamily;
    }

    KRYS_NODISCARD CSSOMString StringValue() const noexcept;

#pragma endregion

    KRYS_NODISCARD UnitType PrimitiveType() const noexcept;

    /// @brief Exposed for the deprecated CSSOM. Throws if conversion to `targetUnit` is not allowed.
    KRYS_NODISCARD dom::ExceptionOr<float> FloatValueDeprecated(UnitType targetUnit) const noexcept;

#pragma region Integer (requires IsInteger())

    template <typename T = int64>
    KRYS_NODISCARD T ResolveAsInteger(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsInteger());
      return Value<T>(conversionData);
    }

    template <typename T = int64>
    KRYS_NODISCARD T ResolveAsIntegerNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(IsInteger());
      return ValueNoConversionDataRequired<T>();
    }

    template <typename T = int64>
    KRYS_NODISCARD T ResolveAsIntegerDeprecated() const noexcept
    {
      krys_debug_assert(IsInteger());
      return ValueDeprecated<T>();
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
    KRYS_NODISCARD T ResolveAsNumberNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(IsNumberOrInteger());
      return ValueNoConversionDataRequired<T>(UnitType::Number);
    }

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsNumberDeprecated() const noexcept
    {
      krys_debug_assert(IsNumberOrInteger());
      return ValueDeprecated<T>(UnitType::Number);
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

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsPercentageDeprecated() const noexcept
    {
      krys_debug_assert(IsPercentage());
      return ValueDeprecated<T>();
    }

    template <typename T = double>
    KRYS_NODISCARD Maybe<T> ResolveAsPercentageIfNotCalculated() const noexcept
    {
      krys_debug_assert(IsPercentage());
      return ValueIfNotCalculated<T>();
    }

#pragma endregion

#pragma region Angle (requires IsAngle())

    template <AngleUnit Unit, typename T = double>
    KRYS_NODISCARD constexpr static T ComputeAngle(UnitType type, T angle) noexcept
    {
      if constexpr (Unit == AngleUnit::Deg)
      {
        switch (type)
        {
          case UnitType::deg:  return angle;
          case UnitType::rad:  return rad2deg(angle);
          case UnitType::grad: return grad2deg(angle);
          case UnitType::turn: return turn2deg(angle);
          default:             krys_debug_assert(false); return 0;
        }
      }
      else if constexpr (Unit == AngleUnit::Rad)
      {
        switch (type)
        {
          case UnitType::deg:  return deg2rad(angle);
          case UnitType::rad:  return angle;
          case UnitType::grad: return grad2rad(angle);
          case UnitType::turn: return turn2rad(angle);
          default:             krys_debug_assert(false); return 0;
        }
      }
      else if constexpr (Unit == AngleUnit::Grad)
      {
        switch (type)
        {
          case UnitType::deg:  return deg2grad(angle);
          case UnitType::rad:  return rad2grad(angle);
          case UnitType::grad: return angle;
          case UnitType::turn: return turn2grad(angle);
          default:             krys_debug_assert(false); return 0;
        }
      }
      else if constexpr (Unit == AngleUnit::Turn)
      {
        switch (type)
        {
          case UnitType::deg:  return deg2turn(angle);
          case UnitType::rad:  return rad2turn(angle);
          case UnitType::grad: return grad2turn(angle);
          case UnitType::turn: return angle;
          default:             krys_debug_assert(false); return 0;
        }
      }
    }

    template <typename T = double, AngleUnit Unit = UnitTraits<AngleUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsAngle(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsAngle());
      return ClampTo<T>(ComputeAngle<Unit>(PrimitiveType(), Value<double>(conversionData)));
    }

    template <typename T = double, AngleUnit Unit = UnitTraits<AngleUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsAngleNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(IsAngle());
      return ClampTo<T>(ComputeAngle<Unit>(PrimitiveType(), ValueNoConversionDataRequired<double>()));
    }

    template <typename T = double, AngleUnit Unit = UnitTraits<AngleUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsAngleDeprecated() const noexcept
    {
      krys_debug_assert(IsAngle());
      return ClampTo<T>(ComputeAngle<Unit>(PrimitiveType(), ValueDeprecated<double>()));
    }

#pragma endregion

#pragma region Time (requires IsTime())

    template <TimeUnit Unit, typename T = double>
    KRYS_NODISCARD constexpr static T ComputeTime(UnitType type, T value) noexcept
    {
      if constexpr (Unit == TimeUnit::S)
      {
        switch (type)
        {
          case UnitType::s:  return value;
          case UnitType::ms: return value * secsPerMs;
          default:           krys_debug_assert(false); return 0;
        }
      }
      else if constexpr (Unit == TimeUnit::Ms)
      {
        switch (type)
        {
          case UnitType::s:  return value / secsPerMs;
          case UnitType::ms: return value;
          default:           krys_debug_assert(false); return 0;
        }
      }
    }

    template <typename T = double, TimeUnit Unit = UnitTraits<TimeUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsTime(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsTime());
      return ClampTo<T>(ComputeTime<Unit>(PrimitiveType(), Value<double>(conversionData)));
    }

    template <typename T = double, TimeUnit Unit = UnitTraits<TimeUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsTimeNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(IsTime());
      return ClampTo<T>(ComputeTime<Unit>(PrimitiveType(), ValueNoConversionDataRequired<double>()));
    }

#pragma endregion

#pragma region Frequency

    template <FrequencyUnit Unit, typename T = double>
    KRYS_NODISCARD constexpr static T ComputeFrequency(UnitType type, T value) noexcept
    {
      if constexpr (Unit == FrequencyUnit::Hz)
      {
        switch (type)
        {
          case UnitType::hz:  return value;
          case UnitType::khz: return value * hertzPerKilohertz;
          default:            krys_debug_assert(false); return 0;
        }
      }
      else if constexpr (Unit == FrequencyUnit::Khz)
      {
        switch (type)
        {
          case UnitType::hz:  return value / hertzPerKilohertz;
          case UnitType::khz: return value;
          default:            krys_debug_assert(false); return 0;
        }
      }
    }

#pragma endregion

#pragma region Resolution (requires IsResolution())

    template <ResolutionUnit Unit, typename T = double>
    KRYS_NODISCARD constexpr static T ComputeResolution(UnitType type, T resolution) noexcept
    {
      if constexpr (Unit == ResolutionUnit::Dppx)
      {
        switch (type)
        {
          case UnitType::dppx: return resolution;
          case UnitType::x:    return resolution * dppxPerX;
          case UnitType::dpi:  return resolution * dppxPerDpi;
          case UnitType::dpcm: return resolution * dppxPerDpcm;
          default:             krys_debug_assert(false); return 0;
        }
      }
      else if constexpr (Unit == ResolutionUnit::X)
      {
        switch (type)
        {
          case UnitType::dppx: return resolution / dppxPerX;
          case UnitType::x:    return resolution;
          case UnitType::dpi:  return resolution * dppxPerDpi / dppxPerX;
          case UnitType::dpcm: return resolution * dppxPerDpcm / dppxPerX;
          default:             krys_debug_assert(false); return 0;
        }
      }
      else if constexpr (Unit == ResolutionUnit::Dpi)
      {
        switch (type)
        {
          case UnitType::dppx: return resolution / dppxPerDpi;
          case UnitType::x:    return resolution * dppxPerX / dppxPerDpi;
          case UnitType::dpi:  return resolution;
          case UnitType::dpcm: return resolution * dppxPerDpcm / dppxPerDpi;
          default:             krys_debug_assert(false); return 0;
        }
      }
      else if constexpr (Unit == ResolutionUnit::Dpcm)
      {
        switch (type)
        {
          case UnitType::dppx: return resolution / dppxPerDpcm;
          case UnitType::x:    return resolution * dppxPerX / dppxPerDpcm;
          case UnitType::dpi:  return resolution * dppxPerDpi / dppxPerDpcm;
          case UnitType::dpcm: return resolution;
          default:             krys_debug_assert(false); return 0;
        }
      }
    }

    template <typename T = double, ResolutionUnit Unit = UnitTraits<ResolutionUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsResolution(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsResolution());
      return ClampTo<T>(ComputeResolution<Unit>(PrimitiveType(), Value<double>(conversionData)));
    }

    template <typename T = double, ResolutionUnit Unit = UnitTraits<ResolutionUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsResolutionNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(IsResolution());
      return ClampTo<T>(ComputeResolution<Unit>(PrimitiveType(), ValueNoConversionDataRequired<double>()));
    }

    template <typename T = double, ResolutionUnit Unit = UnitTraits<ResolutionUnit>::Canonical>
    KRYS_NODISCARD T ResolveAsResolutionDeprecated() const noexcept
    {
      krys_debug_assert(IsResolution());
      return ClampTo<T>(ComputeResolution<Unit>(PrimitiveType(), ValueDeprecated<double>()));
    }

#pragma endregion

#pragma region Flex (requires IsFlex())

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsFlex(const ToLengthConversionData &conversionData) const noexcept
    {
      krys_debug_assert(IsFlex());
      return Value<T>(conversionData);
    }

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsFlexNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(IsFlex());
      return ValueNoConversionDataRequired<T>();
    }

#pragma endregion

#pragma region Length (requires IsLength())

    /// @note Specialised for int64, uint64, int32, uint32, int16, uint16, float and double in
    /// PrimitiveValue.cpp.
    template <typename T = double>
    KRYS_NODISCARD T ResolveAsLength(const ToLengthConversionData &conversionData) const noexcept;

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsLengthNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(IsLength());
      return ValueNoConversionDataRequired<T>(UnitType::px);
    }

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsLengthDeprecated() const noexcept
    {
      krys_debug_assert(IsLength());
      return ValueDeprecated<T>(UnitType::px);
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

    template <typename T = double>
    KRYS_NODISCARD T ValueDividingBy100IfPercentageNoConversionDataRequired() const noexcept
    {
      return ClampTo<T>(DoubleValueDividingBy100IfPercentageNoConversionDataRequired());
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueDividingBy100IfPercentageDeprecated() const noexcept
    {
      return ClampTo<T>(DoubleValueDividingBy100IfPercentageDeprecated());
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

    KRYS_NODISCARD const AttrValue *CSSAttrValue() const noexcept
    {
      return IsAttr() ? _value.Attr : nullptr;
    }

    KRYS_NODISCARD RefPtr<const AttrValue> ProtectedCSSAttrValue() const noexcept
    {
      return ShareRefPtr(CSSAttrValue());
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const PrimitiveValue &other) const noexcept;

    KRYS_NODISCARD constexpr static CSSOMStringView UnitTypeString(UnitType unitType) noexcept;

    void CollectComputedStyleDependencies(css::ComputedStyleDependencies &dependencies) const noexcept;

  private:
    KRYS_NODISCARD UnitType PrimitiveUnitType() const noexcept
    {
      return _unit;
    }

    void SetPrimitiveUnitType(UnitType type) noexcept
    {
      _unit = type;
    }

#pragma region Length converting

    KRYS_NODISCARD double ResolveAsLengthDouble(const ToLengthConversionData &conversionData) const noexcept;

#pragma endregion

#pragma region Arbitrarily converting

    KRYS_NODISCARD double DoubleValue(UnitType targetUnit, const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD double DoubleValueNoConversionDataRequired(UnitType targetUnit) const noexcept;
    KRYS_NODISCARD double DoubleValueDeprecated(UnitType targetUnit) const noexcept;

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

    template <typename T = double>
    KRYS_NODISCARD T ValueDeprecated(UnitType targetUnit) const noexcept
    {
      return ClampTo<T>(DoubleValueDeprecated(targetUnit));
    }

#pragma endregion

#pragma region Non-converting

    KRYS_NODISCARD double DoubleValue(const ToLengthConversionData &) const noexcept;

    KRYS_NODISCARD double DoubleValueNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(!IsCalculated());
      return _value.Number;
    }

    KRYS_NODISCARD double DoubleValueDeprecated() const noexcept;
    KRYS_NODISCARD double DoubleValueDividingBy100IfPercentage(const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD double DoubleValueDividingBy100IfPercentageNoConversionDataRequired() const noexcept;
    KRYS_NODISCARD double DoubleValueDividingBy100IfPercentageDeprecated() const noexcept;

    template <typename T = double>
    KRYS_NODISCARD T ValueDeprecated() const noexcept
    {
      return ClampTo<T>(DoubleValueDeprecated());
    }

#pragma endregion

    KRYS_NODISCARD constexpr static Maybe<double>
      ConversionToCanonicalUnitsScaleFactor(UnitType unitType) noexcept;

    KRYS_NODISCARD Maybe<double> DoubleValueInternal(UnitType targetUnit,
                                                     const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD Maybe<double> DoubleValueInternalDeprecated(UnitType targetUnit) const noexcept;

    KRYS_NODISCARD bool AddDerivedHash(size_t &seed) const noexcept;

    KRYS_NODISCARD CSSOMString SerialiseInternal(const SerialisationContext &) const noexcept;
    KRYS_NODISCARD CSSOMString FormatNumberValue(CSSOMStringView suffix) const noexcept;
    KRYS_NODISCARD CSSOMString FormatIntegerValue(CSSOMStringView suffix) const noexcept;

  public:
    KRYS_NODISCARD constexpr static bool IsFontIndependentLength(UnitType type) noexcept
    {
      return type == UnitType::px || type == UnitType::cm || type == UnitType::mm || type == UnitType::in
             || type == UnitType::pt || type == UnitType::pc;
    }

    KRYS_NODISCARD constexpr static bool IsRootFontRelativeLength(UnitType type) noexcept
    {
      return type == UnitType::rcap || type == UnitType::rch || type == UnitType::rem || type == UnitType::rex
             || type == UnitType::ric || type == UnitType::rlh;
    }

    KRYS_NODISCARD constexpr static bool IsFontRelativeLength(UnitType type) noexcept
    {
      return type == UnitType::em || type == UnitType::ex || type == UnitType::lh || type == UnitType::cap
             || type == UnitType::ch || type == UnitType::ic || type == UnitType::QuirkyEm
             || IsRootFontRelativeLength(type);
    }

    KRYS_NODISCARD constexpr static bool IsContainerPercentageLength(UnitType type) noexcept
    {
      return type == UnitType::cqw || type == UnitType::cqh || type == UnitType::cqi || type == UnitType::cqb
             || type == UnitType::cqmin || type == UnitType::cqmax;
    }

    KRYS_NODISCARD constexpr static bool IsViewportPercentageLength(UnitType type) noexcept
    {
      return type >= UnitType::FirstViewportCSSUnitType && type <= UnitType::LastViewportCSSUnitType;
    }
  };

  constexpr CSSOMStringView PrimitiveValue::UnitTypeString(UnitType unitType) noexcept
  {
    switch (unitType)
    {
      case UnitType::cap:                      return u8"cap";
      case UnitType::ch:                       return u8"ch";
      case UnitType::cm:                       return u8"cm";
      case UnitType::cqb:                      return u8"cqb";
      case UnitType::cqh:                      return u8"cqh";
      case UnitType::cqi:                      return u8"cqi";
      case UnitType::cqmax:                    return u8"cqmax";
      case UnitType::cqmin:                    return u8"cqmin";
      case UnitType::cqw:                      return u8"cqw";
      case UnitType::deg:                      return u8"deg";
      case UnitType::dpcm:                     return u8"dpcm";
      case UnitType::dpi:                      return u8"dpi";
      case UnitType::dppx:                     return u8"dppx";
      case UnitType::dvb:                      return u8"dvb";
      case UnitType::dvh:                      return u8"dvh";
      case UnitType::dvi:                      return u8"dvi";
      case UnitType::dvmax:                    return u8"dvmax";
      case UnitType::dvmin:                    return u8"dvmin";
      case UnitType::dvw:                      return u8"dvw";
      case UnitType::em:                       return u8"em";
      case UnitType::ex:                       return u8"ex";
      case UnitType::fr:                       return u8"fr";
      case UnitType::grad:                     return u8"grad";
      case UnitType::hz:                       return u8"hz";
      case UnitType::ic:                       return u8"ic";
      case UnitType::in:                       return u8"in";
      case UnitType::khz:                      return u8"khz";
      case UnitType::lh:                       return u8"lh";
      case UnitType::lvb:                      return u8"lvb";
      case UnitType::lvh:                      return u8"lvh";
      case UnitType::lvi:                      return u8"lvi";
      case UnitType::lvmax:                    return u8"lvmax";
      case UnitType::lvmin:                    return u8"lvmin";
      case UnitType::lvw:                      return u8"lvw";
      case UnitType::mm:                       return u8"mm";
      case UnitType::ms:                       return u8"ms";
      case UnitType::pc:                       return u8"pc";
      case UnitType::Percentage:               return u8"%";
      case UnitType::pt:                       return u8"pt";
      case UnitType::px:                       return u8"px";
      case UnitType::Q:                        return u8"q";
      case UnitType::rad:                      return u8"rad";
      case UnitType::rcap:                     return u8"rcap";
      case UnitType::rch:                      return u8"rch";
      case UnitType::rem:                      return u8"rem";
      case UnitType::rex:                      return u8"rex";
      case UnitType::ric:                      return u8"ric";
      case UnitType::rlh:                      return u8"rlh";
      case UnitType::s:                        return u8"s";
      case UnitType::svb:                      return u8"svb";
      case UnitType::svh:                      return u8"svh";
      case UnitType::svi:                      return u8"svi";
      case UnitType::svmax:                    return u8"svmax";
      case UnitType::svmin:                    return u8"svmin";
      case UnitType::svw:                      return u8"svw";
      case UnitType::turn:                     return u8"turn";
      case UnitType::vb:                       return u8"vb";
      case UnitType::vh:                       return u8"vh";
      case UnitType::vi:                       return u8"vi";
      case UnitType::vmax:                     return u8"vmax";
      case UnitType::vmin:                     return u8"vmin";
      case UnitType::vw:                       return u8"vw";
      case UnitType::x:                        return u8"x";

      case UnitType::Attr:
      case UnitType::Calc:
      case UnitType::CalcPercentageWithAngle:
      case UnitType::CalcPercentageWithLength:
      case UnitType::Dimension:
      case UnitType::FontFamily:
      case UnitType::Ident:
      case UnitType::Integer:
      case UnitType::Number:
      case UnitType::PropertyId:
      case UnitType::QuirkyEm:
      case UnitType::String:
      case UnitType::Unknown:
      case UnitType::ValueId:
      case UnitType::CustomIdent:              return u8"";
    }

    krys_debug_assert(false);
    return u8"";
  }

  constexpr Maybe<double> PrimitiveValue::ConversionToCanonicalUnitsScaleFactor(UnitType unitType) noexcept
  {
    // FIXME: the switch can be replaced by an array of scale factors.
    switch (unitType)
    {
      // These are "canonical" units in their respective categories.
      case UnitType::px:
      case UnitType::deg:
      case UnitType::s:
      case UnitType::hz:
      case UnitType::dppx: return 1.0;

      case UnitType::x:    return dppxPerX;
      case UnitType::cm:   return pxPerCm;
      case UnitType::dpcm: return dppxPerDpcm;
      case UnitType::mm:   return pxPerMm;
      case UnitType::Q:    return pxPerQ;
      case UnitType::in:   return pxPerInch;
      case UnitType::dpi:  return dppxPerDpi;
      case UnitType::pt:   return pxPerPt;
      case UnitType::pc:   return pxPerPc;
      case UnitType::rad:  return degreesPerRadianDouble;
      case UnitType::grad: return degreesPerGradientDouble;
      case UnitType::turn: return degreesPerTurnDouble;
      case UnitType::ms:   return secsPerMs;
      case UnitType::khz:  return hertzPerKilohertz;

      default:             return null;
    }
  }

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
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PrimitiveValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsPrimitiveValue();
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

  inline int64 Value::IntegerDeprecated() const noexcept
  {
    krys_debug_assert(IsInteger());
    return Downcast<PrimitiveValue>(*this).ResolveAsIntegerDeprecated();
  }

#pragma endregion
}
