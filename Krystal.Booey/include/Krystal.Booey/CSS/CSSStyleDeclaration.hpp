#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSRule;

  /// @see https://www.w3.org/TR/cssom-1/#cssstyledeclaration
  class CSSStyleDeclaration : public RefCounted<CSSStyleDeclaration>
  {
  public:
#pragma region CSSStyleDeclaration - https://www.w3.org/TR/cssom-1/#cssstyledeclaration

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-csstext
    KRYS_NODISCARD CSSOMString CssText() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-csstext
    dom::ExceptionOr<void> CssText(const CSSOMString &text) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-length
    KRYS_NODISCARD size_t Length() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-item
    KRYS_NODISCARD const CSSOMString &Item(size_t index) const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-getpropertyvalue
    KRYS_NODISCARD CSSOMString GetPropertyValue(const CSSOMString &property) const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-getpropertypriority
    KRYS_NODISCARD CSSOMString GetPropertyPriority(const CSSOMString &property) const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-setproperty
    dom::ExceptionOr<void> SetProperty(const CSSOMString &property, const CSSOMString &value,
                                  const CSSOMString &priority = u8"") noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-removeproperty
    dom::ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &property) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-parentrule
    KRYS_NODISCARD RefPtr<CSSRule> ParentRule() const noexcept;

#pragma endregion
  };
}