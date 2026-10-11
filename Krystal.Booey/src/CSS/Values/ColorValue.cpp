#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/Color/ResolvedColor.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys::boo::css
{
  ColorValue::ColorValue(css::Color color) noexcept : css::Value(ValueType::Color), _color(krys::move(color))
  {
  }

  ColorValue::ColorValue(boo::Color color) noexcept
      : ColorValue(css::Color {css::ResolvedColor {krys::move(color)}})
  {
  }

  Ref<ColorValue> ColorValue::Create(css::Color color) noexcept
  {
    return AdoptRef(*new ColorValue(krys::move(color)));
  }

  Ref<ColorValue> ColorValue::Create(boo::Color color) noexcept
  {
    return AdoptRef(*new ColorValue(krys::move(color)));
  }

  ColorValue::ColorValue(StaticValueTag, boo::Color color) noexcept : ColorValue(krys::move(color))
  {
    // TODO: copy what webkit does here, which is to increment separately for static values.
    AddRef();
  }

  boo::Color ColorValue::AbsoluteColor(const Value &value) noexcept
  {
    if (auto *color = DynamicDowncast<ColorValue>(value))
    {
      return color->Color().AbsoluteColor();
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

  IterationStatus
    ColorValue::CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept
  {
    return IterationStatus::Continue;
  }
}