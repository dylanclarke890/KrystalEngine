#pragma once

#include "Krystal.Booey/CSS/Values/Grid/GridNamedAreaMap.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  // <'grid-template-areas'> = none | <string>+
  // https://drafts.csswg.org/css-grid/#propdef-grid-template-areas
  struct GridTemplateAreas
  {
    GridNamedAreaMap Map;

    GridTemplateAreas(keywords::None) noexcept : Map {}
    {
    }

    GridTemplateAreas(const GridNamedAreaMap &map) noexcept : Map {map}
    {
    }

    GridTemplateAreas(GridNamedAreaMap &&map) noexcept : Map {krys::move(map)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !Map.RowCount;
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(keywords::None {});
      }

      return visitor(Map);
    }

    KRYS_NODISCARD bool operator==(const GridTemplateAreas &) const noexcept = default;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::GridTemplateAreas)
