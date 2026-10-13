#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  Ref<RatioValue> RatioValue::Create(Ratio ratio) noexcept
  {
    return AdoptRef(*new RatioValue(krys::move(ratio)));
  }

  RatioValue::RatioValue(Ratio &&ratio) noexcept : Value(ValueType::Ratio), _ratio(krys::move(ratio))
  {
  }

  CSSOMString RatioValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _ratio);
  }

  bool RatioValue::Equals(const RatioValue &other) const noexcept
  {
    return _ratio == other._ratio;
  }

  IterationStatus
    RatioValue::CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _ratio);
  }
}
