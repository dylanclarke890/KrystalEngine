#include "Krystal.Booey/Css/CSSOM/CSSNestedDeclarations.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSStyleProperties.hpp"
#include "Krystal.Booey/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/Css/Properties/StyleProperties.hpp"
#include "Krystal.Booey/Css/Rules/Rule.hpp"
#include "Krystal.Booey/Css/Values/SerialisationContext.hpp"

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
