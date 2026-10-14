#pragma once

#include "Krystal.Booey/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridNamedAreaMap.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridNamedLinesMap.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"

namespace krys::boo::style
{
  // <'grid-template-areas'> = none | <string>+
  // https://drafts.csswg.org/css-grid/#propdef-grid-template-areas
  struct GridTemplateAreas
  {
    GridNamedAreaMap Map {};
    GridNamedLinesMap ImplicitNamedGridColumnLines {};
    GridNamedLinesMap ImplicitNamedGridRowLines {};

    GridTemplateAreas(css::keywords::None) noexcept
    {
    }
    GridTemplateAreas(GridNamedAreaMap &&map) noexcept;
    GridTemplateAreas(const GridNamedAreaMap &map) noexcept;

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
        return visitor(css::keywords::None {});
      }
      return visitor(Map);
    }

    KRYS_NODISCARD size_t Count(GridTrackSizingDirection direction) const noexcept
    {
      return direction == GridTrackSizingDirection::Columns ? Map.ColumnCount : Map.RowCount;
    }

    KRYS_NODISCARD const GridNamedLinesMap &
      ImplicitNamedGridLines(GridTrackSizingDirection direction) const noexcept
    {
      return direction == GridTrackSizingDirection::Columns ? ImplicitNamedGridColumnLines
                                                            : ImplicitNamedGridRowLines;
    }

    KRYS_NODISCARD bool operator==(const GridTemplateAreas &other) const noexcept
    {
      // It is only necessary to compare the `Map` member, as the `Implicit*` members
      // are purely cached derived values based on the value of `Map`.
      return Map == other.Map;
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<GridTemplateAreas>
  {
    KRYS_NODISCARD GridTemplateAreas operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<GridTemplateAreas>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const GridTemplateAreas &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<GridTemplateAreas>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const GridTemplateAreas &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GridTemplateAreas)
