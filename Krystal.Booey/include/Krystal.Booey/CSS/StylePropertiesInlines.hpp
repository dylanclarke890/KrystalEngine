#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyInitialValues.hpp"
#include "Krystal.Booey/CSS/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/MutableStyleProperties.hpp"

namespace krys::boo::css
{
  inline StyleProperties::StyleProperties(css::ParserMode mode) noexcept : _parserMode(static_cast<underlying_t<css::ParserMode>>(mode)), _isMutable(true)
  {
  }

  inline StyleProperties::StyleProperties(css::ParserMode mode, size_t immutableArraySize) noexcept
      : _parserMode(static_cast<underlying_t<css::ParserMode>>(mode)), _isMutable(false), _arraySize(immutableArraySize)
  {
  }

  inline StyleProperties::PropertyReference StyleProperties::PropertyAt(size_t index) const noexcept
  {
    if (_isMutable)
      return uncheckedDowncast<MutableStyleProperties>(*this).propertyAt(index);
    return uncheckedDowncast<ImmutableStyleProperties>(*this).propertyAt(index);
  }

  inline unsigned StyleProperties::propertyCount() const noexcept
  {
    if (_isMutable)
      return uncheckedDowncast<MutableStyleProperties>(*this).propertyCount();
    return uncheckedDowncast<ImmutableStyleProperties>(*this).propertyCount();
  }

  inline void StyleProperties::deref() const noexcept
  {
    if (!derefBase())
      return;

    if (auto *mutableProperties = dynamicDowncast<MutableStyleProperties>(*this))
      delete mutableProperties;
    else if (auto *immutableProperties = dynamicDowncast<ImmutableStyleProperties>(*this))
      delete immutableProperties;
    else
      RELEASE_ASSERT_NOT_REACHED();
  }

  inline int StyleProperties::findPropertyIndex(CSSPropertyID propertyID) const
  {
    if (m_isMutable)
      return uncheckedDowncast<MutableStyleProperties>(*this).findPropertyIndex(propertyID);
    return uncheckedDowncast<ImmutableStyleProperties>(*this).findPropertyIndex(propertyID);
  }

  inline int StyleProperties::findCustomPropertyIndex(StringView propertyName) const
  {
    if (m_isMutable)
      return uncheckedDowncast<MutableStyleProperties>(*this).findCustomPropertyIndex(propertyName);
    return uncheckedDowncast<ImmutableStyleProperties>(*this).findCustomPropertyIndex(propertyName);
  }

  inline bool StyleProperties::isEmpty() const
  {
    return !propertyCount();
  }

  inline unsigned StyleProperties::size() const
  {
    return propertyCount();
  }

  inline String serializeLonghandValue(const CSS::SerializationContext &context, CSSPropertyID property,
                                       const CSSValue *value)
  {
    return value ? serializeLonghandValue(context, property, *value) : String();
  }

  inline ValueId LonghandValueID(PropertyId property, const CSSValue &value)
  {
    return value.isImplicitInitialValue() ? initialValueIDForLonghand(property) : valueID(value);
  }

  inline std::optional<CSSValueID> longhandValueID(CSSPropertyID property, const CSSValue *value)
  {
    if (!value)
      return std::nullopt;
    return longhandValueID(property, *value);
  }

} // namespace WebCore
