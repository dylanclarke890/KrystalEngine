#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.Booey/CSS/Values/Grid/CSSGridNamedAreaMap.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace krys::boo::css
{
  // <'grid-template-areas'> = none | <string>+
  // https://drafts.csswg.org/css-grid/#propdef-grid-template-areas
  struct GridTemplateAreas
  {
    GridNamedAreaMap map;

    GridTemplateAreas(keywords::None) noexcept : map {}
    {
    }

    GridTemplateAreas(const GridNamedAreaMap &map) noexcept : map {map}
    {
    }

    GridTemplateAreas(GridNamedAreaMap &&map) noexcept : map {krys::move(map)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !map.rowCount;
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(keywords::None {});
      }

      return visitor(map);
    }

    KRYS_NODISCARD bool operator==(const GridTemplateAreas &) const noexcept = default;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::GridTemplateAreas);