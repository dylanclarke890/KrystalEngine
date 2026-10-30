#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ResolvedColor.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys::boo::css
{
  Ref<ColorValue> ColorValue::Create(css::Color color) noexcept
  {
    return AdoptRef(*new ColorValue(krys::move(color)));
  }

  Ref<ColorValue> ColorValue::Create(boo::Color color) noexcept
  {
    return AdoptRef(*new ColorValue(krys::move(color)));
  }

  ColorValue::ColorValue(css::Color color) noexcept : css::Value(ValueType::Color), _color(krys::move(color))
  {
  }

  ColorValue::ColorValue(boo::Color color) noexcept
      : ColorValue(css::Color {css::ResolvedColor {krys::move(color)}})
  {
  }

  ColorValue::ColorValue(StaticValueTag, boo::Color color) noexcept : ColorValue(krys::move(color))
  {
    // FIXME: Make static values like upstream (a reference count reserved for static values that are never
    // destroyed). For now, the extra reference is never released.
    AddRef();
  }

  boo::Color ColorValue::GetAbsoluteColor(const Value &value) noexcept
  {
    if (auto *color = DynamicDowncast<ColorValue>(value))
    {
      return color->GetColor().GetAbsoluteColor();
    }

    if (auto valueId = value.ValueId(); IsAbsoluteColorKeyword(valueId))
    {
      return ColorFromAbsoluteKeyword(valueId);
    }
    return {};
  }

  CSSOMString ColorValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return css::Serialise(context, _color);
  }

  bool ColorValue::Equals(const ColorValue &other) const noexcept
  {
    return _color == other._color;
  }

  IterationStatus ColorValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(css::Value &)> &func) const noexcept
  {
    return css::VisitValueChildren(func, _color);
  }
}
