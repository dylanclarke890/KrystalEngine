#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Ratio.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class RatioValue : public Value
  {
  public:
    KRYS_NODISCARD static Ref<RatioValue> Create(Ratio ratio) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::RatioValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsRatioValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()