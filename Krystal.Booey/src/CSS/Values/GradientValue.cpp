#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  CSSOMString GradientValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _gradient);
  }

  bool GradientValue::Equals(const GradientValue &other) const noexcept
  {
    return _gradient == other._gradient;
  }

  IterationStatus GradientValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _gradient);
  }
}
