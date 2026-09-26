#pragma once

#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSRule;
  class CSSStyleSheet;
  class MutableStyleProperties;
  class StyleProperties;
  class StyledElement;

  enum class StyleDeclarationType : uint8
  {
    Style,
    FontFace,
    Page,
    PositionTry,
    Function
  };

  /// @see https://www.w3.org/TR/cssom-1/#cssstyledeclaration
  class CSSStyleDeclaration : public AbstractRefCounted, public CanMakeWeakPtr<CSSStyleDeclaration>
  {
    KRYS_NON_COPYABLE(CSSStyleDeclaration);

  protected:
    CSSStyleDeclaration() noexcept = default;

  public:
    virtual ~CSSStyleDeclaration() noexcept;

    KRYS_NODISCARD virtual StyleDeclarationType DeclarationType() const noexcept = 0;

#pragma region CSSStyleDeclaration - https://www.w3.org/TR/cssom-1/#cssstyledeclaration

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-parentrule
    KRYS_NODISCARD virtual CSSRule *ParentRule() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-csstext
    KRYS_NODISCARD virtual CSSOMString CssText() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-csstext
    KRYS_NODISCARD virtual dom::ExceptionOr<void> CssText(const CSSOMString &text) noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-length
    KRYS_NODISCARD virtual size_t Length() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-item
    KRYS_NODISCARD virtual CSSOMString Item(size_t index) const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-getpropertyvalue
    KRYS_NODISCARD virtual CSSOMString GetPropertyValue(const CSSOMString &propertyName) const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-getpropertypriority
    KRYS_NODISCARD virtual CSSOMString
      GetPropertyPriority(const CSSOMString &propertyName) const noexcept = 0;


    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-setproperty
    KRYS_NODISCARD virtual dom::ExceptionOr<void> SetProperty(const CSSOMString &propertyName,
                                                              const CSSOMString &value,
                                                              const CSSOMString &priority) noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-removeproperty
    KRYS_NODISCARD virtual dom::ExceptionOr<CSSOMString>
      RemoveProperty(const CSSOMString &propertyName) noexcept = 0;

#pragma endregion

    KRYS_NODISCARD virtual CSSOMString
      GetPropertyShorthand(const CSSOMString &propertyName) const noexcept = 0;

    KRYS_NODISCARD virtual bool IsPropertyImplicit(const CSSOMString &propertyName) const noexcept = 0;

    KRYS_NODISCARD virtual StyledElement *ParentElement() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual CSSRule *CssRules() const noexcept = 0;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < Length();
    }

    KRYS_NODISCARD virtual CSSStyleSheet *ParentStyleSheet() const noexcept
    {
      return nullptr;
    }
  };
}
