#pragma once

#include "Krystal.Booey/CSS/Properties/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class CSSStyleProperties;
  class StyledElement;

  struct ParserContext;

  class MutableStyleProperties final : public StyleProperties
  {
    friend class StyleProperties;

  private:
    const UniquePtr<CSSStyleProperties> _cssomWrapper;
    SmallList<Property, 4uz> _propertyVector;

    explicit MutableStyleProperties(css::ParserMode mode) noexcept;
    explicit MutableStyleProperties(const css::StyleProperties &o) noexcept;
    MutableStyleProperties(SmallList<Property> &&properties) noexcept;

  public:
    ~MutableStyleProperties() noexcept;

    void SubRef() const noexcept
    {
      if (SubRefBase())
      {
        delete this;
      }
    }

    KRYS_NODISCARD static Ref<MutableStyleProperties> Create(css::ParserMode mode) noexcept;

    KRYS_NODISCARD static Ref<MutableStyleProperties> Create() noexcept;

    KRYS_NODISCARD static Ref<MutableStyleProperties> Create(SmallList<Property> &&properties) noexcept;

    KRYS_NODISCARD static Ref<MutableStyleProperties> CreateEmpty() noexcept;

    KRYS_NODISCARD Ref<ImmutableStyleProperties> ImmutableCopy() const noexcept;

    KRYS_NODISCARD Ref<ImmutableStyleProperties> ImmutableDeduplicatedCopy() const noexcept;

    KRYS_NODISCARD size_t PropertyCount() const noexcept
    {
      return _propertyVector.size();
    }

    KRYS_NODISCARD PropertyReference PropertyAt(size_t index) const noexcept
    {
      auto &property = _propertyVector[index];
      return PropertyReference(property.Metadata(), &property.Value());
    }

    KRYS_NODISCARD Iterator<MutableStyleProperties> begin() const noexcept
    {
      return {*this};
    }

    KRYS_NODISCARD static constexpr std::nullptr_t end() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD bool empty() const noexcept
    {
      return !PropertyCount();
    }

    KRYS_NODISCARD unsigned size() const noexcept
    {
      return PropertyCount();
    }

    KRYS_NODISCARD CSSStyleProperties *StyleProperties() noexcept;

    KRYS_NODISCARD bool AddParsedProperties(const ParsedPropertyList &properties) noexcept;

    KRYS_NODISCARD bool AddParsedProperty(const Property &property) noexcept;

    // These expand shorthand properties into multiple properties.
    KRYS_NODISCARD bool SetProperty(PropertyId property, const CSSOMString &value, ParserContext context,
                                    IsImportant = IsImportant(false),
                                    bool *didFailParsing = nullptr) noexcept;

    KRYS_NODISCARD bool SetProperty(PropertyId property, const CSSOMString &value,
                                    IsImportant = IsImportant(false),
                                    bool *didFailParsing = nullptr) noexcept;

    void SetProperty(PropertyId property, Ref<Value> &&value, IsImportant = IsImportant(false)) noexcept;

    // These do not. FIXME: This is too messy, we can do better.
    bool SetProperty(PropertyId property, ValueId identifier, IsImportant = IsImportant(false)) noexcept;

    bool SetProperty(const Property &property, Property *slot = nullptr) noexcept;

    bool RemoveProperty(PropertyId, CSSOMString *returnText = nullptr) noexcept;

    bool RemoveProperties(Span<const PropertyId> properties) noexcept;

    bool MergeAndOverrideOnConflict(const css::StyleProperties &o) noexcept;

    void Clear() noexcept;

    bool ParseDeclaration(const CSSOMString &styleDeclaration, ParserContext context) noexcept;

    CSSStyleProperties &EnsureCSSStyleProperties() noexcept;

    CSSStyleProperties &EnsureInlineCSSStyleProperties(StyledElement &parentElement) noexcept;

    KRYS_NODISCARD Maybe<size_t> FindPropertyIndex(PropertyId) const noexcept;

    KRYS_NODISCARD Maybe<size_t> FindCustomPropertyIndex(CSSOMStringView propertyName) const noexcept;

    // Methods for querying and altering CSS custom properties.
    bool SetCustomProperty(const CSSOMString &propertyName, const CSSOMString &value, ParserContext,
                           IsImportant = IsImportant(false)) noexcept;

    bool RemoveCustomProperty(const CSSOMString &propertyName, CSSOMString *returnText = nullptr) noexcept;

  private:
    KRYS_NODISCARD bool RemoveLonghandProperty(PropertyId property, CSSOMString *returnText) noexcept;

    KRYS_NODISCARD bool RemoveShorthandProperty(PropertyId property, CSSOMString *returnText) noexcept;

    KRYS_NODISCARD bool RemovePropertyAtIndex(size_t index, CSSOMString *returnText) noexcept;

    KRYS_NODISCARD Property *FindCSSPropertyWithId(PropertyId property) noexcept;

    KRYS_NODISCARD Property *FindCustomCSSPropertyWithName(const CSSOMString &) noexcept;

    KRYS_NODISCARD bool CanUpdateInPlace(const Property &property, Property *toReplace) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::MutableStyleProperties)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::StyleProperties &properties)
  {
    return properties.IsMutable();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
