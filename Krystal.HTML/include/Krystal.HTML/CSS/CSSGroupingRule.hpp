#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"

namespace Krys::HTML
{
  class CSSRuleList;

  /// @see https://www.w3.org/TR/cssom-1/#cssgroupingrule
  class CSSGroupingRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSGroupingRule);

  public:
    CSSGroupingRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSGroupingRule() noexcept override = default;

#pragma region CSSGroupingRule - https://www.w3.org/TR/cssom-1/#cssgroupingrule

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssgroupingrule-cssrules
    KRYS_NODISCARD virtual RefPtr<CSSRuleList> CssRules() const noexcept = 0;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssgroupingrule-insertrule
    ExceptionOr<uint32> InsertRule(const CSSOMString &rule, uint32 index) noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssgroupingrule-deleterule
    ExceptionOr<void> DeleteRule(uint32 index) noexcept;

#pragma endregion
  };
}