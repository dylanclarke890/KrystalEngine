#pragma once

#include "Krystal.HTML/CSS/CSSGroupingRule.hpp"

namespace Krys::HTML
{
  class CSSPageDescriptors;

  /// @see https://drafts.csswg.org/cssom/#the-csspagerule-interface
  class CSSPageRule : public CSSGroupingRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSPageRule);

  public:
    CSSPageRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSPageRule() noexcept override = default;

#pragma region CSSPageRule - https://drafts.csswg.org/cssom/#csspagerule

    /// @see https://drafts.csswg.org/cssom/#dom-csspagerule-selectortext
    KRYS_NODISCARD CSSOMString SelectorText() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-csspagerule-selectortext
    void SelectorText(const CSSOMString &text) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-csspagerule-style
    KRYS_NODISCARD RefPtr<CSSPageDescriptors> Style() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-csspagerule-style
    ExceptionOr<void> Style(const CSSOMString &text) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSPageRule)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSRule &rule) noexcept
  {
    return rule.IsCSSPageRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();