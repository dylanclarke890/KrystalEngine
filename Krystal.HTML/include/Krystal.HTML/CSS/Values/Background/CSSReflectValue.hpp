#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSReflectValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSReflectValue> Create(CSSValueId direction, Ref<CSSValue> offset,
                                                      RefPtr<CSSValue> mask) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSReflectValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSReflectValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()