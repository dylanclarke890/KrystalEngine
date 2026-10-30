#include "Krystal.Booey/CSS/Values/FontVariationValue.hpp"

namespace krys::boo::css
{
  FontVariationValue::FontVariationValue(FontTag tag, Ref<Value> value) noexcept
      : Value(ValueType::FontVariation), _tag(tag), _value(krys::move(value))
  {
  }

  CSSOMString FontVariationValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;
    result.push_back(u8'"');
    for (char c : _tag)
    {
      result.push_back(static_cast<char8_t>(c));
    }
    result.append(u8"\" ");
    result.append(_value->CssText(context));
    return result;
  }

  bool FontVariationValue::Equals(const FontVariationValue &other) const noexcept
  {
    return _tag == other._tag && CompareValue(_value, other._value);
  }
}
