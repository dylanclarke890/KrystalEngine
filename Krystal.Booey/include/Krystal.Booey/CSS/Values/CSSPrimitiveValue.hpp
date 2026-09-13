#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/CSSAttrValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css
{
  class CSSToLengthConversionData;

  class CSSPrimitiveValue : public CSSValue
  {
    friend class StaticCSSValuePool;
    friend LazyNeverDestroyed<CSSPrimitiveValue>;

  private:
    union ValueUnion
    {
      PropertyId PropertyId;
      css::ValueId ValueId;
      const CSSAttrValue *Attr;
      double Number;
      StringAtomStorage String;
    } _value;

    struct CreateImplicitInitialValueTag
    {
    };

    constexpr static CreateImplicitInitialValueTag CreateImplicitInitialValue {};

#pragma region Constructors

    explicit CSSPrimitiveValue(css::ValueId identifier) noexcept;

    explicit CSSPrimitiveValue(PropertyId property) noexcept;

    explicit CSSPrimitiveValue(Ref<CSSAttrValue> attr) noexcept;

    CSSPrimitiveValue(CSSOMString value, CSSUnitType unit) noexcept;

    CSSPrimitiveValue(double value, CSSUnitType unit) noexcept;

    CSSPrimitiveValue(StaticCSSValueTag, CreateImplicitInitialValueTag) noexcept;

    CSSPrimitiveValue(StaticCSSValueTag, css::ValueId keyword) noexcept;

    CSSPrimitiveValue(StaticCSSValueTag, double value, CSSUnitType unit) noexcept;

#pragma endregion

  public:
#pragma region Static Creation

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(css::ValueId identifier) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(PropertyId property) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(const CSSOMString &value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(const CSSOMStringAtom &value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(Ref<CSSAttrValue> value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> CreateCustomIdent(const CSSOMString &value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> CreateCustomIdent(const CSSOMStringAtom &value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(double value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(double value, CSSUnitType unit) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> CreateInteger(double value) noexcept;

    KRYS_NODISCARD static CSSPrimitiveValue &ImplicitInitialValue() noexcept;

#pragma endregion

#pragma region Type/Unit Checks

    KRYS_NODISCARD bool IsPropertyId() const noexcept
    {
      return _unit == CSSUnitType::PropertyId;
    }

    KRYS_NODISCARD bool IsValueId() const noexcept
    {
      return _unit == CSSUnitType::ValueId;
    }

    KRYS_NODISCARD bool IsAttr() const noexcept
    {
      return _unit == CSSUnitType::Attr;
    }

    KRYS_NODISCARD bool IsNumber() const noexcept
    {
      return _unit == CSSUnitType::Number;
    }

    KRYS_NODISCARD bool IsInteger() const noexcept
    {
      return _unit == CSSUnitType::Integer;
    }

    KRYS_NODISCARD bool IsPercentage() const noexcept
    {
      return _unit == CSSUnitType::Percentage;
    }

    KRYS_NODISCARD bool IsString() const noexcept
    {
      return _unit == CSSUnitType::String;
    }

    KRYS_NODISCARD bool IsCustomIdent() const noexcept
    {
      return _unit == CSSUnitType::CustomIdent;
    }

    KRYS_NODISCARD bool IsCalculated() const noexcept
    {
      return _unit == CSSUnitType::Calc;
    }

    KRYS_NODISCARD bool IsCalculatedPercentageWithAngle() const noexcept
    {
      return _unit == CSSUnitType::CalcPercentageWithAngle;
    }

    KRYS_NODISCARD bool IsCalculatedPercentageWithLength() const noexcept
    {
      return _unit == CSSUnitType::CalcPercentageWithLength;
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
      return _unit == CSSUnitType::fr;
    }

    KRYS_NODISCARD constexpr static bool IsFontIndependentLength(CSSUnitType type) noexcept
    {
      return type == CSSUnitType::px || type == CSSUnitType::cm || type == CSSUnitType::mm
             || type == CSSUnitType::in || type == CSSUnitType::pt || type == CSSUnitType::pc;
    }

    KRYS_NODISCARD constexpr static bool IsRootFontRelativeLength(CSSUnitType type) noexcept
    {
      return type == CSSUnitType::rcap || type == CSSUnitType::rch || type == CSSUnitType::rem
             || type == CSSUnitType::rex || type == CSSUnitType::ric || type == CSSUnitType::rlh;
    }

    KRYS_NODISCARD constexpr static bool IsFontRelativeLength(CSSUnitType type) noexcept
    {
      return type == CSSUnitType::em || type == CSSUnitType::ex || type == CSSUnitType::lh
             || type == CSSUnitType::cap || type == CSSUnitType::ch || type == CSSUnitType::ic
             || type == CSSUnitType::QuirkyEm || IsRootFontRelativeLength(type);
    }

    KRYS_NODISCARD constexpr static bool IsContainerPercentageLength(CSSUnitType type) noexcept
    {
      return type == CSSUnitType::cqw || type == CSSUnitType::cqh || type == CSSUnitType::cqi
             || type == CSSUnitType::cqb || type == CSSUnitType::cqmin || type == CSSUnitType::cqmax;
    }

    KRYS_NODISCARD constexpr static bool IsLength(CSSUnitType type) noexcept
    {
      return type == CSSUnitType::em || type == CSSUnitType::ex || type == CSSUnitType::px
             || type == CSSUnitType::cm || type == CSSUnitType::mm || type == CSSUnitType::in
             || type == CSSUnitType::pt || type == CSSUnitType::pc || type == CSSUnitType::Q
             || IsFontRelativeLength(type) || IsViewportPercentageLength(type)
             || IsContainerPercentageLength(type) || type == CSSUnitType::QuirkyEm;
    }

    KRYS_NODISCARD constexpr static bool IsViewportPercentageLength(CSSUnitType type) noexcept
    {
      return type >= CSSUnitType::FirstViewportCSSUnitType && type <= CSSUnitType::LastViewportCSSUnitType;
    }

#pragma endregion

    template <typename T = double>
    KRYS_NODISCARD T Value(CSSUnitType targetUnit,
                           const CSSToLengthConversionData &conversionData) const noexcept
    {
      return ClampTo<T>(DoubleValue(targetUnit, conversionData));
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueNoConversionDataRequired() const noexcept
    {
      return ClampTo<T>(DoubleValueNoConversionDataRequired());
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueNoConversionDataRequired(CSSUnitType targetUnit) const noexcept
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
      assert(IsInteger());
      return ValueDeprecated<T>();
    }

    template <typename T = double>
    KRYS_NODISCARD T ResolveAsPercentageNoConversionDataRequired() const noexcept
    {
      assert(IsPercentage());
      return ValueNoConversionDataRequired<T>();
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueDeprecated() const noexcept
    {
      return ClampTo<T>(DoubleValueDeprecated());
    }

    template <typename T = double>
    KRYS_NODISCARD T ValueDeprecated(CSSUnitType targetUnit) const noexcept
    {
      return ClampTo<T>(DoubleValueDeprecated(targetUnit));
    }

    KRYS_NODISCARD double DoubleValue(CSSUnitType targetUnit,
                                      const CSSToLengthConversionData &) const noexcept;

    KRYS_NODISCARD double DoubleValueNoConversionDataRequired(CSSUnitType targetUnit) const noexcept;

    KRYS_NODISCARD double DoubleValueNoConversionDataRequired() const noexcept
    {
      assert(!IsCalculated());
      return _value.Number;
    }

    KRYS_NODISCARD double DoubleValueDeprecated() const noexcept
    {
      // TODO
      // if (RefPtr calcValue = CSSCalcValue())
      //   return calcValue->DoubleValueDeprecated();
      return _value.Number;
    }

    KRYS_NODISCARD CSSOMString StringValue() const noexcept
    {
      switch (_unit)
      {
        case CSSUnitType::String:
        case CSSUnitType::CustomIdent:
        case CSSUnitType::FontFamily:
        {
          return *_value.String;
        }
        case CSSUnitType::ValueId:
        {
          return CSSOMString(ToString(_value.ValueId));
        }
        case CSSUnitType::PropertyId:
        {
          return CSSOMString(ToString(_value.PropertyId));
        }
        // TODO:
        // case CSSUnitType::Attr:
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
      assert(IsCustomIdent());
      return *_value.String;
    }
  };

  KRYS_NODISCARD inline ValueId GetValueId(const CSSPrimitiveValue &value) noexcept
  {
    return value.ValueId();
  }

  KRYS_NODISCARD inline ValueId GetValueId(const CSSPrimitiveValue *value) noexcept
  {
    return value ? GetValueId(*value) : ValueId::Invalid;
  }

  KRYS_NODISCARD inline ValueId GetValueId(const CSSValue &value) noexcept
  {
    auto *primitiveValue = DynamicDowncast<CSSPrimitiveValue>(value);
    return primitiveValue ? GetValueId(*primitiveValue) : ValueId::Invalid;
  }

  KRYS_NODISCARD inline ValueId GetValueId(const CSSValue *value) noexcept
  {
    return value ? GetValueId(*value) : ValueId::Invalid;
  }

  KRYS_NODISCARD inline bool IsValueId(const CSSPrimitiveValue &value, ValueId id) noexcept
  {
    return GetValueId(value) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const CSSPrimitiveValue *value, ValueId id) noexcept
  {
    return value ? IsValueId(*value, id) : false;
  }

  KRYS_NODISCARD inline bool IsValueId(const RefPtr<CSSPrimitiveValue> &value, ValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsValueId(const Ref<CSSPrimitiveValue> &value, ValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsValueId(const CSSValue &value, ValueId id) noexcept
  {
    return GetValueId(value) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const CSSValue *value, ValueId id) noexcept
  {
    return value ? IsValueId(*value, id) : false;
  }

  KRYS_NODISCARD inline bool IsValueId(const RefPtr<CSSValue> &value, ValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsValueId(const Ref<CSSValue> &value, ValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsCustomIdentValue(const CSSValue &value) noexcept
  {
    auto *primitiveValue = DynamicDowncast<CSSPrimitiveValue>(value);
    return primitiveValue != nullptr && primitiveValue->IsCustomIdent();
  }

}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSPrimitiveValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSPrimitiveValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()