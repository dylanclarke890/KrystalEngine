#pragma once

#include "Krystal.Booey/CSS/Properties/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  struct ParserContext;

  /// @brief A declaration block that can be changed in place.
  class MutableStyleProperties final : public StyleProperties
  {
    friend class StyleProperties;

  private:
    // FIXME: Add the CSSOM wrapper (CSSStyleProperties) and CssStyleProperties(), EnsureCSSStyleProperties()
    // and EnsureInlineCSSStyleProperties() once the CSSOM wrappers are ported.
    SmallList<Property, 4uz> _propertyVector;

    explicit MutableStyleProperties(css::ParserMode mode) noexcept;
    explicit MutableStyleProperties(const StyleProperties &other) noexcept;
    explicit MutableStyleProperties(List<Property> &&properties) noexcept;

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

    KRYS_NODISCARD static Ref<MutableStyleProperties> Create(List<Property> &&properties) noexcept;

    KRYS_NODISCARD static Ref<MutableStyleProperties> CreateEmpty() noexcept;

    KRYS_NODISCARD Ref<ImmutableStyleProperties> ImmutableCopy() const noexcept;

    KRYS_NODISCARD Ref<ImmutableStyleProperties> ImmutableDeduplicatedCopy() const noexcept;

    KRYS_NODISCARD uint32 PropertyCount() const noexcept
    {
      return static_cast<uint32>(_propertyVector.size());
    }

    KRYS_NODISCARD bool empty() const noexcept
    {
      return !PropertyCount();
    }

    KRYS_NODISCARD PropertyReference PropertyAt(uint32 index) const noexcept KRYS_LIFETIME_BOUND
    {
      auto &property = _propertyVector[index];
      return PropertyReference(property.Metadata(), &property.Value());
    }

    KRYS_NODISCARD Iterator<MutableStyleProperties> begin() const noexcept KRYS_LIFETIME_BOUND
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

    bool AddParsedProperties(const ParsedPropertyList &properties) noexcept;

    bool AddParsedProperty(const Property &property) noexcept;

    // These expand shorthand properties into multiple properties.
    // FIXME: Define the string overloads once the sheet parser (CSSParser::ParseValue) is ported.
    bool SetProperty(PropertyId property, const CSSOMString &value, ParserContext context,
                     IsImportant important = IsImportant(false), bool *didFailParsing = nullptr) noexcept;

    bool SetProperty(PropertyId property, const CSSOMString &value,
                     IsImportant important = IsImportant(false), bool *didFailParsing = nullptr) noexcept;

    void SetProperty(PropertyId property, Ref<css::Value> &&value,
                     IsImportant important = IsImportant(false)) noexcept;

    // These do not. FIXME: This is too messy, we can do better.
    bool SetProperty(PropertyId property, ValueId identifier,
                     IsImportant important = IsImportant(false)) noexcept;

    bool SetProperty(const Property &property, Property *slot = nullptr) noexcept;

    bool RemoveProperty(PropertyId property, CSSOMString *returnText = nullptr) noexcept;

    bool RemoveProperties(Span<const PropertyId> properties) noexcept;

    bool MergeAndOverrideOnConflict(const StyleProperties &other) noexcept;

    void Clear() noexcept;

    // FIXME: Define once the sheet parser (CSSParser::ParseDeclarationList) is ported.
    bool ParseDeclaration(const CSSOMString &styleDeclaration, ParserContext context) noexcept;

    KRYS_NODISCARD Maybe<uint32> FindPropertyIndex(PropertyId property) const noexcept;

    KRYS_NODISCARD Maybe<uint32> FindCustomPropertyIndex(CSSOMStringView propertyName) const noexcept;

    // Methods for querying and altering CSS custom properties.
    // FIXME: Define SetCustomProperty once the sheet parser (CSSParser::ParseCustomPropertyValue) is ported.
    bool SetCustomProperty(const CSSOMString &propertyName, const CSSOMString &value, ParserContext context,
                           IsImportant important = IsImportant(false)) noexcept;

    bool RemoveCustomProperty(CSSOMStringView propertyName, CSSOMString *returnText = nullptr) noexcept;

  private:
    bool RemoveLonghandProperty(PropertyId property, CSSOMString *returnText) noexcept;

    bool RemoveShorthandProperty(PropertyId property, CSSOMString *returnText) noexcept;

    bool RemovePropertyAtIndex(Maybe<uint32> index, CSSOMString *returnText) noexcept;

    KRYS_NODISCARD Property *FindCSSPropertyWithId(PropertyId property) noexcept;

    KRYS_NODISCARD Property *FindCustomCSSPropertyWithName(CSSOMStringView propertyName) noexcept;

    KRYS_NODISCARD bool CanUpdateInPlace(const Property &property, const Property *toReplace) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::MutableStyleProperties)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::StyleProperties &properties) noexcept
  {
    return properties.IsMutable();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
