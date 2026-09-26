#pragma once

#include "Krystal.Booey/CSS/StyleProperties.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::css
{
  class ImmutableStyleProperties final : public StyleProperties
  {
  private:
    void *_storage;

    ImmutableStyleProperties(Span<const Property> properties, css::ParserMode mode) noexcept;

  public:
    ~ImmutableStyleProperties() noexcept;

    KRYS_NODISCARD static Ref<ImmutableStyleProperties> Create(Span<const Property> properties,
                                                               css::ParserMode mode) noexcept;

    KRYS_NODISCARD static Ref<ImmutableStyleProperties> CreateDeduplicating(Span<const Property> properties,
                                                                            css::ParserMode mode) noexcept;

    KRYS_NODISCARD size_t PropertyCount() const noexcept
    {
      return _arraySize;
    }

    KRYS_NODISCARD PropertyReference PropertyAt(size_t index) const noexcept
    {
      return PropertyReference(MetadataSpan()[index], ValueSpan()[index].get());
    }

    Iterator<ImmutableStyleProperties> begin() const noexcept
    {
      return {*this};
    }

    KRYS_NODISCARD constexpr static std::nullptr_t end() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD bool empty() const noexcept
    {
      return !PropertyCount();
    }

    KRYS_NODISCARD size_t size() const noexcept
    {
      return PropertyCount();
    }

    KRYS_NODISCARD Maybe<size_t> FindPropertyIndex(PropertyId property) const noexcept;

    KRYS_NODISCARD Maybe<size_t> FindCustomPropertyIndex(CSSOMStringView propertyName) const noexcept;

    KRYS_NODISCARD constexpr static size_t ObjectSize(size_t propertyCount) noexcept
    {
      return sizeof(ImmutableStyleProperties) - sizeof(void *) + sizeof(PropertyMetadata) * propertyCount
             + sizeof(PackedPtr<const Value>) * propertyCount;
    }

    static void ClearDeduplicationMap() noexcept;

  private:
    Span<PackedPtr<const Value>> ValueSpan() const noexcept
    {
      return unsafeMakeSpan(
        std::bit_cast<PackedPtr<const Value> *>(std::bit_cast<const uint8_t *>(MetadataSpan().data())
                                                + (_arraySize * sizeof(PropertyMetadata))),
        PropertyCount());
    }
    Span<const PropertyMetadata> MetadataSpan() const noexcept
    {
      return unsafeMakeSpan(
        reinterpret_cast<const PropertyMetadata *>(const_cast<const void **>((&(this->_storage)))),
        PropertyCount());
    }

    void SubRef() const noexcept
    {
      if (SubRefBase())
      {
        delete this;
      }
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ImmutableStyleProperties)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::StyleProperties &properties)
  {
    return !properties.IsMutable();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
