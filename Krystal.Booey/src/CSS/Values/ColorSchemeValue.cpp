#include "Krystal.Booey/CSS/Values/ColorSchemeValue.hpp"

namespace krys::boo::css
{
  Ref<ColorSchemeValue> ColorSchemeValue::Create(css::ColorScheme colorScheme) noexcept
  {
    return AdoptRef(*new ColorSchemeValue(krys::move(colorScheme)));
  }

  ColorSchemeValue::ColorSchemeValue(css::ColorScheme colorScheme) noexcept
      : Value(ValueType::ColorScheme), _colorScheme(krys::move(colorScheme))
  {
  }

  CSSOMString ColorSchemeValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return css::Serialise(context, _colorScheme);
  }

  bool ColorSchemeValue::Equals(const ColorSchemeValue &other) const noexcept
  {
    return _colorScheme == other._colorScheme;
  }

  IterationStatus ColorSchemeValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(css::Value &)> &func) const noexcept
  {
    return css::VisitValueChildren(func, _colorScheme);
  }
}
