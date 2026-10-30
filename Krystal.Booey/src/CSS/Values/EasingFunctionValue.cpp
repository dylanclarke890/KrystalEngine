#include "Krystal.Booey/CSS/Values/EasingFunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  Ref<EasingFunctionValue> EasingFunctionValue::Create(boo::css::EasingFunction easingFunction) noexcept
  {
    return AdoptRef(*new EasingFunctionValue(krys::move(easingFunction)));
  }

  EasingFunctionValue::EasingFunctionValue(boo::css::EasingFunction &&easingFunction) noexcept
      : Value(ValueType::EasingFunction), _easingFunction(krys::move(easingFunction))
  {
  }

  CSSOMString EasingFunctionValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _easingFunction);
  }

  bool EasingFunctionValue::Equals(const EasingFunctionValue &other) const noexcept
  {
    return _easingFunction == other._easingFunction;
  }

  IterationStatus EasingFunctionValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _easingFunction);
  }
}
