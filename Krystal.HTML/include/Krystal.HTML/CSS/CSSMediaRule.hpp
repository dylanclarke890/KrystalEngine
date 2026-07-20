#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"

namespace Krys::HTML
{
  /// @see https://drafts.csswg.org/cssom/#the-cssmediarule-interface
  class CSSMediaRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSMediaRule);

  public:
    CSSMediaRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSMediaRule() noexcept override = default;

#pragma region CSSMediaRule - https://drafts.csswg.org/css-conditional-3/#cssmediarule

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSMediaRule)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSRule &rule) noexcept
  {
    return rule.IsCSSMediaRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();