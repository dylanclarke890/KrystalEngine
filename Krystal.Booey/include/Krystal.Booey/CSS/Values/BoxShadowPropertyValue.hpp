#pragma once

#include "Krystal.Booey/CSS/Values/Background/BoxShadowProperty.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class BoxShadowPropertyValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<BoxShadowPropertyValue> Create(BoxShadowProperty boxShadow) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BoxShadowPropertyValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBoxShadowPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()