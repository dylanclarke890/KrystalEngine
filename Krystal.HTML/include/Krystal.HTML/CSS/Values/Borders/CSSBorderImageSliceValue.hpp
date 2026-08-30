#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSBorderImageSliceValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSBorderImageSliceValue> Create(Quad slices, bool fill) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSBorderImageSliceValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSBorderImageSliceValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()