#include "Krystal.Booey/CSS/Values/BackgroundRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  BackgroundRepeatValue::BackgroundRepeatValue(css::ValueId xValue, css::ValueId yValue) noexcept
      : Value(ValueType::BackgroundRepeat), _xValue(xValue), _yValue(yValue)
  {
  }

  Ref<BackgroundRepeatValue> BackgroundRepeatValue::Create(css::ValueId repeatXValue,
                                                           css::ValueId repeatYValue) noexcept
  {
    return AdoptRef(*new BackgroundRepeatValue(repeatXValue, repeatYValue));
  }

  CSSOMString BackgroundRepeatValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    // background-repeat/mask-repeat behave a little like a shorthand, but `repeat no-repeat` is transformed
    // to `repeat-x`.
    if (_xValue != _yValue)
    {
      if (_xValue == ValueId::Repeat && _yValue == ValueId::NoRepeat)
      {
        return CSSOMString(ToString(ValueId::RepeatX));
      }
      if (_xValue == ValueId::NoRepeat && _yValue == ValueId::Repeat)
      {
        return CSSOMString(ToString(ValueId::RepeatY));
      }

      CSSOMString result {ToString(_xValue)};
      result.push_back(u8' ');
      result.append(ToString(_yValue));
      return result;
    }
    return CSSOMString(ToString(_xValue));
  }

  bool BackgroundRepeatValue::Equals(const BackgroundRepeatValue &other) const noexcept
  {
    return _xValue == other._xValue && _yValue == other._yValue;
  }
}
