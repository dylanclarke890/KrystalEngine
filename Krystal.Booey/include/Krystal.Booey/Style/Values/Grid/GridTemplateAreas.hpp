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

#pragma once

#include "Krystal.Booey/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridNamedAreaMap.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridNamedLinesMap.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

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
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const GridTemplateAreas &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GridTemplateAreas)
