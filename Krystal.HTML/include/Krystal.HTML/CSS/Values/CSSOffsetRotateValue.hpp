#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSOffsetRotateValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSOffsetRotateValue> Create(RefPtr<CSSValue> modifier,
                                                           RefPtr<CSSValue> angle) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSOffsetRotateValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSOffsetRotateValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()