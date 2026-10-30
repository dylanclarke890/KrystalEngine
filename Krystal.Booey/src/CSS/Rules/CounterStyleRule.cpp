#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"

namespace krys::boo::css
{
  CounterStyleRule::CounterStyleRule(const CSSOMStringAtom &name,
                                     CounterStyleDescriptors &&descriptors) noexcept
      : Rule(RuleType::CounterStyle), _name(name), _descriptors(krys::move(descriptors))
  {
  }

  CounterStyleRule::~CounterStyleRule() noexcept = default;

  Ref<CounterStyleRule> CounterStyleRule::Create(const CSSOMStringAtom &name,
                                                 CounterStyleDescriptors &&descriptors) noexcept
  {
    return AdoptRef(*new CounterStyleRule(name, krys::move(descriptors)));
  }

  Ref<CounterStyleRule> CounterStyleRule::Copy() const noexcept
  {
    return AdoptRef(*new CounterStyleRule(*this));
  }
}
