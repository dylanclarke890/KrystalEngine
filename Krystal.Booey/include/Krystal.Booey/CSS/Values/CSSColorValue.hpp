#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"

namespace krys::boo::css
{
  class CSSColorValue : public CSSValue
  {
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSColorValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSColorValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()