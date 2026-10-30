#pragma once

#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyInitialValues.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css
{
  inline StyleProperties::StyleProperties(css::ParserMode mode) noexcept
      : _parserMode(static_cast<uint32>(mode)), _isMutable(true)
  {
  }

  inline StyleProperties::StyleProperties(css::ParserMode mode, uint32 immutableArraySize) noexcept
      : _parserMode(static_cast<uint32>(mode)), _isMutable(false), _arraySize(immutableArraySize)
  {
  }

  inline StyleProperties::PropertyReference StyleProperties::PropertyAt(uint32 index) const noexcept
  {
    if (_isMutable)
    {
      return Downcast<MutableStyleProperties>(*this).PropertyAt(index);
    }

    return Downcast<ImmutableStyleProperties>(*this).PropertyAt(index);
  }

  inline uint32 StyleProperties::PropertyCount() const noexcept
  {
    if (_isMutable)
    {
      return Downcast<MutableStyleProperties>(*this).PropertyCount();
    }

    return Downcast<ImmutableStyleProperties>(*this).PropertyCount();
  }

  inline void StyleProperties::SubRef() const noexcept
  {
    if (!SubRefBase())
    {
      return;
    }

    if (auto *mutableProperties = DynamicDowncast<MutableStyleProperties>(*this))
    {
      delete mutableProperties;
    }
    else if (auto *immutableProperties = DynamicDowncast<ImmutableStyleProperties>(*this))
    {
      delete immutableProperties;
    }
    else
    {
      krys_unreachable();
    }
  }

  inline Maybe<uint32> StyleProperties::FindPropertyIndex(PropertyId property) const noexcept
  {
    if (_isMutable)
    {
      return Downcast<MutableStyleProperties>(*this).FindPropertyIndex(property);
    }

    return Downcast<ImmutableStyleProperties>(*this).FindPropertyIndex(property);
  }

  inline Maybe<uint32> StyleProperties::FindCustomPropertyIndex(CSSOMStringView propertyName) const noexcept
  {
    if (_isMutable)
    {
      return Downcast<MutableStyleProperties>(*this).FindCustomPropertyIndex(propertyName);
    }

    return Downcast<ImmutableStyleProperties>(*this).FindCustomPropertyIndex(propertyName);
  }

  inline bool StyleProperties::empty() const noexcept
  {
    return !PropertyCount();
  }

  inline uint32 StyleProperties::size() const noexcept
  {
    return PropertyCount();
  }

  inline CSSOMString SerialiseLonghandValue(const SerialisationContext &context, PropertyId property,
                                            const Value *value) noexcept
  {
    return value ? SerialiseLonghandValue(context, property, *value) : CSSOMString();
  }

  inline ValueId LonghandValueId(PropertyId property, const Value &value) noexcept
  {
    return value.IsImplicitInitialValue() ? InitialValueIdForLonghand(property) : GetValueId(value);
  }

  inline Maybe<ValueId> LonghandValueId(PropertyId property, const Value *value) noexcept
  {
    if (!value)
    {
      return null;
    }

    return LonghandValueId(property, *value);
  }
}
