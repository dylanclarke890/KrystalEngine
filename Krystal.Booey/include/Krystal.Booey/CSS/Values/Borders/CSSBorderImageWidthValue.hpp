#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSBorderImageWidthValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSBorderImageWidthValue> Create(Quad widths,
                                                               bool overridesBorderWidths) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSBorderImageWidthValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSBorderImageWidthValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()