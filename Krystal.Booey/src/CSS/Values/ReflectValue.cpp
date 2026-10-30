#include "Krystal.Booey/CSS/Values/ReflectValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  ReflectValue::ReflectValue(css::ValueId direction, Ref<Value> offset, RefPtr<Value> mask) noexcept
      : Value(ValueType::Reflect), _direction(direction), _offset(krys::move(offset)), _mask(krys::move(mask))
  {
  }

  Ref<ReflectValue> ReflectValue::Create(css::ValueId direction, Ref<Value> offset,
                                         RefPtr<Value> mask) noexcept
  {
    return AdoptRef(*new ReflectValue(direction, krys::move(offset), krys::move(mask)));
  }

  CSSOMString ReflectValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result {ToString(_direction)};
    result.push_back(u8' ');
    result.append(_offset->CssText(context));
    if (_mask)
    {
      result.push_back(u8' ');
      result.append(_mask->CssText(context));
    }
    return result;
  }

  bool ReflectValue::Equals(const ReflectValue &other) const noexcept
  {
    return _direction == other._direction && CompareValue(_offset, other._offset)
           && CompareValuePtr(_mask, other._mask);
  }
}
