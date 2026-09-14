#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include <algorithm>

namespace krys::boo::css
{
  PrimitiveValue::PrimitiveValue(css::ValueId identifier) noexcept : css::Value(ValueType::Primitive)
  {
    _unit = UnitType::ValueId;
    _value.ValueId = identifier;
  }

  PrimitiveValue::PrimitiveValue(PropertyId property) noexcept : css::Value(ValueType::Primitive)
  {
    _unit = UnitType::PropertyId;
    _value.PropertyId = property;
  }

  PrimitiveValue::PrimitiveValue(Ref<AttrValue> attr) noexcept : css::Value(ValueType::Primitive)
  {
    _unit = UnitType::Attr;
    _value.Attr = attr.release();
  }

  PrimitiveValue::PrimitiveValue(Ref<calc::Value> calc) noexcept : css::Value(ValueType::Primitive)
  {
    _unit = UnitType::Calc;
    _value.Calc = calc.release();
  }

  PrimitiveValue::PrimitiveValue(CSSOMString value, UnitType unit) noexcept : css::Value(ValueType::Primitive)
  {
    _unit = unit;

    auto atom = CSSOMStringAtom(value);
    _value.String = *reinterpret_cast<const StringAtomStorage *>(atom.View().data());
  }

  PrimitiveValue::PrimitiveValue(double value, UnitType unit) noexcept : css::Value(ValueType::Primitive)
  {
    _unit = unit;
    _value.Number = value;
  }

  PrimitiveValue::PrimitiveValue(StaticCSSValueTag, CreateImplicitInitialValueTag) noexcept
      : PrimitiveValue(StaticCSSValue, ValueId::Initial)
  {
    _isImplicitInitialValue = true;
  }

  PrimitiveValue::PrimitiveValue(StaticCSSValueTag, css::ValueId keyword) noexcept
      : css::Value(ValueType::Primitive)
  {
    _unit = UnitType::ValueId;
    _value.ValueId = keyword;

    // TODO: we should handle this like webkit (by having ref count increment by 2 so a value of 1 is reserved
    // for static values that will not be destroyed) . For now, we will just increase the ref count by 1.
    AddRef();
  }

  PrimitiveValue::PrimitiveValue(StaticCSSValueTag, double number, UnitType type) noexcept
      : PrimitiveValue(number, type)
  {
    // TODO: we should handle this like webkit (by having ref count increment by 2 so a value of 1 is reserved
    // for static values that will not be destroyed) . For now, we will just increase the ref count by 1.
    AddRef();
  }

  KRYS_NODISCARD static PrimitiveValue *ValueFromPool(Span<AlignedStorage<PrimitiveValue>> pool,
                                                      double value) noexcept
  {
    // Casting to a signed integer first since casting a negative floating point value to an unsigned
    // integer is undefined behavior.
    size_t poolIndex = static_cast<size_t>(static_cast<int64>(value));
    double roundTripValue = static_cast<double>(poolIndex);

    if (std::ranges::equal(Span<const byte>(reinterpret_cast<const byte *>(&value), sizeof(double)),
                           Span<const byte>(reinterpret_cast<const byte *>(&roundTripValue), sizeof(double)))
        && poolIndex < pool.size())
    {
      return pool[poolIndex].get();
    }

    return nullptr;
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(PropertyId property) noexcept
  {
    return AdoptRef(*new PrimitiveValue(property));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(double value) noexcept
  {
    if (auto *result = ValueFromPool(CommonCSSValuePool->_numberValues, value))
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
      {
        if (auto *result = ValueFromPool(CommonCSSValuePool->_numberValues, value))
        {
          return ShareRef(*result);
        }
        break;
      }
      case UnitType::Percentage:
      {
        if (auto *result = ValueFromPool(CommonCSSValuePool->_percentageValues, value))
        {
          return ShareRef(*result);
        }
        break;
      }
      case UnitType::px:
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

  Ref<PrimitiveValue> PrimitiveValue::Create(Ref<AttrValue> value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(krys::move(value)));
  }

  Ref<PrimitiveValue> PrimitiveValue::Create(Ref<calc::Value> value) noexcept
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

  Ref<PrimitiveValue> PrimitiveValue::CreateInteger(double value) noexcept
  {
    return AdoptRef(*new PrimitiveValue(value, UnitType::Integer));
  }
}