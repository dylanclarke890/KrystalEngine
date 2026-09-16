#pragma once

#include "Krystal.Booey/CSS/Values/Filter/AppleColorFilter.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class AppleColorFilterValue final : public Value
  {
  private:
    AppleColorFilter _filter;

    AppleColorFilterValue(AppleColorFilter) noexcept;

  public:
    KRYS_NODISCARD static Ref<AppleColorFilterValue> Create(AppleColorFilter filter) noexcept;

    KRYS_NODISCARD const AppleColorFilter &Filter() const noexcept
    {
      return _filter;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::AppleColorFilterValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsAppleColorFilterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()