#include "Krystal.Booey/CSS/Values/GridIntegerRepeatValue.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

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
    StringBuilder result;
    result.Append(u8"repeat(");
    result.Append(_repetitions->CssText(context));
    result.Append(u8", ");
    SerialiseItems(result, context);
    result.Append(u8')');
    return krys::move(result).ToString();
  }

  bool GridIntegerRepeatValue::Equals(const GridIntegerRepeatValue &other) const noexcept
  {
    return CompareValue(_repetitions, other._repetitions) && ItemsEqual(other);
  }
}
