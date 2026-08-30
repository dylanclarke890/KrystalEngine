#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Filter/AppleColorFilter.hpp"

namespace Krys::HTML
{
  class CSSAppleColorFilterValue final : public CSSValue
  {
  private:
    AppleColorFilter _filter;

    CSSAppleColorFilterValue(AppleColorFilter) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSAppleColorFilterValue> Create(AppleColorFilter filter) noexcept;

    KRYS_NODISCARD const AppleColorFilter &Filter() const noexcept
    {
      return _filter;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSAppleColorFilterValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSAppleColorFilterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()