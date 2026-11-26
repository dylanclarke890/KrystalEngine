#pragma once

#include "Krystal.Booey/Css/Properties/Property.hpp"
#include "Krystal.Booey/Css/Types/CSSOMString.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <iterator>

namespace krys::boo
{
  class CachedResource;
  class Color;
}

namespace krys::boo::css
{
  class ImmutableStyleProperties;
  class MutableStyleProperties;

  struct SerialisationContext;

  enum class ParserMode : uint8;
  enum class ValueId : uint16;

  /// @brief A declaration block: an ordered list of longhand and custom property declarations. It's either
  /// mutable (`MutableStyleProperties`) or a compact immutable copy that rules share
  /// (`ImmutableStyleProperties`); this base dispatches to whichever it is.
  /// @see https://drafts.csswg.org/cssom-1/#css-declaration-block
  class StyleProperties : public RefCounted<StyleProperties>
  {
  protected:
    uint32 _parserMode : 3;
    mutable uint32 _isMutable : 1 {true};
    /// @brief An immutable block's declaration count; its storage follows the object.
    uint32 _arraySize : 28 {0u};

    inline explicit StyleProperties(css::ParserMode mode) noexcept;

    inline StyleProperties(css::ParserMode mode, uint32 immutableArraySize) noexcept;

  public:
    /// @brief Releases a reference, deleting the object as its subclass when it was the last.
    inline void SubRef() const noexcept;

    /// @brief A view of one declaration in the block.
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

      /// @brief The property, or `PropertyId::Custom` for a custom property.
      KRYS_NODISCARD PropertyId Id() const noexcept
      {
        return _metadata.Id;
      }

      /// @brief The shorthand that set the declaration, or `PropertyId::Invalid` if it was set directly.
      KRYS_NODISCARD PropertyId ShorthandId() const noexcept
      {
        return _metadata.ShorthandId();
      }

      KRYS_NODISCARD bool IsImportant() const noexcept
      {
        return _metadata.Flags.contains(PropertyFlag::Important);
      }

      /// @brief The property's name, a custom property's included.
      KRYS_NODISCARD CSSOMString CssName() const noexcept;

      KRYS_NODISCARD const css::Value *Value() const noexcept
      {
        return _value;
      }

      // FIXME: Remove this.
      KRYS_NODISCARD Property ToProperty() const noexcept
      {
        return Property(_metadata, ShareRef(const_cast<css::Value &>(*_value)));
      }
    };

    /// @brief Walks a block's declarations, ending at the `nullptr` sentinel `end()` returns.
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

      explicit Iterator(const T &properties) noexcept : _properties {properties}
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
        return _index >= _properties.size();
      }
    };

    /// @brief The number of declarations.
    KRYS_NODISCARD inline uint32 size() const noexcept;

    KRYS_NODISCARD inline bool empty() const noexcept;

    KRYS_NODISCARD inline PropertyReference PropertyAt(uint32 index) const noexcept KRYS_LIFETIME_BOUND;

    KRYS_NODISCARD Iterator<StyleProperties> begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return Iterator<StyleProperties>(*this);
    }

    KRYS_NODISCARD constexpr static std::nullptr_t end() noexcept
    {
      return nullptr;
    }

    /// @brief Longhand @p property's value, or null if it isn't declared or is part of a system font.
    KRYS_NODISCARD RefPtr<css::Value> GetPropertyCSSValue(PropertyId property) const noexcept;

    /// @brief @p property's value serialised, a shorthand's from its longhands; empty if it isn't declared.
    /// @see https://drafts.csswg.org/cssom-1/#dom-cssstyledeclaration-getpropertyvalue
    KRYS_NODISCARD CSSOMString GetPropertyValue(PropertyId property) const noexcept;

    /// @brief Longhand @p property's value as an absolute colour, if it's declared and is one.
    KRYS_NODISCARD Maybe<boo::Color> PropertyAsColor(PropertyId property) const noexcept;

    /// @brief Longhand @p property's value as a keyword, its initial one if set implicitly.
    KRYS_NODISCARD Maybe<ValueId> PropertyAsValueId(PropertyId property) const noexcept;

    /// @brief Whether @p property is declared `!important`; a shorthand is when all its longhands are.
    KRYS_NODISCARD bool PropertyIsImportant(PropertyId property) const noexcept;

    /// @brief Custom property @p propertyName's value, or null if it isn't declared.
    KRYS_NODISCARD RefPtr<css::Value> GetCustomPropertyCSSValue(CSSOMStringView propertyName) const noexcept;

    /// @brief Custom property @p propertyName's value serialised; empty if it isn't declared.
    KRYS_NODISCARD CSSOMString GetCustomPropertyValue(CSSOMStringView propertyName) const noexcept;

    KRYS_NODISCARD bool CustomPropertyIsImportant(CSSOMStringView propertyName) const noexcept;

    /// @brief The mode the block was parsed in.
    KRYS_NODISCARD css::ParserMode ParserMode() const noexcept
    {
      return static_cast<css::ParserMode>(_parserMode);
    }

    KRYS_NODISCARD Ref<MutableStyleProperties> MutableCopy() const noexcept;

    /// @brief This block if it's immutable, or a shared immutable copy of it.
    KRYS_NODISCARD Ref<ImmutableStyleProperties> ImmutableCopyIfNeeded() const noexcept;

    /// @brief A mutable block with @p properties' declarations, those that are declared.
    KRYS_NODISCARD Ref<MutableStyleProperties>
      CopyProperties(Span<const PropertyId> properties) const noexcept;

    /// @brief The block serialised, with longhands collapsed into their shorthands where they can be.
    /// @see https://drafts.csswg.org/cssom-1/#serialize-a-css-declaration-block
    KRYS_NODISCARD CSSOMString AsText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD CSSOMStringAtom AsTextAtom(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool IsMutable() const noexcept
    {
      return _isMutable;
    }

    /// @brief Calls @p handler with each resource the values load until it returns true.
    /// @return Whether @p handler returned true.
    KRYS_NODISCARD bool
      TraverseSubresources(KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;

    /// @brief Whether any value, or one nested in it, has a relative URL.
    KRYS_NODISCARD bool MayDependOnBaseUrl() const noexcept;

    /// @brief A typical immutable block's size, for estimating a style sheet's memory use.
    KRYS_NODISCARD static size_t AverageSizeInBytes() noexcept;

    /// @brief Whether longhand @p property is declared with a value equal to @p value.
    KRYS_NODISCARD bool PropertyMatches(PropertyId property, const css::Value *value) const noexcept;

    KRYS_NODISCARD inline Maybe<uint32> FindPropertyIndex(PropertyId property) const noexcept;

    KRYS_NODISCARD inline Maybe<uint32> FindCustomPropertyIndex(CSSOMStringView propertyName) const noexcept;

  private:
    KRYS_NODISCARD CSSOMString SerialiseLonghandValue(const SerialisationContext &context,
                                                      PropertyId property) const noexcept;

    KRYS_NODISCARD Maybe<CSSOMString> SerialiseShorthandValue(const SerialisationContext &context,
                                                              PropertyId property) const noexcept;
  };

  /// @brief Serialises @p value as @p property's, writing an implicit initial value, a list's items'
  /// included, as the property's initial value rather than `initial`.
  KRYS_NODISCARD CSSOMString SerialiseLonghandValue(const SerialisationContext &context, PropertyId property,
                                                    const Value &value) noexcept;

  /// @brief Serialises @p value as @p property's, or gives an empty string if it's null.
  KRYS_NODISCARD inline CSSOMString SerialiseLonghandValue(const SerialisationContext &context,
                                                           PropertyId property, const Value *value) noexcept;

  /// @brief @p value's keyword, or @p property's initial keyword if @p value is an implicit initial value.
  KRYS_NODISCARD inline ValueId LonghandValueId(PropertyId property, const Value &value) noexcept;

  KRYS_NODISCARD inline Maybe<ValueId> LonghandValueId(PropertyId property, const Value *value) noexcept;
}
