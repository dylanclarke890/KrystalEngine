#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class OffsetRotateValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<OffsetRotateValue> Create(RefPtr<Value> modifier, RefPtr<Value> angle) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::OffsetRotateValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsOffsetRotateValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()