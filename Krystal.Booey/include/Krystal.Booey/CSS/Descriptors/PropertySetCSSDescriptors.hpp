#pragma once

#include "Krystal.Booey/CSS/CSSStyleDeclaration.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  struct ParserContext;

  class PropertySetCSSDescriptors : public CSSStyleDeclaration, public RefCounted<PropertySetCSSDescriptors>
  {
  protected:
    WeakPtr<CSSRule> m_parentRule;
    // FIXME: Replace this with a more descriptor specific property map that doesn't have all the complexity
    // of the Style one.
    Ref<MutableStyleProperties> m_propertySet;

    PropertySetCSSDescriptors(MutableStyleProperties &properties, CSSRule &rule) noexcept;

  public:
    virtual ~PropertySetCSSDescriptors() noexcept;

    virtual RuleType RuleType() const noexcept = 0;

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }

    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }

    uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

    void Reattach(MutableStyleProperties &properties) noexcept;

    void ClearParentRule() noexcept
    {
      m_parentRule = nullptr;
    }

    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept final;

    KRYS_NODISCARD CSSRule *ParentRule() const noexcept final;

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

    KRYS_NODISCARD bool IsPropertyImplicit(const CSSOMString &propertyName) const noexcept final;

    dom::ExceptionOr<void> SetProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                       const CSSOMString &priority) noexcept final;

    dom::ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &propertyName) noexcept final;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    dom::ExceptionOr<void> CssText(const CSSOMString &) noexcept final;

    KRYS_NODISCARD bool IsExposed(PropertyId property) const noexcept;

    enum class MutationType : uint8
    {
      NoChanges,
      StyleAttributeChanged,
      PropertyChanged
    };

    KRYS_NODISCARD bool WillMutate() noexcept;

    void DidMutate(MutationType type) noexcept;

    // CSSPropertyID versions of the CSSOM functions to support bindings.
    KRYS_NODISCARD CSSOMString GetPropertyValueInternal(PropertyId property) const noexcept;

    KRYS_NODISCARD virtual dom::ExceptionOr<void>
      SetPropertyInternal(PropertyId property, const CSSOMString &value, IsImportant isImportant) noexcept;

    ParserContext CssParserContext() const;
  };
}
