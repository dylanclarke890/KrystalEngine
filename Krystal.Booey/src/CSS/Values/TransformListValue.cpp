#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"

namespace krys::boo::css
{
  TransformListValue::TransformListValue(ValueListBuilder builder) noexcept
      : ValueContainingList(ValueType::TransformList, ValueSeparator::SpaceSeparator, krys::move(builder))
  {
  }

  TransformListValue::TransformListValue(Ref<Value> value) noexcept
      : ValueContainingList(ValueType::TransformList, ValueSeparator::SpaceSeparator, krys::move(value))
  {
  }

  Ref<TransformListValue> TransformListValue::Create(ValueListBuilder builder) noexcept
  {
    return AdoptRef(*new TransformListValue(krys::move(builder)));
  }

  Ref<TransformListValue> TransformListValue::Create(Ref<Value> value) noexcept
  {
    return AdoptRef(*new TransformListValue(krys::move(value)));
  }
}
