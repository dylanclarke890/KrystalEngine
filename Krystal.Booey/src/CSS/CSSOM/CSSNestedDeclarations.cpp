#include "Krystal.Booey/CSS/CSSOM/CSSNestedDeclarations.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo
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
