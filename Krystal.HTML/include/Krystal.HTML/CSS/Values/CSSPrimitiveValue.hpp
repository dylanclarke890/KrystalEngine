#pragma once

#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Lib/NeverDestroyed.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

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

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(const CSSOMString &value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> Create(const CSSOMStringAtom &value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> CreateCustomIdent(const CSSOMString &value) noexcept;

    KRYS_NODISCARD static Ref<CSSPrimitiveValue> CreateCustomIdent(const CSSOMStringAtom &value) noexcept;

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

    KRYS_NODISCARD bool IsCustomIdent() const noexcept
    {
      return _unit == CSSUnitType::CustomIdent;
    }
  };

  KRYS_NODISCARD inline CSSValueId ValueId(const CSSPrimitiveValue &value) noexcept
  {
    return value.ValueId();
  }

  KRYS_NODISCARD inline CSSValueId ValueId(const CSSPrimitiveValue *value) noexcept
  {
    return value ? ValueId(*value) : CSSValueId::Invalid;
  }

  KRYS_NODISCARD inline CSSValueId ValueId(const CSSValue &value) noexcept
  {
    auto *primitiveValue = DynamicDowncast<CSSPrimitiveValue>(value);
    return primitiveValue ? ValueId(*primitiveValue) : CSSValueId::Invalid;
  }

  KRYS_NODISCARD inline CSSValueId ValueId(const CSSValue *value) noexcept
  {
    return value ? ValueId(*value) : CSSValueId::Invalid;
  }

  KRYS_NODISCARD inline bool IsValueId(const CSSPrimitiveValue &value, CSSValueId id) noexcept
  {
    return ValueId(value) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const CSSPrimitiveValue *value, CSSValueId id) noexcept
  {
    return value ? IsValueId(*value, id) : false;
  }

  KRYS_NODISCARD inline bool IsValueId(const RefPtr<CSSPrimitiveValue> &value, CSSValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsValueId(const Ref<CSSPrimitiveValue> &value, CSSValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsValueId(const CSSValue &value, CSSValueId id) noexcept
  {
    return ValueId(value) == id;
  }

  KRYS_NODISCARD inline bool IsValueId(const CSSValue *value, CSSValueId id) noexcept
  {
    return value ? IsValueId(*value, id) : false;
  }

  KRYS_NODISCARD inline bool IsValueId(const RefPtr<CSSValue> &value, CSSValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsValueId(const Ref<CSSValue> &value, CSSValueId id) noexcept
  {
    return IsValueId(value.get(), id);
  }

  KRYS_NODISCARD inline bool IsCustomIdentValue(const CSSValue &value) noexcept
  {
    auto *primitiveValue = DynamicDowncast<CSSPrimitiveValue>(value);
    return primitiveValue && primitiveValue->IsCustomIdent();
  }

}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSPrimitiveValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSPrimitiveValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()