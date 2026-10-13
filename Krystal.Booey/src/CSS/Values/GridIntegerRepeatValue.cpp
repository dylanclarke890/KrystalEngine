#include "Krystal.Booey/CSS/Values/GridIntegerRepeatValue.hpp"

namespace krys::boo::css
{
  GridIntegerRepeatValue::GridIntegerRepeatValue(Ref<PrimitiveValue> repetitions,
                                                 ValueListBuilder builder) noexcept
      : ValueContainingList(ValueType::GridIntegerRepeat, ValueSeparator::SpaceSeparator,
                            krys::move(builder)),
        _repetitions(krys::move(repetitions))
  {
  }

  Ref<GridIntegerRepeatValue> GridIntegerRepeatValue::Create(Ref<PrimitiveValue> repetitions,
                                                             ValueListBuilder builder) noexcept
  {
    return AdoptRef(*new GridIntegerRepeatValue(krys::move(repetitions), krys::move(builder)));
  }

  CSSOMString GridIntegerRepeatValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result {u8"repeat("};
    result.append(_repetitions->CssText(context));
    result.append(u8", ");
    SerialiseItems(result, context);
    result.push_back(u8')');
    return result;
  }

  bool GridIntegerRepeatValue::Equals(const GridIntegerRepeatValue &other) const noexcept
  {
    return CompareValue(_repetitions, other._repetitions) && ItemsEqual(other);
  }
}
