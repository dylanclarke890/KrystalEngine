#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSURL.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSURLValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSURLValue> Create(const CSSURL &url) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSURLValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSURLValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()