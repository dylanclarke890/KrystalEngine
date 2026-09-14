#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/AttrValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css::calc
{
  class Value;
}

namespace krys::boo::css
{
  class CSSToLengthConversionData;

  class PrimitiveValue : public Value
  {
    friend class StaticCSSValuePool;
    friend LazyNeverDestroyed<PrimitiveValue>;

  private:
    union ValueUnion
    {
      PropertyId PropertyId;
      css::ValueId ValueId;
      const AttrValue *Attr;
      const calc::Value *Calc;
      double Number;
      StringAtomStorage String;
    } _value;

    struct CreateImplicitInitialValueTag
    {
    };

    constexpr static CreateImplicitInitialValueTag CreateImplicitInitialValue {};

#pragma region Constructors

    explicit PrimitiveValue(css::ValueId identifier) noexcept;

    explicit PrimitiveValue(PropertyId property) noexcept;

    explicit PrimitiveValue(Ref<AttrValue> attr) noexcept;

    explicit PrimitiveValue(Ref<calc::Value> calc) noexcept;

    PrimitiveValue(CSSOMString value, UnitType unit) noexcept;

    PrimitiveValue(double value, UnitType unit) noexcept;

    PrimitiveValue(StaticCSSValueTag, CreateImplicitInitialValueTag) noexcept;

    PrimitiveValue(StaticCSSValueTag, css::ValueId keyword) noexcept;

    PrimitiveValue(StaticCSSValueTag, double value, UnitType unit) noexcept;

#pragma endregion

  public:
    ~PrimitiveValue() noexcept = default;

#pragma region Static Creation

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(css::ValueId identifier) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(PropertyId property) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(const CSSOMString &value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(const CSSOMStringAtom &value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(Ref<AttrValue> value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(Ref<calc::Value> value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> CreateCustomIdent(const CSSOMString &value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> CreateCustomIdent(const CSSOMStringAtom &value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(double value) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> Create(double value, UnitType unit) noexcept;

    KRYS_NODISCARD static Ref<PrimitiveValue> CreateInteger(double value) noexcept;

    KRYS_NODISCARD static PrimitiveValue &ImplicitInitialValue() noexcept;

#pragma endregion

#pragma region Type/Unit Checks

    KRYS_NODISCARD bool IsPropertyId() const noexcept
    {
      return _unit == UnitType::PropertyId;
    }

    KRYS_NODISCARD bool IsValueId() const noexcept
    {
      return _unit == UnitType::ValueId;
    }

    KRYS_NODISCARD bool IsAttr() const noexcept
    {
      return _unit == UnitType::Attr;
    }

    KRYS_NODISCARD bool IsNumber() const noexcept
    {
      return _unit == UnitType::Number;
    }

    KRYS_NODISCARD bool IsInteger() const noexcept
    {
      return _unit == UnitType::Integer;
    }

    KRYS_NODISCARD bool IsPercentage() const noexcept
    {
      return _unit == UnitType::Percentage;
    }

    KRYS_NODISCARD bool IsString() const noexcept
    {
      return _unit == UnitType::String;
    }

    KRYS_NODISCARD bool IsCustomIdent() const noexcept
    {
      return _unit == UnitType::CustomIdent;
    }

    KRYS_NODISCARD bool IsCalculated() const noexcept
    {
      return _unit == UnitType::Calc;
    }

    KRYS_NODISCARD bool IsCalculatedPercentageWithAngle() const noexcept
    {
      return _unit == UnitType::CalcPercentageWithAngle;
    }

    KRYS_NODISCARD bool IsCalculatedPercentageWithLength() const noexcept
    {
      return _unit == UnitType::CalcPercentageWithLength;
    }

    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return IsLength(_unit);
    }

    KRYS_NODISCARD Maybe<bool> IsZero() const noexcept
    {
      if (IsCalculated())
      {
        return null;
      }

      return !_value.Number;
    }

    KRYS_NODISCARD Maybe<bool> IsNegative() const noexcept
    {
      if (IsCalculated())
      {
        return null;
      }

      return _value.Number < 0.0;
    }

    KRYS_NODISCARD bool IsFlex() const noexcept
    {
      return _unit == UnitType::fr;
    }

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

    KRYS_NODISCARD constexpr static bool IsLength(UnitType type) noexcept
    {
      return type == UnitType::em || type == UnitType::ex || type == UnitType::px || type == UnitType::cm
             || type == UnitType::mm || type == UnitType::in || type == UnitType::pt || type == UnitType::pc
             || type == UnitType::Q || IsFontRelativeLength(type) || IsViewportPercentageLength(type)
             || IsContainerPercentageLength(type) || type == UnitType::QuirkyEm;
    }

    KRYS_NODISCARD constexpr static bool IsViewportPercentageLength(UnitType type) noexcept
    {
      return type >= UnitType::FirstViewportCSSUnitType && type <= UnitType::LastViewportCSSUnitType;
    }

#pragma endregion

    KRYS_NODISCARD double DoubleValue(UnitType targetUnit, const CSSToLengthConversionData &) const noexcept;

    KRYS_NODISCARD double DoubleValueDeprecated() const noexcept
    {
      // TODO
      // if (RefPtr calcValue = CSSCalcValue())
      //   return calcValue->DoubleValueDeprecated();
      return _value.Number;
    }

    KRYS_NODISCARD double DoubleValueNoConversionDataRequired(UnitType targetUnit) const noexcept;

    KRYS_NODISCARD double DoubleValueNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(!IsCalculated());
      return _value.Number;
    }

    template <typename T = double>
    KRYS_NODISCARD T Value(UnitType targetUnit,
                           const CSSToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(DoubleValue(targetUnit, conversionData));
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueDeprecated() const noexcept
    {
      return ClampTo<T>(DoubleValueDeprecated());
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueDeprecated(UnitType targetUnit) const noexcept
    {
      return ClampTo<T>(DoubleValueDeprecated(targetUnit));
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueNoConversionDataRequired() const noexcept
    {
      return ClampTo<T>(DoubleValueNoConversionDataRequired());
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueNoConversionDataRequired(UnitType targetUnit) const noexcept
    {
      return ClampTo<T>(DoubleValueNoConversionDataRequired(targetUnit));
    }

    template <typename T = double>
    KRYS_NODISCARD Maybe<T> ResolveAsNumberIfNotCalculated() const noexcept
    {
      if (IsCalculated())
      {
        return null;
      }

      return _value.Number;
    }

    template <typename T = int64>
    KRYS_NODISCARD Maybe<T> ResolveAsIntegerIfNotCalculated() const noexcept
    {
      if (IsCalculated())
      {
        return null;
      }

      return static_cast<T>(_value.Number);
    }

    template <typename T = int64>
    KRYS_NODISCARD T ResolveAsIntegerDeprecated() const noexcept
    {
      krys_debug_assert(IsInteger());
      return ValueDeprecated<T>();
    }

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsPercentageNoConversionDataRequired() const noexcept
    {
      krys_debug_assert(IsPercentage());
      return ValueNoConversionDataRequired<T>();
    }

    KRYS_NODISCARD CSSOMString StringValue() const noexcept
    {
      switch (_unit)
      {
        case UnitType::String:
        case UnitType::CustomIdent:
        case UnitType::FontFamily:
        {
          return *_value.String;
        }
        case UnitType::ValueId:
        {
          return CSSOMString(ToString(_value.ValueId));
        }
        case UnitType::PropertyId:
        {
          return CSSOMString(ToString(_value.PropertyId));
        }
        // TODO:
        // case UnitType::Attr:
        // {
        //   return protectedCssAttrValue()->cssText(CSS::defaultSerializationContext());
        // }
        default:
        {
          return {};
        }
      }
    }

    KRYS_NODISCARD CSSOMString CustomIdent() const noexcept
    {
      krys_debug_assert(IsCustomIdent());
      return *_value.String;
    }
  };

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
    return value ? IsValueId(*value, id) : false;
  }

  KRYS_NODISCARD inline bool IsValueId(const RefPtr<PrimitiveValue> &value, ValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsValueId(const Ref<PrimitiveValue> &value, ValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsValueId(const Value &value, ValueId id) noexcept
  {
    return GetValueId(value) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const Value *value, ValueId id) noexcept
  {
    return value ? IsValueId(*value, id) : false;
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
    return primitiveValue != nullptr && primitiveValue->IsCustomIdent();
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PrimitiveValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsPrimitiveValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()