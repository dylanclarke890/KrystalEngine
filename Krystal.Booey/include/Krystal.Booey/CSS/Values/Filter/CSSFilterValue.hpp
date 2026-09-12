#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Filter/Filter.hpp"

namespace krys::boo::css
{
  class CSSFilterValue final : public CSSValue
  {
  private:
    Filter _filter;

    CSSFilterValue(Filter) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSFilterValue> Create(Filter filter) noexcept;

    KRYS_NODISCARD const Filter &Filter() const noexcept
    {
      return _filter;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFilterValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSFilterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()