#include "Krystal.Booey/Style/Values/Grid/GridTemplateAreas.hpp"

#include "Krystal.Booey/CSS/Values/GridTemplateAreasValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include <algorithm>

namespace krys::boo::style
{
  KRYS_NODISCARD static GridNamedLinesMap
    InitialiseImplicitNamedGridLines(const GridNamedAreaMap &namedGridAreas,
                                     GridTrackSizingDirection direction) noexcept
  {
    GridNamedLinesMap namedGridLines;

    for (const auto &[name, area] : namedGridAreas.Map)
    {
      auto areaSpan = direction == GridTrackSizingDirection::Rows ? area.Rows : area.Columns;
      {
        auto &startVector = namedGridLines.Map.try_emplace(name + u8"-start").first->second;
        startVector.push_back(areaSpan.StartLine());
        std::ranges::sort(startVector);
      }
      {
        auto &endVector = namedGridLines.Map.try_emplace(name + u8"-end").first->second;
        endVector.push_back(areaSpan.EndLine());
        std::ranges::sort(endVector);
      }
    }

    // FIXME: For acceptable performance, should sort once at the end, not as we add each item, or at least
    // insert in sorted order instead of using std::sort each time.

    return namedGridLines;
  }

  GridTemplateAreas::GridTemplateAreas(GridNamedAreaMap &&map) noexcept
      : Map {krys::move(map)}, ImplicitNamedGridColumnLines {InitialiseImplicitNamedGridLines(
                                 Map, GridTrackSizingDirection::Columns)},
        ImplicitNamedGridRowLines {InitialiseImplicitNamedGridLines(Map, GridTrackSizingDirection::Rows)}
  {
  }

  GridTemplateAreas::GridTemplateAreas(const GridNamedAreaMap &map) noexcept
      : Map {map}, ImplicitNamedGridColumnLines {InitialiseImplicitNamedGridLines(
                     Map, GridTrackSizingDirection::Columns)},
        ImplicitNamedGridRowLines {InitialiseImplicitNamedGridLines(Map, GridTrackSizingDirection::Rows)}
  {
  }

  // MARK: - Conversion

  GridTemplateAreas CSSValueConversion<GridTemplateAreas>::operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
  {
    if (css::IsValueId(value, css::ValueId::None))
    {
      return css::keywords::None {};
    }

    auto *gridTemplateAreasValue = RequiredDowncast<css::GridTemplateAreasValue>(state, value);
    if (!gridTemplateAreasValue)
    {
      return css::keywords::None {};
    }

    return GridTemplateAreas {gridTemplateAreasValue->Areas().Map};
  }

  Ref<css::Value> CSSValueCreation<GridTemplateAreas>::operator()(css::ValuePool &pool,
                                                                  const RenderStyle &style,
                                                                  const GridTemplateAreas &value) noexcept
  {
    return krys::SwitchOn(
      value, [&](const css::keywords::None &keyword) -> Ref<css::Value>
      { return CreateCSSValue(pool, style, keyword); }, [&](const GridNamedAreaMap &map) -> Ref<css::Value>
      { return css::GridTemplateAreasValue::Create(css::GridTemplateAreas {map}); });
  }

  // MARK: - Serialization

  void Serialiser<GridTemplateAreas>::operator()(css::CSSOMString &builder,
                                                 const css::SerialisationContext &context,
                                                 const RenderStyle &style,
                                                 const GridTemplateAreas &value) noexcept
  {
    krys::SwitchOn(
      value, [&](const css::keywords::None &keyword) { Serialise(builder, context, style, keyword); },
      [&](const GridNamedAreaMap &map) { Serialise(builder, context, style, map); });
  }
}
