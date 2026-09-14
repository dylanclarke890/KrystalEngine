#pragma once

#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css
{
  class TransformListValue final : public ValueWithSmallList
  {
  public:
    KRYS_NODISCARD static Ref<TransformListValue> Create(ValueListBuilder) noexcept;

    KRYS_NODISCARD static Ref<TransformListValue> Create(Ref<Value>) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::TransformListValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsTransformListValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()