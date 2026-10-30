#include "Krystal.Booey/CSS/CSSOM/CSSNestedDeclarations.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css
{
  CSSNestedDeclarations::CSSNestedDeclarations(NestedDeclarationsRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _styleRule(ShareRef(rule))
  {
  }

  CSSNestedDeclarations::~CSSNestedDeclarations() noexcept = default;

  Ref<CSSNestedDeclarations> CSSNestedDeclarations::Create(NestedDeclarationsRule &rule,
                                                           CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSNestedDeclarations(rule, sheet));
  }

  CSSStyleProperties &CSSNestedDeclarations::Style() noexcept
  {
    if (!_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper = StyleRuleCSSStyleProperties::Create(_styleRule->MutableProperties(), *this);
    }
    return *_propertiesCSSOMWrapper;
  }

  CSSOMString CSSNestedDeclarations::CssText() const noexcept
  {
    return _styleRule->Properties().AsText(DefaultSerialisationContext());
  }

  void CSSNestedDeclarations::Reattach(Rule &rule) noexcept
  {
    _styleRule = ShareRef(Downcast<NestedDeclarationsRule>(rule));
    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->Reattach(_styleRule->MutableProperties());
    }
  }
}
