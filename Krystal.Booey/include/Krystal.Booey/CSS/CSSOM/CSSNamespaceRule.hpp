#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class NamespaceRule;

  /// @brief An `@namespace` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#the-cssnamespacerule-interface
  class CSSNamespaceRule final : public CSSRule
  {
  private:
    Ref<NamespaceRule> _namespaceRule;

    CSSNamespaceRule(NamespaceRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSNamespaceRule() noexcept;

    KRYS_NODISCARD static Ref<CSSNamespaceRule> Create(NamespaceRule &rule, CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSOMStringAtom NamespaceURI() const noexcept;
    KRYS_NODISCARD CSSOMStringAtom Prefix() const noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Namespace;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    void Reattach(Rule &rule) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSNamespaceRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Namespace;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
