#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"

namespace Krys::HTML
{
  /// @see https://www.w3.org/TR/cssom-1/#cssnamespacerule
  class CSSNamespaceRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSNamespaceRule);

  private:
    CSSOMStringAtom _namespaceURI;
    CSSOMStringAtom _prefix;

  public:
    CSSNamespaceRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSNamespaceRule() noexcept override = default;

#pragma region CSSNamespaceRule - https://www.w3.org/TR/cssom-1/#cssnamespacerule

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssnamespacerule-namespaceuri
    KRYS_NODISCARD CSSOMStringAtom NamespaceURI() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssnamespacerule-prefix
    KRYS_NODISCARD CSSOMStringAtom Prefix() const noexcept;

#pragma endregion
  };
}