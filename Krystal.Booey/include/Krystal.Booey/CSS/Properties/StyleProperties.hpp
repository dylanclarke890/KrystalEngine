#pragma once

#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <iterator>

namespace krys::boo
{
  class CachedResource;
}

namespace krys::boo::css
{
  class ImmutableStyleProperties;
  class MutableStyleProperties;

  struct SerialisationContext;

  enum class ValueId : uint16;
  enum class ParserMode : uint8;

  /// @brief A declaration block: an ordered list of longhand and custom property declarations, either mutable
  /// or immutable (a compact copy shared between rules).
  class StyleProperties : public RefCounted<StyleProperties>
  {
  protected:
    uint32 _parserMode : 3;
    mutable uint32 _isMutable : 1 {true};
    uint32 _arraySize : 28 {0u};

    inline explicit StyleProperties(css::ParserMode mode) noexcept;
    inline StyleProperties(css::ParserMode mode, uint32 immutableArraySize) noexcept;

  public:
    // Override RefCounted's SubRef() to ensure operator delete is called on the appropriate subclass type.
    inline void SubRef() const noexcept;

    class PropertyReference
    {
    private:
      const PropertyMetadata &_metadata;
      const css::Value *_value;

    public:
      PropertyReference(const PropertyMetadata &metadata, const css::Value *value) noexcept
          : _metadata(metadata), _value(value)
      {
      }

      KRYS_NODISCARD PropertyId Id() const noexcept
      {
        return _metadata.Id;
      }

      KRYS_NODISCARD PropertyId ShorthandId() const noexcept
      {
        return _metadata.ShorthandId();
      }

      KRYS_NODISCARD bool IsImportant() const noexcept
      {
        return HasFlag(_metadata.Flags, PropertyFlag::Important);
      }

      KRYS_NODISCARD bool IsImplicit() const noexcept
      {
        return HasFlag(_metadata.Flags, PropertyFlag::Implicit);
      }

      KRYS_NODISCARD CSSOMString CssName() const noexcept;

      KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept;

      KRYS_NODISCARD const css::Value *Value() const noexcept
      {
        return _value;
      }

      // FIXME: We should try to remove this mutable overload.
      KRYS_NODISCARD css::Value *Value() noexcept
      {
        return const_cast<css::Value *>(_value);
      }

      KRYS_NODISCARD Ref<css::Value> ProtectedValue() const noexcept
      {
        return ShareRef(const_cast<css::Value &>(*_value));
      }

      // FIXME: Remove this.
      KRYS_NODISCARD Property ToProperty() const noexcept
      {
        return Property(_metadata, ProtectedValue());
      }
    };

    template <typename T>
    struct Iterator
    {
    private:
      const T &_properties;
      uint32 _index {0u};

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = PropertyReference;
      using difference_type = ptrdiff_t;
      using pointer = PropertyReference;
      using reference = PropertyReference;

      Iterator(const T &properties) noexcept : _properties {properties}
      {
      }

      KRYS_NODISCARD PropertyReference operator*() const noexcept
      {
        return _properties.PropertyAt(_index);
      }

      Iterator &operator++() noexcept
      {
        ++_index;
        return *this;
      }

      KRYS_NODISCARD bool operator==(std::nullptr_t) const noexcept
      {
        return _index >= _properties.PropertyCount();
      }
    };

    KRYS_NODISCARD inline uint32 PropertyCount() const noexcept;

    KRYS_NODISCARD inline bool empty() const noexcept;

    KRYS_NODISCARD inline PropertyReference PropertyAt(uint32 index) const noexcept KRYS_LIFETIME_BOUND;

    KRYS_NODISCARD Iterator<StyleProperties> begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return {*this};
    }

    KRYS_NODISCARD constexpr static std::nullptr_t end() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD inline uint32 size() const noexcept;

    KRYS_NODISCARD RefPtr<css::Value> GetPropertyCSSValue(PropertyId property) const noexcept;

    KRYS_NODISCARD CSSOMString GetPropertyValue(PropertyId property) const noexcept;

    KRYS_NODISCARD Maybe<boo::Color> PropertyAsColor(PropertyId property) const noexcept;

    KRYS_NODISCARD Maybe<ValueId> PropertyAsValueId(PropertyId property) const noexcept;

    KRYS_NODISCARD bool PropertyIsImportant(PropertyId property) const noexcept;

    KRYS_NODISCARD CSSOMStringView GetPropertyShorthand(PropertyId property) const noexcept;

    KRYS_NODISCARD bool IsPropertyImplicit(PropertyId property) const noexcept;

    KRYS_NODISCARD RefPtr<css::Value> GetCustomPropertyCSSValue(CSSOMStringView propertyName) const noexcept;

    KRYS_NODISCARD CSSOMString GetCustomPropertyValue(CSSOMStringView propertyName) const noexcept;

    KRYS_NODISCARD bool CustomPropertyIsImportant(CSSOMStringView propertyName) const noexcept;

    KRYS_NODISCARD css::ParserMode ParserMode() const noexcept
    {
      return static_cast<css::ParserMode>(_parserMode);
    }

    KRYS_NODISCARD Ref<MutableStyleProperties> MutableCopy() const noexcept;

    KRYS_NODISCARD Ref<ImmutableStyleProperties> ImmutableCopyIfNeeded() const noexcept;

    KRYS_NODISCARD Ref<MutableStyleProperties>
      CopyProperties(Span<const PropertyId> properties) const noexcept;

    KRYS_NODISCARD CSSOMString AsText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD CSSOMStringAtom AsTextAtom(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool HasCSSOMWrapper() const noexcept;

    KRYS_NODISCARD bool IsMutable() const noexcept
    {
      return _isMutable;
    }

    KRYS_NODISCARD bool
      TraverseSubresources(KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;

    KRYS_NODISCARD bool MayDependOnBaseURL() const noexcept;

    KRYS_NODISCARD static size_t AverageSizeInBytes() noexcept;

#if KRYS_DEBUG
    void ShowStyle() const noexcept;
#endif

    KRYS_NODISCARD bool PropertyMatches(PropertyId property, const css::Value *value) const noexcept;

    KRYS_NODISCARD inline Maybe<uint32> FindPropertyIndex(PropertyId property) const noexcept;

    KRYS_NODISCARD inline Maybe<uint32> FindCustomPropertyIndex(CSSOMStringView propertyName) const noexcept;

  private:
    KRYS_NODISCARD CSSOMString AsTextInternal(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD CSSOMString SerialiseLonghandValue(const SerialisationContext &context,
                                                      PropertyId property) const noexcept;

    KRYS_NODISCARD Maybe<CSSOMString> SerialiseShorthandValue(const SerialisationContext &context,
                                                              PropertyId property) const noexcept;
  };

  /// @brief Serialises `value` as the value of `property`, writing implicit initial values (including those
  /// in comma-separated lists) as the property's initial value.
  KRYS_NODISCARD CSSOMString SerialiseLonghandValue(const SerialisationContext &context, PropertyId property,
                                                    const Value &value) noexcept;

  KRYS_NODISCARD inline CSSOMString SerialiseLonghandValue(const SerialisationContext &context,
                                                           PropertyId property, const Value *value) noexcept;

  KRYS_NODISCARD inline ValueId LonghandValueId(PropertyId property, const Value &value) noexcept;

  KRYS_NODISCARD inline Maybe<ValueId> LonghandValueId(PropertyId property, const Value *value) noexcept;
}
