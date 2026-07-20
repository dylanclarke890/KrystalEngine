#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"

namespace Krys::HTML
{
  class CSSRuleList;

  /// @see https://drafts.csswg.org/cssom/#the-cssgroupingrule-interface
  class CSSGroupingRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSGroupingRule);

  public:
    CSSGroupingRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSGroupingRule() noexcept override = default;

#pragma region CSSGroupingRule - https://drafts.csswg.org/cssom/#cssgroupingrule

    /// @see https://drafts.csswg.org/cssom/#dom-cssgroupingrule-cssrules
    KRYS_NODISCARD virtual RefPtr<CSSRuleList> CssRules() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-cssgroupingrule-insertrule
    ExceptionOr<uint32> InsertRule(const CSSOMString &rule, uint32 index) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssgroupingrule-deleterule
    ExceptionOr<void> DeleteRule(uint32 index) noexcept;

#pragma endregion
  };
}