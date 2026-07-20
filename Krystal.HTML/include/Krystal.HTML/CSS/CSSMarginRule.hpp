#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"

namespace Krys::HTML
{
  class CSSMarginDescriptors;

  /// @see https://drafts.csswg.org/cssom/#the-cssmarginrule-interface
  class CSSMarginRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSMarginRule);

  public:
    CSSMarginRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSMarginRule() noexcept override = default;

#pragma region CSSMarginRule - https://drafts.csswg.org/cssom/#cssmarginrule

    /// @see https://drafts.csswg.org/cssom/#dom-cssmarginrule-name
    KRYS_NODISCARD CSSOMString Name() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssmarginrule-style
    KRYS_NODISCARD RefPtr<CSSMarginDescriptors> Style() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssmarginrule-style
    ExceptionOr<void> Style(const CSSOMString &text) noexcept;

#pragma endregion
  };
}