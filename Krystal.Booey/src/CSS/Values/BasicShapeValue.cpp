#include "Krystal.Booey/CSS/Values/BasicShapeValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  Ref<BasicShapeValue> BasicShapeValue::Create(BasicShape shape) noexcept
  {
    return AdoptRef(*new BasicShapeValue(krys::move(shape)));
  }

  BasicShapeValue::BasicShapeValue(BasicShape &&shape) noexcept
      : Value(ValueType::BasicShape), _shape(krys::move(shape))
  {
  }

  CSSOMString BasicShapeValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _shape);
  }

  bool BasicShapeValue::Equals(const BasicShapeValue &other) const noexcept
  {
    return _shape == other._shape;
  }

  IterationStatus BasicShapeValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _shape);
  }
}
