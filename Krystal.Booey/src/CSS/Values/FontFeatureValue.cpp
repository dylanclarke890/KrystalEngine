#include "Krystal.Booey/CSS/Values/FontFeatureValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css
{
  FontFeatureValue::FontFeatureValue(FontTag tag, Ref<Value> value) noexcept
      : Value(ValueType::FontFeature), _tag(tag), _value(krys::move(value))
  {
  }

  CSSOMString FontFeatureValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;
    result.push_back(u8'"');
    for (char c : _tag)
    {
      result.push_back(static_cast<char8_t>(c));
    }
    result.push_back(u8'"');
    // Omit the value if it's `1` as `1` is implied by default.
    if (auto *primitiveValue = DynamicDowncast<PrimitiveValue>(_value.get());
        !(primitiveValue && primitiveValue->ResolveAsIntegerIfNotCalculated<int>() == 1))
    {
      result.push_back(u8' ');
      result.append(_value->CssText(context));
    }
    return result;
  }

  bool FontFeatureValue::Equals(const FontFeatureValue &other) const noexcept
  {
    return _tag == other._tag && CompareValue(_value, other._value);
  }
}
