#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSGridTemplateAreasValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSGridTemplateAreasValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()