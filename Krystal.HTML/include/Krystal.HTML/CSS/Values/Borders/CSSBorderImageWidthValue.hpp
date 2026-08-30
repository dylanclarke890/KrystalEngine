#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSBorderImageWidthValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSBorderImageWidthValue> Create(Quad widths,
                                                               bool overridesBorderWidths) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSBorderImageWidthValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSBorderImageWidthValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()