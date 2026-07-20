#pragma once
#pragma once

#include "Krystal.HTML/CSS/CSSGroupingRule.hpp"

namespace Krys::HTML
{
  class CSSPageDescriptors;

  /// @see https://www.w3.org/TR/cssom-1/#csspagerule
  class CSSPageRule : public CSSGroupingRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSPageRule);

  public:
    CSSPageRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSPageRule() noexcept override = default;

#pragma region CSSPageRule - https://www.w3.org/TR/cssom-1/#csspagerule

    /// @see https://www.w3.org/TR/cssom-1/#dom-csspagerule-selectortext
    KRYS_NODISCARD CSSOMString SelectorText() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-csspagerule-selectortext
    void SelectorText(const CSSOMString &text) noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-csspagerule-style
    KRYS_NODISCARD RefPtr<CSSPageDescriptors> Style() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-csspagerule-style
    ExceptionOr<void> Style(const CSSOMString &text) noexcept;

#pragma endregion
  };
}