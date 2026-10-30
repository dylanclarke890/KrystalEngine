#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class ViewTransitionRule;

  /// @brief An `@view-transition` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-view-transitions-2/#the-cssviewtransitionrule-interface
  class CSSViewTransitionRule final : public CSSRule
  {
  private:
    Ref<css::ViewTransitionRule> _viewTransitionRule;

    CSSViewTransitionRule(css::ViewTransitionRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSViewTransitionRule() noexcept;

    KRYS_NODISCARD static Ref<CSSViewTransitionRule> Create(css::ViewTransitionRule &rule,
                                                            CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;

    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::ViewTransition;
    }

    KRYS_NODISCARD CSSOMStringAtom Navigation() const noexcept;
    KRYS_NODISCARD List<CSSOMStringAtom> Types() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSViewTransitionRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::ViewTransition;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
