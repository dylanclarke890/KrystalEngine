#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"

namespace Krys::HTML
{
  /// @see https://drafts.csswg.org/cssom/#the-cssnamespacerule-interface
  class CSSNamespaceRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSNamespaceRule);

  private:
    CSSOMStringAtom _namespaceURI;
    CSSOMStringAtom _prefix;

  public:
    CSSNamespaceRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSNamespaceRule() noexcept override = default;

#pragma region CSSNamespaceRule - https://drafts.csswg.org/cssom/#cssnamespacerule

    /// @see https://drafts.csswg.org/cssom/#dom-cssnamespacerule-namespaceuri
    KRYS_NODISCARD CSSOMStringAtom NamespaceURI() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssnamespacerule-prefix
    KRYS_NODISCARD CSSOMStringAtom Prefix() const noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSNamespaceRule)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSRule &rule) noexcept
  {
    return rule.IsCSSNamespaceRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();