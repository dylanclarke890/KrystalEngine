#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSGradientValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSGradientValue> Create(Gradient gradient) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSGradientValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSGradientValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()