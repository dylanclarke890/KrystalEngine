#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"

namespace Krys::HTML
{
  class CSSMarginDescriptors;

  /// @see https://www.w3.org/TR/cssom-1/#cssmarginrule
  class CSSMarginRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSMarginRule);

  public:
    CSSMarginRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSMarginRule() noexcept override = default;

#pragma region CSSMarginRule - https://www.w3.org/TR/cssom-1/#cssmarginrule

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssmarginrule-name
    KRYS_NODISCARD CSSOMString Name() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssmarginrule-style
    KRYS_NODISCARD RefPtr<CSSMarginDescriptors> Style() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssmarginrule-style
    ExceptionOr<void> Style(const CSSOMString &text) noexcept;

#pragma endregion
  };
}