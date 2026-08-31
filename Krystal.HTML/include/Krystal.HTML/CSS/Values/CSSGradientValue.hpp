#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSGradientValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSGradientValue> Create(Gradient gradient) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSGradientValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSGradientValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()