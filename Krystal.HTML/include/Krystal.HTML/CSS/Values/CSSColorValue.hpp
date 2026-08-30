#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"

namespace Krys::HTML
{
  class CSSColorValue : public CSSValue
  {
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSColorValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSColorValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()