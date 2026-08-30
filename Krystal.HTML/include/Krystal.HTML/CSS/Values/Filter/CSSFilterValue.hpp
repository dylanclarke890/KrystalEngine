#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Filter/Filter.hpp"

namespace Krys::HTML
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSFilterValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSFilterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()