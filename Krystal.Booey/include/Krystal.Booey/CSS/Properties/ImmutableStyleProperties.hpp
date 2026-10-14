#pragma once

#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Packed.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <bit>

namespace krys::boo::css
{
  /// @brief A declaration block that can't be changed, stored inline after the object: the property metadata,
  /// then the values.
  class ImmutableStyleProperties final : public StyleProperties
  {
  private:
    void *_storage;

    ImmutableStyleProperties(Span<const Property> properties, css::ParserMode mode) noexcept;

  public:
    ~ImmutableStyleProperties() noexcept;

    /// @brief The storage is allocated with the object (see `ObjectSize`), so it's freed unsized.
    static void operator delete(void *pointer) noexcept
    {
      ::operator delete(pointer);
    }

    void SubRef() const noexcept
    {
      if (SubRefBase())
      {
        delete this;
      }
    }

    KRYS_NODISCARD static Ref<ImmutableStyleProperties> Create(Span<const Property> properties,
                                                               css::ParserMode mode) noexcept;

    KRYS_NODISCARD static Ref<ImmutableStyleProperties> CreateDeduplicating(Span<const Property> properties,
                                                                            css::ParserMode mode) noexcept;

    KRYS_NODISCARD uint32 PropertyCount() const noexcept
    {
      return _arraySize;
    }

    KRYS_NODISCARD bool empty() const noexcept
    {
      return !PropertyCount();
    }

    KRYS_NODISCARD PropertyReference PropertyAt(uint32 index) const noexcept KRYS_LIFETIME_BOUND
    {
      return PropertyReference(MetadataSpan()[index], ValueSpan()[index].get());
    }

    KRYS_NODISCARD Iterator<ImmutableStyleProperties> begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return {*this};
    }

    KRYS_NODISCARD constexpr static std::nullptr_t end() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD uint32 size() const noexcept
    {
      return PropertyCount();
    }

    KRYS_NODISCARD Maybe<uint32> FindPropertyIndex(PropertyId property) const noexcept;

    KRYS_NODISCARD Maybe<uint32> FindCustomPropertyIndex(CSSOMStringView propertyName) const noexcept;

    KRYS_NODISCARD constexpr static size_t ObjectSize(uint32 propertyCount) noexcept;

    static void ClearDeduplicationMap() noexcept;

  private:
    KRYS_NODISCARD Span<PackedPtr<const css::Value>> ValueSpan() const noexcept
    {
      return Span<PackedPtr<const css::Value>>(
        std::bit_cast<PackedPtr<const css::Value> *>(std::bit_cast<const uint8 *>(MetadataSpan().data())
                                                     + (_arraySize * sizeof(PropertyMetadata))),
        PropertyCount());
    }

    KRYS_NODISCARD Span<const PropertyMetadata> MetadataSpan() const noexcept
    {
      return Span<const PropertyMetadata>(reinterpret_cast<const PropertyMetadata *>(&_storage),
                                          PropertyCount());
    }
  };

  constexpr size_t ImmutableStyleProperties::ObjectSize(uint32 count) noexcept
  {
    return sizeof(ImmutableStyleProperties) - sizeof(void *) + sizeof(PropertyMetadata) * count
           + sizeof(PackedPtr<const css::Value>) * count;
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ImmutableStyleProperties)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::StyleProperties &properties) noexcept
  {
    return !properties.IsMutable();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
