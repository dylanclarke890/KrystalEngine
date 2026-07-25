#pragma once
#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/CSS/Properties/CSSInternalStyleProperties.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelectorList.hpp"

namespace Krys::HTML
{
  class CSSStyleProperties;

  /// @see https://drafts.csswg.org/cssom/#the-cssstylerule-interface
  class CSSStyleRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSStyleRule);

  private:
    CSSSelectorList _selectors;
    Ref<CSSInternalStyleProperties> _properties;

  public:
    CSSStyleRule(CSSSelectorList &&selectors, Ref<CSSInternalStyleProperties> properties) noexcept
        : CSSRule(CSSRuleType::Style, nullptr), _selectors(Krys::Move(selectors)),
          _properties(Krys::Move(properties))
    {
    }

    ~CSSStyleRule() noexcept override = default;

#pragma region CSSStyleRule - https://drafts.csswg.org/cssom/#cssstylerule

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-selectortext
    KRYS_NODISCARD CSSOMString SelectorText() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-selectortext
    void SelectorText(const CSSOMString &text) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-style
    KRYS_NODISCARD RefPtr<CSSStyleProperties> Style() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-style
    ExceptionOr<void> Style(const CSSOMString &text) noexcept;

#pragma endregion

#pragma region CSSRule - https://drafts.csswg.org/cssom/#cssrule

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-csstext
    KRYS_NODISCARD CSSOMString CssText() const noexcept final
    {
      // TODO
      return {};
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSStyleRule)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSRule &rule) noexcept
  {
    return rule.IsCSSStyleRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();