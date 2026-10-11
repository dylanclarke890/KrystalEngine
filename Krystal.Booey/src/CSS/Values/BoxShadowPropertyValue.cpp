#include "Krystal.Booey/CSS/Values/BoxShadowPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  Ref<BoxShadowPropertyValue> BoxShadowPropertyValue::Create(BoxShadowProperty boxShadow) noexcept
  {
    return AdoptRef(*new BoxShadowPropertyValue(krys::move(boxShadow)));
  }

  CSSOMString BoxShadowPropertyValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return css::Serialise(context, _shadow);
  }

  bool BoxShadowPropertyValue::Equals(const BoxShadowPropertyValue &other) const noexcept
  {
    return _shadow == other._shadow;
  }

  IterationStatus BoxShadowPropertyValue::CustomVisitChildren(
    const Func<IterationStatus(css::Value &)> &func) const noexcept
  {
    return css::VisitValueChildren(func, _shadow);
  }
}