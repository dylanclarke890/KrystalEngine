#pragma once

#include "Krystal.Booey/CSS/CSSRule.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.csswg.org/cssom/#the-cssmediarule-interface
  class CSSMediaRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSMediaRule);

  public:
    CSSMediaRule(CSSStyleSheet *stylesheet) noexcept;

    ~CSSMediaRule() noexcept override = default;

#pragma region CSSMediaRule - https://drafts.csswg.org/css-conditional-3/#cssmediarule

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMediaRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.IsCSSMediaRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();