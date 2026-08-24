#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePool.hpp"

namespace Krys::HTML
{
  CSSPrimitiveValue::CSSPrimitiveValue(CSSValueId identifier) noexcept : CSSValue(CSSValueType::Primitive)
  {
    _unit = CSSUnitType::ValueId;
    _value.ValueId = identifier;
  }

  CSSPrimitiveValue::CSSPrimitiveValue(CSSPropertyId property) : CSSValue(CSSValueType::Primitive)
  {
    _unit = CSSUnitType::PropertyId;
    _value.PropertyId = property;
  }

  CSSPrimitiveValue::CSSPrimitiveValue(CSSOMString value, CSSUnitType unit) noexcept
      : CSSValue(CSSValueType::Primitive)
  {
    _unit = unit;

    auto atom = CSSOMStringAtom(value);
    _value.String = *reinterpret_cast<const StringAtomStorage *>(atom.View().data());
  }

  CSSPrimitiveValue::CSSPrimitiveValue(double value, CSSUnitType unit) noexcept
      : CSSValue(CSSValueType::Primitive)
  {
    _unit = unit;
    _value.Number = value;
  }

  CSSPrimitiveValue::CSSPrimitiveValue(StaticCSSValueTag, CreateImplicitInitialValueTag) noexcept
      : CSSPrimitiveValue(StaticCSSValue, CSSValueId::Initial)
  {
    _isImplicitInitialValue = true;
  }

  CSSPrimitiveValue::CSSPrimitiveValue(StaticCSSValueTag, CSSValueId keyword) noexcept
      : CSSValue(CSSValueType::Primitive)
  {
    _unit = CSSUnitType::ValueId;
    _value.ValueId = keyword;

    // TODO: we should handle this like webkit (by having ref count increment by 2 so a value of 1 is reserved
    // for static values that will not be destroyed) . For now, we will just increase the ref count by 1.
    AddRef();
  }

  CSSPrimitiveValue::CSSPrimitiveValue(StaticCSSValueTag, double number, CSSUnitType type) noexcept
      : CSSPrimitiveValue(number, type)
  {
    // TODO: we should handle this like webkit (by having ref count increment by 2 so a value of 1 is reserved
    // for static values that will not be destroyed) . For now, we will just increase the ref count by 1.
    AddRef();
  }

  KRYS_NODISCARD static CSSPrimitiveValue *ValueFromPool(Span<AlignedStorage<CSSPrimitiveValue>> pool,
                                                         double value) noexcept
  {
    // Casting to a signed integer first since casting a negative floating point value to an unsigned
    // integer is undefined behavior.
    size_t poolIndex = static_cast<size_t>(static_cast<int64>(value));
    double roundTripValue = poolIndex;

    if (std::ranges::equal(Span<const byte>(reinterpret_cast<const byte *>(&value), sizeof(double)),
                           Span<const byte>(reinterpret_cast<const byte *>(&roundTripValue), sizeof(double)))
        && poolIndex < pool.size())
    {
      return pool[poolIndex].get();
    }

    return nullptr;
  }

  Ref<CSSPrimitiveValue> CSSPrimitiveValue::Create(CSSPropertyId property) noexcept
  {
    return AdoptRef(*new CSSPrimitiveValue(property));
  }

  Ref<CSSPrimitiveValue> CSSPrimitiveValue::Create(double value) noexcept
  {
    if (auto* result = ValueFromPool(CommonCSSValuePool->_numberValues, value))
    {
      return ShareRef(*result);
    }

    return AdoptRef(*new CSSPrimitiveValue(value, CSSUnitType::Number));
  }

  Ref<CSSPrimitiveValue> CSSPrimitiveValue::Create(double value, CSSUnitType type) noexcept
  {
    switch (type)
    {
      case CSSUnitType::Number:
      {
        if (auto *result = ValueFromPool(CommonCSSValuePool->_numberValues, value))
        {
          return ShareRef(*result);
        }
        break;
      }
      case CSSUnitType::Percentage:
      {
        if (auto *result = ValueFromPool(CommonCSSValuePool->_percentageValues, value))
        {
          return ShareRef(*result);
        }
        break;
      }
      case CSSUnitType::px:
      {
        if (auto *result = ValueFromPool(CommonCSSValuePool->_pixelValues, value))
        {
          return ShareRef(*result);
        }
        break;
      }
      default:
      {
        break;
      }
    }
    return AdoptRef(*new CSSPrimitiveValue(value, type));
  }

  Ref<CSSPrimitiveValue> CSSPrimitiveValue::Create(const CSSOMString &value) noexcept
  {
    return AdoptRef(*new CSSPrimitiveValue(value, CSSUnitType::String));
  }

  Ref<CSSPrimitiveValue> CSSPrimitiveValue::Create(const CSSOMStringAtom &value) noexcept
  {
    return AdoptRef(*new CSSPrimitiveValue(CSSOMString(value.View()), CSSUnitType::String));
  }

  Ref<CSSPrimitiveValue> CSSPrimitiveValue::CreateCustomIdent(const CSSOMString &value) noexcept
  {
    return AdoptRef(*new CSSPrimitiveValue(value, CSSUnitType::CustomIdent));
  }

  Ref<CSSPrimitiveValue> CSSPrimitiveValue::CreateCustomIdent(const CSSOMStringAtom &value) noexcept
  {
    return AdoptRef(*new CSSPrimitiveValue(CSSOMString(value.View()), CSSUnitType::CustomIdent));
  }

  Ref<CSSPrimitiveValue> CSSPrimitiveValue::CreateInteger(double value) noexcept
  {
    return AdoptRef(*new CSSPrimitiveValue(value, CSSUnitType::Integer));
  }
}