#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"

namespace krys::boo::css
{
  PositionTryRule::PositionTryRule(CSSOMStringAtom &&name, Ref<StyleProperties> &&properties) noexcept
      : Rule(RuleType::PositionTry), _name(krys::move(name)), _properties(krys::move(properties))
  {
  }

  PositionTryRule::PositionTryRule(const PositionTryRule &other) noexcept
      : Rule(other), _name(other._name), _properties(other.ProtectedProperties()->MutableCopy())
  {
  }

  PositionTryRule::~PositionTryRule() noexcept = default;

  Ref<PositionTryRule> PositionTryRule::Create(CSSOMStringAtom &&name,
                                               Ref<StyleProperties> &&properties) noexcept
  {
    return AdoptRef(*new PositionTryRule(krys::move(name), krys::move(properties)));
  }

  Ref<PositionTryRule> PositionTryRule::Copy() const noexcept
  {
    return AdoptRef(*new PositionTryRule(*this));
  }

  Ref<StyleProperties> PositionTryRule::ProtectedProperties() const noexcept
  {
    return _properties;
  }

  MutableStyleProperties &PositionTryRule::MutableProperties() noexcept
  {
    if (!Is<MutableStyleProperties>(*_properties))
    {
      _properties = _properties->MutableCopy();
    }
    return Downcast<MutableStyleProperties>(*_properties);
  }

  Ref<MutableStyleProperties> PositionTryRule::ProtectedMutableProperties() noexcept
  {
    return Ref<MutableStyleProperties>::WithRef(MutableProperties());
  }
}
