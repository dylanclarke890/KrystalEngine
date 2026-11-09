/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Grid/GridTemplateAreas.hpp"

#include "Krystal.Booey/CSS/Values/GridTemplateAreasValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
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

  void Serialiser<GridTemplateAreas>::operator()(StringBuilder &builder,
                                                 const css::SerialisationContext &context,
                                                 const RenderStyle &style,
                                                 const GridTemplateAreas &value) noexcept
  {
    krys::SwitchOn(
      value, [&](const css::keywords::None &keyword) { Serialise(builder, context, style, keyword); },
      [&](const GridNamedAreaMap &map) { Serialise(builder, context, style, map); });
  }
}
