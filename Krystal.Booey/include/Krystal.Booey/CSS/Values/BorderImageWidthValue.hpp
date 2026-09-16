#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class BorderImageWidthValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<BorderImageWidthValue> Create(Quad widths, bool overridesBorderWidths) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BorderImageWidthValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBorderImageWidthValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()