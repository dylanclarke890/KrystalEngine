#include "Krystal.Web/Css/CSSOM/CSSNestedDeclarations.hpp"
#include "Krystal.Web/Css/CSSOM/CSSStyleProperties.hpp"
#include "Krystal.Web/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Web/Css/Properties/StyleProperties.hpp"
#include "Krystal.Web/Css/Rules/Rule.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"

namespace krys::web
{
  CSSNestedDeclarations::CSSNestedDeclarations(css::NestedDeclarationsRule &rule,
                                               CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _rule(ShareRef(rule))
  {
  }

  CSSNestedDeclarations::~CSSNestedDeclarations() noexcept = default;

  Ref<CSSNestedDeclarations> CSSNestedDeclarations::Create(css::NestedDeclarationsRule &rule,
                                                           CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSNestedDeclarations(rule, parent));
  }

  CSSStyleProperties &CSSNestedDeclarations::Style() noexcept
  {
    if (!_style)
    {
      _style = StyleRuleCSSStyleProperties::Create(_rule->MutableProperties(), *this);
    }
    return *_style;
  }

  CSSOMString CSSNestedDeclarations::CssText() const noexcept
  {
    return _rule->Properties().AsText(css::DefaultSerialisationContext());
  }

  void CSSNestedDeclarations::Reattach(css::Rule &rule) noexcept
  {
    _rule = ShareRef(Downcast<css::NestedDeclarationsRule>(rule));
    if (_style)
    {
      _style->Reattach(_rule->MutableProperties());
    }
  }
}
