#pragma once

#include "Krystal.Booey/Css/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class NestedDeclarationsRule;
}

namespace krys::boo
{
  class CSSStyleProperties;
  class StyleRuleCSSStyleProperties;

  /// @brief Declarations in a style rule that come after a nested rule, which keep their place among the
  /// nested rules, as scripts see them.
  /// @see https://drafts.csswg.org/css-nesting/#the-cssnestrule
  class CSSNestedDeclarations final : public CSSRule
  {
  private:
    Ref<css::NestedDeclarationsRule> _rule;
    RefPtr<StyleRuleCSSStyleProperties> _style;

    CSSNestedDeclarations(css::NestedDeclarationsRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSNestedDeclarations() noexcept override;

    KRYS_NODISCARD static Ref<CSSNestedDeclarations> Create(css::NestedDeclarationsRule &rule,
                                                            CSSStyleSheet *parent) noexcept;

    /// @brief The declarations, made when first asked for.
    /// @see https://drafts.csswg.org/css-nesting/#dom-cssnesteddeclarations-style
    KRYS_NODISCARD CSSStyleProperties &Style() noexcept;

  private:
    KRYS_NODISCARD css::RuleType StyleRuleType() const noexcept final
    {
      return css::RuleType::NestedDeclarations;
    }

    /// @brief The serialisation of the declarations, with nothing around them.
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    void Reattach(css::Rule &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CSSNestedDeclarations)
  KRYS_NODISCARD static bool IsType(const krys::boo::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::NestedDeclarations;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
