#include "Krystal.Booey/CSS/Values/BoxShadowPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  Ref<BoxShadowPropertyValue> BoxShadowPropertyValue::Create(BoxShadowProperty shadow) noexcept
  {
    return AdoptRef(*new BoxShadowPropertyValue(krys::move(shadow)));
  }

  BoxShadowPropertyValue::BoxShadowPropertyValue(BoxShadowProperty &&shadow) noexcept
      : Value(ValueType::BoxShadowProperty), _shadow(krys::move(shadow))
  {
  }

  CSSOMString BoxShadowPropertyValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _shadow);
  }

  bool BoxShadowPropertyValue::Equals(const BoxShadowPropertyValue &other) const noexcept
  {
    return _shadow == other._shadow;
  }

  IterationStatus BoxShadowPropertyValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _shadow);
  }
}
