#include "Krystal.Booey/CSS/CSSOM/CSSFunctionDeclarations.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSFunctionDescriptors.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/FunctionRule.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css
{
  CSSFunctionDeclarations::CSSFunctionDeclarations(FunctionDeclarationsRule &rule,
                                                   CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _styleRule(ShareRef(rule))
  {
  }

  CSSFunctionDeclarations::~CSSFunctionDeclarations() noexcept = default;

  Ref<CSSFunctionDeclarations> CSSFunctionDeclarations::Create(FunctionDeclarationsRule &rule,
                                                               CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSFunctionDeclarations(rule, sheet));
  }

  CSSFunctionDescriptors &CSSFunctionDeclarations::Style() noexcept
  {
    if (!_descriptorsCSSOMWrapper)
    {
      auto properties = ShareRef(_styleRule->MutableProperties());
      _descriptorsCSSOMWrapper = CSSFunctionDescriptors::Create(*properties, *this);
    }
    return *_descriptorsCSSOMWrapper;
  }

  CSSOMString CSSFunctionDeclarations::CssText() const noexcept
  {
    return _styleRule->Properties().AsText(DefaultSerialisationContext());
  }

  void CSSFunctionDeclarations::Reattach(Rule &rule) noexcept
  {
    _styleRule = ShareRef(Downcast<FunctionDeclarationsRule>(rule));
    if (_descriptorsCSSOMWrapper)
    {
      auto properties = ShareRef(_styleRule->MutableProperties());
      _descriptorsCSSOMWrapper->Reattach(*properties);
    }
  }
}
