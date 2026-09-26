#pragma once

#include "Krystal.Booey/CSS/CSSStyleDeclaration.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
// #include <WebCore/StyledElement.h>
// #include <wtf/OptionalOrReference.h>

namespace krys::boo::css
{
  class StyleSheetContents;

  struct ParserContext;

  class CSSStyleProperties : public CSSStyleDeclaration
  {
  public:
    KRYS_NODISCARD StyleDeclarationType DeclarationType() const noexcept final
    {
      return StyleDeclarationType::Style;
    }

    KRYS_NODISCARD CSSOMString CssFloat() noexcept;
    dom::ExceptionOr<void> CssFloat(const CSSOMString &value) noexcept;

    /// @brief PropertyId versions of the CSSOM functions to support bindings and editing.
    KRYS_NODISCARD virtual CSSOMString GetPropertyValueInternal(PropertyId property) const noexcept = 0;

    virtual dom::ExceptionOr<void> SetPropertyInternal(PropertyId property, const CSSOMString &value,
                                                       IsImportant isImportant) noexcept = 0;

    KRYS_NODISCARD virtual Ref<MutableStyleProperties> CopyProperties() const noexcept = 0;

    // FIXME: It would be more efficient, by virtue of avoiding the text transformation and hash lookup
    // currently required in the implementation, if we could could smuggle the CSSPropertyID through the
    // bindings, perhaps by encoding it into the HashTableValue and then passing it together with the
    // PropertyName.

    // Shared implementation for all properties that match
    // https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-camel_cased_attribute.
    KRYS_NODISCARD CSSOMString
      PropertyValueForCamelCasedIDLAttribute(const CSSOMStringAtom &attribute) noexcept;
    dom::ExceptionOr<void> SetPropertyValueForCamelCasedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                     const CSSOMString &value) noexcept;

    // Shared implementation for all properties that match
    // https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-webkit_cased_attribute.
    KRYS_NODISCARD CSSOMString
      PropertyValueForWebKitCasedIDLAttribute(const CSSOMStringAtom &attribute) noexcept;

    dom::ExceptionOr<void> SetPropertyValueForWebKitCasedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                      const CSSOMString &value);

    // Shared implementation for all properties that match
    // https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-dashed_attribute.
    KRYS_NODISCARD CSSOMString PropertyValueForDashedIDLAttribute(const CSSOMStringAtom &attribute) noexcept;

    dom::ExceptionOr<void> SetPropertyValueForDashedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                 const CSSOMString &value) noexcept;

    // Shared implementation for all properties that match non-standard Epub-cased.
    KRYS_NODISCARD CSSOMString
      PropertyValueForEpubCasedIDLAttribute(const CSSOMStringAtom &attribute) noexcept;
    dom::ExceptionOr<void> SetPropertyValueForEpubCasedIDLAttribute(const CSSOMStringAtom &attribute,
                                                                    const CSSOMString &value) noexcept;
  };

  class PropertySetCSSStyleProperties : public CSSStyleProperties
  {
  public:
    explicit PropertySetCSSStyleProperties(MutableStyleProperties &propertySet) : m_propertySet(&propertySet)
    {
    }

    void AddRef() const noexcept override;

    void SubRef() const noexcept override;

    KRYS_NODISCARD StyleSheetContents *ContextStyleSheet() const noexcept;

  protected:
    enum class MutationType : uint8_t
    {
      NoChanges,
      StyleAttributeChanged,
      PropertyChanged
    };

    virtual ParserContext *CssParserContext() const;

    MutableStyleProperties *m_propertySet;

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

    KRYS_NODISCARD size_t Length() const noexcept final;

    KRYS_NODISCARD CSSOMString Item(size_t index) const noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyValue(const CSSOMString &propertyName) const noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyPriority(const CSSOMString &propertyName) const noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyShorthand(const CSSOMString &propertyName) const noexcept final;

    bool IsPropertyImplicit(const CSSOMString &propertyName) const noexcept final;

    dom::ExceptionOr<void> SetProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                       const CSSOMString &priority) noexcept final;

    dom::ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &propertyName) noexcept final;

    CSSOMString CssText() const noexcept final;
    dom::ExceptionOr<void> CssText(const CSSOMString &text) noexcept final;

    CSSOMString GetPropertyValueInternal(PropertyId property) const noexcept final;

    dom::ExceptionOr<void> SetPropertyInternal(PropertyId property, const CSSOMString &value,
                                               IsImportant) noexcept final;

    Ref<MutableStyleProperties> CopyProperties() const noexcept final;

    KRYS_NODISCARD bool IsExposed(PropertyId property) const noexcept;

    KRYS_NODISCARD virtual bool WillMutate() const noexcept
    {
      return true;
    }

    virtual void DidMutate(MutationType) noexcept
    {
    }
  };

  class StyleRuleCSSStyleProperties final : public PropertySetCSSStyleProperties,
                                            public RefCounted<StyleRuleCSSStyleProperties>
  {
  private:
    RuleType _parentRuleType;
    WeakPtr<CSSRule> _parentRule;

    StyleRuleCSSStyleProperties(MutableStyleProperties &properties, CSSRule &rule) noexcept;

  public:
    virtual ~StyleRuleCSSStyleProperties() noexcept;

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
                                                                  CSSRule &parentRule) noexcept
    {
      return AdoptRef(*new StyleRuleCSSStyleProperties(propertySet, parentRule));
    }

    void ClearParentRule()
    {
      _parentRule = nullptr;
    }

    void Reattach(MutableStyleProperties &);

  private:
    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept final;

    KRYS_NODISCARD CSSRule *ParentRule() const noexcept final;

    KRYS_NODISCARD bool WillMutate() const noexcept final;

    void DidMutate(MutationType type) noexcept final;
  };

  class InlineCSSStyleProperties final : public PropertySetCSSStyleProperties
  {
  private:
    WeakPtr<StyledElement> _parentElement;

  public:
    InlineCSSStyleProperties(MutableStyleProperties &propertySet, StyledElement &parentElement)
        : PropertySetCSSStyleProperties(propertySet), _parentElement(CreateWeakPtr(&parentElement))
    {
    }

  private:
    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept final;

    KRYS_NODISCARD StyledElement *ParentElement() const noexcept final
    {
      return _parentElement.get();
    }

    KRYS_NODISCARD bool WillMutate() const noexcept final;

    void DidMutate(MutationType type) noexcept final;
  };

}

SPECIALIZE_TYPE_TRAITS_CSS_STYLE_DECLARATION(CSSStyleProperties, StyleDeclarationType::Style)
