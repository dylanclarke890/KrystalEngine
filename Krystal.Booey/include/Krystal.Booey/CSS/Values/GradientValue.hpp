#pragma once

#include "Krystal.Booey/CSS/Values/Images/Gradient.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class GradientValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<GradientValue> Create(Gradient gradient) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GradientValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGradientValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()