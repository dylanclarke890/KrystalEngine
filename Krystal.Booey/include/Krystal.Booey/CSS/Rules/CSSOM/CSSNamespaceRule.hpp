#pragma once

#include "Krystal.Booey/CSS/CSSRule.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.csswg.org/cssom/#the-cssnamespacerule-interface
  class CSSNamespaceRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSNamespaceRule);

  private:
    CSSOMStringAtom _namespaceURI;
    CSSOMStringAtom _prefix;

  public:
    CSSNamespaceRule(CSSOMStringAtom prefix, CSSOMStringAtom namespaceURI,
                     CSSStyleSheet *stylesheet) noexcept
        : CSSRule(RuleType::Namespace, stylesheet), _namespaceURI(krys::move(namespaceURI)),
          _prefix(krys::move(prefix))
    {
    }

    ~CSSNamespaceRule() noexcept override = default;

#pragma region CSSNamespaceRule - https://drafts.csswg.org/cssom/#cssnamespacerule

    /// @see https://drafts.csswg.org/cssom/#dom-cssnamespacerule-namespaceuri
    KRYS_NODISCARD CSSOMStringAtom NamespaceURI() const noexcept
    {
      return _namespaceURI;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-cssnamespacerule-prefix
    KRYS_NODISCARD CSSOMStringAtom Prefix() const noexcept
    {
      return _prefix;
    }

#pragma endregion

    
#pragma region CSSRule - https://drafts.csswg.org/cssom/#cssrule

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-csstext
    KRYS_NODISCARD CSSOMString CssText() const noexcept final
    {
      // TODO(CSSNamespaceRule): Implement CssText()
      return u8"";
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSNamespaceRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.IsCSSNamespaceRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();