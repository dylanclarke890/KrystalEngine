#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"

namespace krys::boo::css
{
  class CSSFunctionDescriptors;
  class FunctionDeclarationsRule;

  class CSSFunctionDeclarations final : public CSSRule
  {
  public:
    KRYS_NODISCARD static Ref<CSSFunctionDeclarations> Create(FunctionDeclarationsRule &rule,
                                                              CSSStyleSheet *sheet) noexcept
    {
      return AdoptRef(*new CSSFunctionDeclarations(rule, sheet));
    };

    virtual ~CSSFunctionDeclarations() noexcept;

    KRYS_NODISCARD CSSFunctionDescriptors &Style() noexcept;

  private:
    CSSFunctionDeclarations(FunctionDeclarationsRule &rule, CSSStyleSheet *sheet) noexcept;

    CSSOMString CssText() const noexcept final;
    CSSOMString CssTextInternal(CSSOMString &declarations, StringBuilder &rules) const noexcept;

    void Reattach(RuleBase &) noexcept final;

    RuleType Type() const noexcept final
    {
      return RuleType::FunctionDeclarations;
    }

    Ref<FunctionDeclarationsRule> _rule;
    RefPtr<CSSFunctionDescriptors> _descriptorsCSSOMWrapper;
  };
}

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSFunctionDeclarations, RuleType::FunctionDeclarations)
