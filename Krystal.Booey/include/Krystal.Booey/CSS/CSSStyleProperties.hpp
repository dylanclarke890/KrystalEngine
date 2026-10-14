#pragma once

#include "Krystal.Booey/CSS/CSSStyleDeclaration.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class MutableStyleProperties;
  class StyleSheetContents;
  struct ParserContext;

  /// @brief A declaration block of CSS properties exposed to scripts: a style rule's `style`, an element's
  /// `style` attribute or a computed style.
  /// @see https://drafts.csswg.org/cssom/#the-cssstyleproperties-interface
  class CSSStyleProperties : public CSSStyleDeclaration
  {
  public:
    KRYS_NODISCARD StyleDeclarationType GetStyleDeclarationType() const noexcept final
    {
      return StyleDeclarationType::Style;
    }

    KRYS_NODISCARD CSSOMString CssFloat() noexcept;
    dom::ExceptionOr<void> SetCssFloat(const CSSOMString &value) noexcept;

    // PropertyId versions of the CSSOM functions to support bindings and editing.
    KRYS_NODISCARD virtual CSSOMString GetPropertyValueInternal(PropertyId property) noexcept = 0;
    virtual dom::ExceptionOr<void> SetPropertyInternal(PropertyId property, const CSSOMString &value,
                                                       IsImportant important) noexcept = 0;
    KRYS_NODISCARD virtual Ref<MutableStyleProperties> CopyProperties() const noexcept = 0;

    // FIXME: It would be more efficient, by virtue of avoiding the text transformation and hash lookup
    // currently required in the implementation, if we could could smuggle the PropertyId through the
    // bindings, perhaps by encoding it into the HashTableValue and then passing it together with the
    // PropertyName.

    // Shared implementation for all properties that match
    // https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-camel_cased_attribute.
    KRYS_NODISCARD CSSOMString
      GetPropertyValueForCamelCasedIDLAttribute(const CSSOMStringAtom &attribute) noexcept;
    dom::ExceptionOr<void> SetPropertyValueForCamelCasedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                     const CSSOMString &value) noexcept;

    // Shared implementation for all properties that match
    // https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-webkit_cased_attribute.
    KRYS_NODISCARD CSSOMString
      GetPropertyValueForWebKitCasedIDLAttribute(const CSSOMStringAtom &attribute) noexcept;
    dom::ExceptionOr<void> SetPropertyValueForWebKitCasedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                      const CSSOMString &value) noexcept;

    // Shared implementation for all properties that match
    // https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-dashed_attribute.
    KRYS_NODISCARD CSSOMString
      GetPropertyValueForDashedIDLAttribute(const CSSOMStringAtom &attribute) noexcept;
    dom::ExceptionOr<void> SetPropertyValueForDashedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                 const CSSOMString &value) noexcept;

    /// @brief The property named by a script property name: camel-cased (`backgroundColor`) or
    /// WebKit-cased (`webkitTransform`).
    // FIXME: This needs to pass in Settings to work correctly.
    KRYS_NODISCARD static PropertyId
      GetPropertyIdFromJavaScriptPropertyName(const CSSOMStringAtom &propertyName) noexcept;
  };

  /// @brief The CSSOM view of a declaration block. The declaration block owns it.
  // FIXME: InlineCSSStyleProperties (an element's `style`) and the style attribute mutation records
  // (StyleAttributeMutationScope) come with inline styles.
  class PropertySetCSSStyleProperties : public CSSStyleProperties
  {
  protected:
    enum class MutationType : uint8
    {
      NoChanges,
      StyleAttributeChanged,
      PropertyChanged
    };

    MutableStyleProperties *_propertySet;

  public:
    explicit PropertySetCSSStyleProperties(MutableStyleProperties &propertySet) noexcept
        : _propertySet(&propertySet)
    {
    }

    void AddRef() const noexcept override;
    void SubRef() const noexcept override;
    KRYS_NODISCARD uint32 GetRefCount() const noexcept override;

    KRYS_NODISCARD StyleSheetContents *ContextStyleSheet() const noexcept;

  protected:
    KRYS_NODISCARD virtual ParserContext CssParserContext() const noexcept;

  private:
    KRYS_NODISCARD CSSRule *ParentRule() const noexcept override
    {
      return nullptr;
    }

    // FIXME: To implement.
    KRYS_NODISCARD CSSRule *CssRules() const noexcept override
    {
      return nullptr;
    }

    KRYS_NODISCARD uint32 Length() const noexcept final;
    KRYS_NODISCARD CSSOMString Item(uint32 index) const noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyValue(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyPriority(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyShorthand(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD bool IsPropertyImplicit(const CSSOMString &propertyName) noexcept final;
    dom::ExceptionOr<void> SetProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                       const CSSOMString &priority) noexcept final;
    dom::ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    dom::ExceptionOr<void> SetCssText(const CSSOMString &text) noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyValueInternal(PropertyId property) noexcept final;
    dom::ExceptionOr<void> SetPropertyInternal(PropertyId property, const CSSOMString &value,
                                               IsImportant important) noexcept final;
    KRYS_NODISCARD Ref<MutableStyleProperties> CopyProperties() const noexcept final;

    KRYS_NODISCARD bool IsExposed(PropertyId property) const noexcept;

    KRYS_NODISCARD virtual bool WillMutate() noexcept
    {
      return true;
    }

    virtual void DidMutate(MutationType) noexcept
    {
    }
  };

  /// @brief The `style` of a style, keyframe or nested declarations rule.
  class StyleRuleCSSStyleProperties final : public PropertySetCSSStyleProperties,
                                            public RefCounted<StyleRuleCSSStyleProperties>
  {
  private:
    RuleType _parentRuleType;
    WeakPtr<CSSRule> _parentRule;

    StyleRuleCSSStyleProperties(MutableStyleProperties &propertySet, CSSRule &parentRule) noexcept;

  public:
    ~StyleRuleCSSStyleProperties() noexcept;

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }

    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

    KRYS_NODISCARD static Ref<StyleRuleCSSStyleProperties> Create(MutableStyleProperties &propertySet,
                                                                  CSSRule &parentRule) noexcept;

    void ClearParentRule() noexcept
    {
      _parentRule = nullptr;
    }

    void Reattach(MutableStyleProperties &propertySet) noexcept;

  private:
    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept final;
    KRYS_NODISCARD CSSRule *ParentRule() const noexcept final;

    KRYS_NODISCARD bool WillMutate() noexcept final;
    void DidMutate(MutationType type) noexcept final;
    KRYS_NODISCARD ParserContext CssParserContext() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSStyleProperties)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleDeclaration &declaration) noexcept
  {
    return declaration.GetStyleDeclarationType() == krys::boo::css::StyleDeclarationType::Style;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
