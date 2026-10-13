#include "Krystal.Booey/CSS/Values/TextShadowPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  Ref<TextShadowPropertyValue> TextShadowPropertyValue::Create(TextShadowProperty shadow) noexcept
  {
    return AdoptRef(*new TextShadowPropertyValue(krys::move(shadow)));
  }

  TextShadowPropertyValue::TextShadowPropertyValue(TextShadowProperty &&shadow) noexcept
      : Value(ValueType::TextShadowProperty), _shadow(krys::move(shadow))
  {
  }

  CSSOMString TextShadowPropertyValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _shadow);
  }

  bool TextShadowPropertyValue::Equals(const TextShadowPropertyValue &other) const noexcept
  {
    return _shadow == other._shadow;
  }

  IterationStatus TextShadowPropertyValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _shadow);
  }
}
