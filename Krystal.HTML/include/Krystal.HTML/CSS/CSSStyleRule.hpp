#pragma once
#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"

namespace Krys::HTML
{
  class CSSStyleDeclaration;

  /// @see https://www.w3.org/TR/cssom-1/#cssstylerule
  class CSSStyleRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSStyleRule);

  public:
    CSSStyleRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSStyleRule() noexcept override = default;

#pragma region CSSStyleRule - https://www.w3.org/TR/cssom-1/#cssstylerule

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssstylerule-selectortext
    KRYS_NODISCARD CSSOMString SelectorText() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssstylerule-selectortext
    void SelectorText(const CSSOMString &text) noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssstylerule-style
    KRYS_NODISCARD RefPtr<CSSStyleDeclaration> Style() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssstylerule-style
    ExceptionOr<void> Style(const CSSOMString& text) noexcept;

#pragma endregion
  };
}