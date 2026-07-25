#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/CSS/Properties/CSSInternalStyleProperties.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelectorList.hpp"

namespace Krys::HTML
{
  class CSSStyleProperties;

  /// @see https://drafts.csswg.org/css-nesting/#the-cssnestrule
  class CSSNestedDeclarations final : public CSSRule
  {
    friend class CSSParser;

    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSNestedDeclarations);

  private:
    CSSSelectorList _selectors;
    RefPtr<CSSInternalStyleProperties> _properties;
    ParsedCSSRuleList _nestedRules;

  public:
    CSSNestedDeclarations(CSSSelectorList &&selectors, Ref<CSSInternalStyleProperties> properties,
                          ParsedCSSRuleList &&nestedRules) noexcept
        : CSSRule(CSSRuleType::Style, nullptr), _selectors(Krys::Move(selectors)),
          _properties(Krys::Move(properties)), _nestedRules(Krys::Move(nestedRules))
    {
    }

    CSSNestedDeclarations(Ref<CSSInternalStyleProperties> properties) noexcept
        : CSSRule(CSSRuleType::Style, nullptr), _properties(Krys::Move(properties))
    {
    }

    ~CSSNestedDeclarations() noexcept override = default;

#pragma region CSSNestedDeclarations - https://drafts.csswg.org/css-nesting/#cssnesteddeclarations

    /// @see https://drafts.csswg.org/css-nesting/#dom-cssnesteddeclarations-style
    KRYS_NODISCARD RefPtr<CSSStyleProperties> Style() const noexcept
    {
      // TODO
      return {};
    }

    /// @see https://drafts.csswg.org/css-nesting/#dom-cssnesteddeclarations-style
    ExceptionOr<void> Style(const CSSOMString &text) noexcept
    {
      // TODO
      return {};
    }

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSNestedDeclarations)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSRule &rule) noexcept
  {
    return rule.IsCSSNestedDeclarations();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();