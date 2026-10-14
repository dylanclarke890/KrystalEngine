#include "Krystal.Booey/CSS/CSSOM/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  CSSNamespaceRule::CSSNamespaceRule(NamespaceRule &namespaceRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _namespaceRule(ShareRef(namespaceRule))
  {
  }

  CSSNamespaceRule::~CSSNamespaceRule() noexcept = default;

  Ref<CSSNamespaceRule> CSSNamespaceRule::Create(NamespaceRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSNamespaceRule(rule, sheet));
  }

  CSSOMStringAtom CSSNamespaceRule::NamespaceURI() const noexcept
  {
    return _namespaceRule->Uri();
  }

  CSSOMStringAtom CSSNamespaceRule::Prefix() const noexcept
  {
    return _namespaceRule->Prefix();
  }

  CSSOMString CSSNamespaceRule::CssText() const noexcept
  {
    auto prefix = Prefix();
    auto prefixIsEmpty = prefix == CSSOMStringAtom::Null() || prefix.View().empty();

    CSSOMString result;
    result.append(u8"@namespace ");
    if (!prefixIsEmpty)
    {
      SerialiseIdentifier(CSSOMString(prefix.View()), result);
    }
    result.append(prefixIsEmpty ? u8"" : u8" ").append(u8"url(");
    result.append(SerialiseString(CSSOMString(NamespaceURI().View()))).append(u8");");
    return result;
  }

  void CSSNamespaceRule::Reattach(Rule &) noexcept
  {
  }
}
