#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `grid-template-areas` value made of `<string>`s, e.g. `"a a" "b c"`.
  class GridTemplateAreasValue final : public Value
  {
  private:
    GridTemplateAreas _areas;

    explicit GridTemplateAreasValue(GridTemplateAreas &&areas) noexcept;

  public:
    KRYS_NODISCARD static Ref<GridTemplateAreasValue> Create(GridTemplateAreas areas) noexcept;

    KRYS_NODISCARD const GridTemplateAreas &Areas() const noexcept
    {
      return _areas;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const GridTemplateAreasValue &other) const noexcept;

    KRYS_NODISCARD CSSOMString StringForRow(size_t row) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GridTemplateAreasValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGridTemplateAreasValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
