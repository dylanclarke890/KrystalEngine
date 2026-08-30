#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.HTML/CSS/Values/Grid/CSSGridNamedAreaMap.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // <'grid-template-areas'> = none | <string>+
  // https://drafts.csswg.org/css-grid/#propdef-grid-template-areas
  struct GridTemplateAreas
  {
    GridNamedAreaMap map;

    GridTemplateAreas(Keywords::None) noexcept : map {}
    {
    }

    GridTemplateAreas(const GridNamedAreaMap &map) noexcept : map {map}
    {
    }

    GridTemplateAreas(GridNamedAreaMap &&map) noexcept : map {Krys::Move(map)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !map.rowCount;
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = Krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(Keywords::None {});
      }

      return visitor(map);
    }

    KRYS_NODISCARD bool operator==(const GridTemplateAreas &) const noexcept = default;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(Krys::HTML::GridTemplateAreas);