#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSGridTemplateAreasValue final : public CSSValue
  {
  private:
    GridTemplateAreas _areas;

    explicit CSSGridTemplateAreasValue(GridTemplateAreas &&) noexcept;
    explicit CSSGridTemplateAreasValue(const GridTemplateAreas &) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSGridTemplateAreasValue> Create(GridTemplateAreas &&areas) noexcept;

    KRYS_NODISCARD static Ref<CSSGridTemplateAreasValue> Create(const GridTemplateAreas &areas) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSGridTemplateAreasValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSGridTemplateAreasValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()