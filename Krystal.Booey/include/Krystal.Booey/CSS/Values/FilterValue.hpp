#pragma once

#include "Krystal.Booey/CSS/Values/Filter/Filter.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class FilterValue final : public Value
  {
  private:
    Filter _filter;

    FilterValue(Filter filter) noexcept;

  public:
    KRYS_NODISCARD static Ref<FilterValue> Create(Filter filter) noexcept;

    KRYS_NODISCARD const Filter &Filter() const noexcept
    {
      return _filter;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FilterValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFilterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()