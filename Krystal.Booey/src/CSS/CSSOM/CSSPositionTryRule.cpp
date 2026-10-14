#include "Krystal.Booey/CSS/CSSOM/CSSPositionTryRule.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSPositionTryDescriptors.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css
{
  CSSPositionTryRule::CSSPositionTryRule(css::PositionTryRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _positionTryRule(ShareRef(rule))
  {
  }

  CSSPositionTryRule::~CSSPositionTryRule() noexcept = default;

  Ref<CSSPositionTryRule> CSSPositionTryRule::Create(css::PositionTryRule &rule,
                                                     CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSPositionTryRule(rule, parent));
  }

  Ref<css::PositionTryRule> CSSPositionTryRule::ProtectedPositionTryRule() const noexcept
  {
    return _positionTryRule;
  }

  CSSOMString CSSPositionTryRule::CssText() const noexcept
  {
    CSSOMString builder;
    builder.append(u8"@position-try ").append(Name().View()).append(u8" {");

    auto properties = _positionTryRule->ProtectedProperties();
    if (auto declarations = properties->AsText(DefaultSerialisationContext()); !declarations.empty())
    {
      builder.append(u8" ").append(declarations).push_back(u8' ');
    }
    else
    {
      builder.push_back(u8' ');
    }

    builder.push_back(u8'}');
    return builder;
  }

  void CSSPositionTryRule::Reattach(Rule &rule) noexcept
  {
    _positionTryRule = ShareRef(Downcast<css::PositionTryRule>(rule));
    if (auto propertiesCSSOMWrapper = _propertiesCSSOMWrapper)
    {
      propertiesCSSOMWrapper->Reattach(*ProtectedPositionTryRule()->ProtectedMutableProperties());
    }
  }

  CSSOMStringAtom CSSPositionTryRule::Name() const noexcept
  {
    return _positionTryRule->Name();
  }

  CSSPositionTryDescriptors &CSSPositionTryRule::Style() noexcept
  {
    auto mutableProperties = ShareRef(ProtectedPositionTryRule()->MutableProperties());
    if (!_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper = CSSPositionTryDescriptors::Create(*mutableProperties, *this);
    }
    return *_propertiesCSSOMWrapper;
  }
}
