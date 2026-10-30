#include "Krystal.Booey/CSS/Values/RectValue.hpp"

namespace krys::boo::css
{
  RectValue::RectValue(Rect rect) noexcept : Value(ValueType::Rect), _rect(krys::move(rect))
  {
  }

  Ref<RectValue> RectValue::Create(Rect rect) noexcept
  {
    return AdoptRef(*new RectValue(krys::move(rect)));
  }

  CSSOMString RectValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return _rect.CssText(context);
  }

  bool RectValue::Equals(const RectValue &other) const noexcept
  {
    return _rect.Equals(other._rect);
  }
}
