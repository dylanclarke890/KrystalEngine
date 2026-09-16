#pragma once

#include "Krystal.Booey/CSS/Values/Motion/RayFunction.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class RayValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<RayValue> Create(RayFunction ray) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::RayValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsRayValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()