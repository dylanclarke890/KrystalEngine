#pragma once

#include "Krystal.Booey/CSS/CSSRule.hpp"

namespace krys::boo::css
{
  class CSSMarginDescriptors;

  /// @see https://drafts.csswg.org/cssom/#the-cssmarginrule-interface
  class CSSMarginRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSMarginRule);

  public:
    CSSMarginRule(CSSStyleSheet *stylesheet) noexcept;

    ~CSSMarginRule() noexcept override = default;

#pragma region CSSMarginRule - https://drafts.csswg.org/cssom/#cssmarginrule

    /// @see https://drafts.csswg.org/cssom/#dom-cssmarginrule-name
    KRYS_NODISCARD CSSOMString Name() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssmarginrule-style
    KRYS_NODISCARD RefPtr<CSSMarginDescriptors> Style() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssmarginrule-style
    dom::ExceptionOr<void> Style(const CSSOMString &text) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMarginRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.IsCSSMarginRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();