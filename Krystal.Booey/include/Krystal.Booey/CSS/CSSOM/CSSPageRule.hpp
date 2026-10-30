#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSPageDescriptors;
  class PageRule;

  /// @brief An `@page` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#the-csspagerule-interface
  class CSSPageRule final : public CSSRule
  {
  private:
    Ref<PageRule> _pageRule;
    mutable RefPtr<CSSPageDescriptors> _propertiesCSSOMWrapper;

    CSSPageRule(PageRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSPageRule() noexcept;

    KRYS_NODISCARD static Ref<CSSPageRule> Create(PageRule &rule, CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSPageDescriptors &Style() noexcept;

    KRYS_NODISCARD CSSOMString SelectorText() const noexcept;
    void SetSelectorText(const CSSOMString &selectorText) noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Page;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSPageRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Page;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
