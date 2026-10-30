#pragma once

#include "Krystal.Booey/CSS/CSSStyleDeclaration.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class MutableStyleProperties;
  struct ParserContext;

  /// @brief The base of the descriptor blocks exposed to scripts (`@font-face`, `@page`, `@position-try` and
  /// `@function` declarations).
  class PropertySetCSSDescriptors : public CSSStyleDeclaration, public RefCounted<PropertySetCSSDescriptors>
  {
  protected:
    enum class MutationType : uint8
    {
      NoChanges,
      StyleAttributeChanged,
      PropertyChanged
    };

    WeakPtr<CSSRule> _parentRule;
    // FIXME: Replaced this with a more descriptor specific property map that doesn't have all the complexity
    // of the Style one.
    Ref<MutableStyleProperties> _propertySet;

    PropertySetCSSDescriptors(MutableStyleProperties &propertySet, CSSRule &parentRule) noexcept;

  public:
    virtual ~PropertySetCSSDescriptors() noexcept;

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

    void ClearParentRule() noexcept
    {
      _parentRule = nullptr;
    }

    void Reattach(MutableStyleProperties &propertySet) noexcept;

    KRYS_NODISCARD virtual RuleType GetRuleType() const noexcept = 0;

  protected:
    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept final;
    KRYS_NODISCARD CSSRule *ParentRule() const noexcept final;

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

    KRYS_NODISCARD bool IsExposed(PropertyId property) const noexcept;

    KRYS_NODISCARD bool WillMutate() noexcept;
    void DidMutate(MutationType type) noexcept;

    // PropertyId versions of the CSSOM functions to support bindings.
    KRYS_NODISCARD CSSOMString GetPropertyValueInternal(PropertyId property) const noexcept;
    virtual dom::ExceptionOr<void> SetPropertyInternal(PropertyId property, const CSSOMString &value,
                                                       IsImportant important) noexcept;

    KRYS_NODISCARD ParserContext CssParserContext() const noexcept;
    KRYS_NODISCARD Ref<MutableStyleProperties> ProtectedPropertySet() const noexcept;
  };
}
