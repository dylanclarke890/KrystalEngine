#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleProperties;
  class NestedDeclarationsRule;
  class StyleRuleCSSStyleProperties;

  /// @brief Declarations that follow nested rules, exposed to scripts.
  /// @see https://drafts.csswg.org/css-nesting/#the-cssnestrule
  // FIXME: styleMap (DeclaredStylePropertyMap) comes with the Typed OM.
  class CSSNestedDeclarations final : public CSSRule
  {
  private:
    Ref<NestedDeclarationsRule> _styleRule;
    RefPtr<StyleRuleCSSStyleProperties> _propertiesCSSOMWrapper;

    CSSNestedDeclarations(NestedDeclarationsRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSNestedDeclarations() noexcept;

    KRYS_NODISCARD static Ref<CSSNestedDeclarations> Create(NestedDeclarationsRule &rule,
                                                            CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSStyleProperties &Style() noexcept;

  private:
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::NestedDeclarations;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSNestedDeclarations)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::NestedDeclarations;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
