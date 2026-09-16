#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class CrossFadeValue : public Value
  {
  public:
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CrossFadeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsCrossFadeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()