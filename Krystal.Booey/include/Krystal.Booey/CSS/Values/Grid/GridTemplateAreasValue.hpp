#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class GridTemplateAreasValue final : public Value
  {
  private:
    GridTemplateAreas _areas;

    explicit GridTemplateAreasValue(GridTemplateAreas &&) noexcept;

    explicit GridTemplateAreasValue(const GridTemplateAreas &) noexcept;

  public:
    KRYS_NODISCARD static Ref<GridTemplateAreasValue> Create(GridTemplateAreas &&areas) noexcept;

    KRYS_NODISCARD static Ref<GridTemplateAreasValue> Create(const GridTemplateAreas &areas) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GridTemplateAreasValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGridTemplateAreasValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()