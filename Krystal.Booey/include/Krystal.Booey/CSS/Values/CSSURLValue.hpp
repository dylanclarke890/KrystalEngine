#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSURL.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSURLValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSURLValue> Create(const CSSURL &url) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSURLValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSURLValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()