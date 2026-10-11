#include "Krystal.Booey/CSS/Values/ColorSchemeValue.hpp"

namespace krys::boo::css
{
  Ref<ColorSchemeValue> ColorSchemeValue::Create(ColorScheme scheme) noexcept
  {
    return AdoptRef(*new ColorSchemeValue(scheme));
  }

  CSSOMString ColorSchemeValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return css::Serialise(context, _scheme);
  }

  bool ColorSchemeValue::Equals(const ColorSchemeValue &other) const noexcept
  {
    return _scheme == other._scheme;
  }

  IterationStatus
    ColorSchemeValue::CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept
  {
    return css::VisitValueChildren(func, _scheme);
  }
}