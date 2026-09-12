#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Filter/AppleColorFilter.hpp"

namespace krys::boo::css
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSAppleColorFilterValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSAppleColorFilterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()