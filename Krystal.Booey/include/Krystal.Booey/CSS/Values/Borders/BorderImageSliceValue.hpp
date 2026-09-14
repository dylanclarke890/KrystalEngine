#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class BorderImageSliceValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<BorderImageSliceValue> Create(Quad slices, bool fill) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BorderImageSliceValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBorderImageSliceValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()