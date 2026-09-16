#pragma once

#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class ColorSchemeValue : public Value
  {
  public:
    KRYS_NODISCARD static Ref<ColorSchemeValue> Create(ColorScheme scheme) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ColorSchemeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsColorSchemeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()