#pragma once

#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Lib/NeverDestroyed.hpp"

namespace Krys::HTML
{
  class CSSPrimitiveValue : public CSSValue
  {
    friend class StaticCSSValuePool;
    friend LazyNeverDestroyed<CSSPrimitiveValue>;

  private:
    union ValueUnion
    {
      CSSPropertyId PropertyId;
      CSSValueId ValueId;
      double Number;
      StringAtomStorage String;
    } _value;

    struct CreateImplicitInitialValueTag
    {
    };

    constexpr inline static CreateImplicitInitialValueTag CreateImplicitInitialValue {};

    explicit CSSPrimitiveValue(CSSValueId identifier) noexcept;

    explicit CSSPrimitiveValue(CSSPropertyId property) noexcept;

    CSSPrimitiveValue(CSSOMString value, CSSUnitType unit) noexcept;

    CSSPrimitiveValue(double value, CSSUnitType unit) noexcept;

    CSSPrimitiveValue(StaticCSSValueTag, CreateImplicitInitialValueTag) noexcept;

    CSSPrimitiveValue(StaticCSSValueTag, CSSValueId keyword) noexcept;

    CSSPrimitiveValue(StaticCSSValueTag, double value, CSSUnitType unit) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(CSSValueId identifier) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(CSSPropertyId property) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(CSSOMString value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> CreateCustomIdent(CSSOMString value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(double value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(double value, CSSUnitType unit) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> CreateInteger(double value) noexcept;

    KRYS_NODISCARD static CSSPrimitiveValue &ImplicitInitialValue() noexcept;

    KRYS_NODISCARD bool IsPropertyId() const noexcept
    {
      return _unit == CSSUnitType::PropertyId;
    }

    KRYS_NODISCARD bool IsValueId() const noexcept
    {
      return _unit == CSSUnitType::ValueId;
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
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSPrimitiveValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSPrimitiveValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()