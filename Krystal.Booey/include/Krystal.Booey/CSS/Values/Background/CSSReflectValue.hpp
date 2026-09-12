#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSReflectValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSReflectValue> Create(CSSValueId direction, Ref<CSSValue> offset,
                                                      RefPtr<CSSValue> mask) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSReflectValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSReflectValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()