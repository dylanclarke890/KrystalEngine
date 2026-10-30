#include "Krystal.Booey/CSS/CSSOM/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

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

    StringBuilder result;
    result.Append(u8"@namespace ");
    if (!prefixIsEmpty)
    {
      SerialiseIdentifier(CSSOMString(prefix.View()), result);
    }
    result.Append(prefixIsEmpty ? u8"" : u8" ", u8"url(");
    result.Append(SerialiseString(CSSOMString(NamespaceURI().View())), u8");");
    return krys::move(result).ToString();
  }

  void CSSNamespaceRule::Reattach(Rule &) noexcept
  {
  }
}
