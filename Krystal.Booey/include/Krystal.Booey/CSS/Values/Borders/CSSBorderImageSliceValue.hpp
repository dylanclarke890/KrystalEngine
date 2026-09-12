#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSBorderImageSliceValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSBorderImageSliceValue> Create(Quad slices, bool fill) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSBorderImageSliceValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSBorderImageSliceValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()