#include "Krystal.Booey/CSS/Values/BackgroundRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  BackgroundRepeatValue::BackgroundRepeatValue(css::ValueId repeatXValue, css::ValueId repeatYValue) noexcept
      : css::Value(ValueType::BackgroundRepeat), _x(repeatXValue), _y(repeatYValue)
  {
  }

  Ref<BackgroundRepeatValue> BackgroundRepeatValue::Create(css::ValueId repeatX,
                                                           css::ValueId repeatY) noexcept
  {
    return AdoptRef(*new BackgroundRepeatValue(repeatX, repeatY));
  }

  CSSOMString BackgroundRepeatValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    // background-repeat/mask-repeat behaves like a shorthand, but `repeat no-repeat` is transformed to
    // `repeat-x`.
    if (_x != _y)
    {
      if (_x == ValueId::Repeat && _y == ValueId::NoRepeat)
      {
        return CSSOMString(ToString(ValueId::RepeatX));
      }

      if (_x == ValueId::NoRepeat && _y == ValueId::Repeat)
      {
        return CSSOMString(ToString(ValueId::RepeatY));
      }

      return CSSOMString(ToString(_x)) + u8" " + CSSOMString(ToString(_y));
    }

    return CSSOMString(ToString(_x));
  }

  bool BackgroundRepeatValue::Equals(const BackgroundRepeatValue &other) const noexcept
  {
    return _x == other._x && _y == other._y;
  }
}