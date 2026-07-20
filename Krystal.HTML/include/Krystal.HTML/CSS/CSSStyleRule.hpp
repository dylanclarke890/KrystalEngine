#pragma once
#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"

namespace Krys::HTML
{
  class CSSStyleProperties;

  /// @see https://drafts.csswg.org/cssom/#the-cssstylerule-interface
  class CSSStyleRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSStyleRule);

  public:
    CSSStyleRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSStyleRule() noexcept override = default;

#pragma region CSSStyleRule - https://drafts.csswg.org/cssom/#cssstylerule

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-selectortext
    KRYS_NODISCARD CSSOMString SelectorText() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-selectortext
    void SelectorText(const CSSOMString &text) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-style
    KRYS_NODISCARD RefPtr<CSSStyleProperties> Style() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-style
    ExceptionOr<void> Style(const CSSOMString& text) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSStyleRule)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSRule &rule) noexcept
  {
    return rule.IsCSSStyleRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();