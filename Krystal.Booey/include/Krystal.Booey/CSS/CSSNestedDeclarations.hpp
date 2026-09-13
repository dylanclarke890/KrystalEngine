#pragma once

#include "Krystal.Booey/CSS/CSSRule.hpp"
#include "Krystal.Booey/CSS/Parser/Context/ParsedRuleList.hpp"
#include "Krystal.Booey/CSS/Properties/InternalStyleProperties.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"

namespace krys::boo::css
{
  class CSSStyleProperties;

  /// @see https://drafts.csswg.org/css-nesting/#the-cssnestrule
  class CSSNestedDeclarations final : public CSSRule
  {
    friend class Parser;

    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSNestedDeclarations);

  private:
    SelectorList _selectors;
    RefPtr<InternalStyleProperties> _properties;
    ParsedRuleList _nestedRules;

  public:
    CSSNestedDeclarations(SelectorList &&selectors, Ref<InternalStyleProperties> properties,
                          ParsedRuleList &&nestedRules) noexcept
        : CSSRule(RuleType::Style, nullptr), _selectors(krys::move(selectors)),
          _properties(krys::move(properties)), _nestedRules(krys::move(nestedRules))
    {
    }

    CSSNestedDeclarations(Ref<InternalStyleProperties> properties) noexcept
        : CSSRule(RuleType::Style, nullptr), _properties(krys::move(properties))
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
    dom::ExceptionOr<void> Style(const CSSOMString &text) noexcept
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSNestedDeclarations)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.IsCSSNestedDeclarations();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();