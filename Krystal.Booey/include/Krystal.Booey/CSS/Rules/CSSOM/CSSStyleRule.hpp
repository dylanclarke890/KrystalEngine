#pragma once
#pragma once

#include "Krystal.Booey/CSS/CSSRule.hpp"
#include "Krystal.Booey/CSS/Properties/InternalStyleProperties.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"

namespace krys::boo::css
{
  class CSSStyleProperties;

  /// @see https://drafts.csswg.org/cssom/#the-cssstylerule-interface
  class CSSStyleRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSStyleRule);

  private:
    SelectorList _selectors;
    Ref<InternalStyleProperties> _properties;

  public:
    CSSStyleRule(SelectorList &&selectors, Ref<InternalStyleProperties> properties) noexcept
        : CSSRule(RuleType::Style, nullptr), _selectors(krys::move(selectors)),
          _properties(krys::move(properties))
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
    dom::ExceptionOr<void> Style(const CSSOMString &text) noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSStyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.IsCSSStyleRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();