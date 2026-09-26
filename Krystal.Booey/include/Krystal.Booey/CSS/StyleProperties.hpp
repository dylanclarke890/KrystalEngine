#pragma once

#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class CachedResource;
  class Color;
  class ImmutableStyleProperties;
  class MutableStyleProperties;

  struct SerialisationContext;

  enum class ValueId : uint16;
  enum class ParserMode : uint8;

  class StyleProperties : public RefCounted<StyleProperties>
  {
  protected:
    size_t _parserMode : 3;
    mutable size_t _isMutable : 1 {true};
    size_t _arraySize : 28 {0};

    explicit StyleProperties(css::ParserMode parserMode) noexcept;

    StyleProperties(css::ParserMode parserMode, size_t immutableArraySize) noexcept;

  public:
    // Override RefCounted's SubRef() to ensure operator delete is called on the appropriate subclass type.
    void SubRef() const noexcept;

    class PropertyReference
    {
    private:
      const PropertyMetadata &_metadata;
      const Value *_value;

    public:
      PropertyReference(const PropertyMetadata &metadata, const Value *value) noexcept
          : _metadata(metadata), _value(value)
      {
      }

      KRYS_NODISCARD PropertyId Id() const noexcept
      {
        return static_cast<PropertyId>(_metadata.Id);
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

      const css::Value *Value() const noexcept
      {
        return _value;
      }
    };

    template <typename T>
    struct Iterator
    {
    private:
      const T &_properties;
      size_t _index {0uz};

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = PropertyReference;
      using difference_type = ptrdiff_t;
      using pointer = PropertyReference;
      using reference = PropertyReference;

      Iterator(const T &properties) noexcept : _properties {properties}
      {
      }

      PropertyReference operator*() const noexcept
      {
        return _properties.PropertyAt(_index);
      }

      Iterator &operator++() noexcept
      {
        ++_index;
        return *this;
      }

      bool operator==(std::nullptr_t) const noexcept
      {
        return _index >= _properties.PropertyCount();
      }
    };

    KRYS_NODISCARD size_t PropertyCount() const noexcept;

    KRYS_NODISCARD PropertyReference PropertyAt(size_t index) const noexcept;

    KRYS_NODISCARD Iterator<StyleProperties> begin() const noexcept
    {
      return {*this};
    }

    KRYS_NODISCARD constexpr static std::nullptr_t end() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD bool empty() const noexcept;

    KRYS_NODISCARD size_t size() const noexcept;

    KRYS_NODISCARD RefPtr<Value> GetPropertyCSSValue(PropertyId property) const noexcept;

    KRYS_NODISCARD CSSOMString GetPropertyValue(PropertyId property) const noexcept;

    KRYS_NODISCARD Maybe<Color> PropertyAsColor(PropertyId property) const noexcept;

    KRYS_NODISCARD Maybe<ValueId> PropertyAsValueID(PropertyId property) const noexcept;

    KRYS_NODISCARD bool PropertyIsImportant(PropertyId property) const noexcept;

    KRYS_NODISCARD CSSOMString GetPropertyShorthand(PropertyId property) const noexcept;

    KRYS_NODISCARD bool IsPropertyImplicit(PropertyId property) const noexcept;

    KRYS_NODISCARD RefPtr<Value> GetCustomPropertyCSSValue(const CSSOMString &propertyName) const noexcept;

    KRYS_NODISCARD CSSOMString GetCustomPropertyValue(const CSSOMString &propertyName) const noexcept;

    KRYS_NODISCARD bool CustomPropertyIsImportant(const CSSOMString &propertyName) const noexcept;

    KRYS_NODISCARD Ref<MutableStyleProperties> CopyBlockProperties() const noexcept;

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
      TraverseSubresources(const Func<bool(const CachedResource &)> &handler) const noexcept;

    KRYS_NODISCARD bool MayDependOnBaseURL() const noexcept;

    KRYS_NODISCARD static size_t AverageSizeInBytes() noexcept;

#ifdef KRYS_DEBUG
    void ShowStyle() noexcept;
#endif

    KRYS_NODISCARD bool PropertyMatches(PropertyId property, const Value *value) const noexcept;

    KRYS_NODISCARD Maybe<size_t> FindPropertyIndex(PropertyId property) const noexcept;

    KRYS_NODISCARD Maybe<size_t> FindCustomPropertyIndex(CSSOMStringView propertyName) const noexcept;

  private:
    KRYS_NODISCARD CSSOMString AsTextInternal(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD CSSOMString SerialiseLonghandValue(const SerialisationContext &context,
                                                      PropertyId property) const noexcept;

    KRYS_NODISCARD CSSOMString SerialiseShorthandValue(const SerialisationContext &context,
                                                       PropertyId property) const noexcept;
  };

  KRYS_NODISCARD inline CSSOMString SerialiseLonghandValue(const SerialisationContext &context,
                                                           PropertyId property, const Value &value) noexcept;

  KRYS_NODISCARD inline CSSOMString SerialiseLonghandValue(const SerialisationContext &context,
                                                           PropertyId property, const Value *value) noexcept;

  KRYS_NODISCARD inline ValueId LonghandValueId(PropertyId property, const Value &value) noexcept;

  KRYS_NODISCARD inline Maybe<ValueId> LonghandValueId(PropertyId property, const Value *value) noexcept;
}