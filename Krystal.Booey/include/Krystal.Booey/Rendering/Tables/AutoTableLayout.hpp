/*
 * Copyright (C) 2002 Lars Knoll (knoll@kde.org)
 *           (C) 2002 Dirk Mueller (mueller@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/Rendering/Tables/TableLayout.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderTableCell;

  /// @brief The automatic table layout algorithm: column widths come from the cells' contents.
  // https://www.w3.org/TR/CSS22/tables.html#auto-table-layout
  class AutoTableLayout final : public TableLayout
  {
  private:
    struct ColumnLayout
    {
      style::PreferredSize LogicalWidth {css::keywords::Auto {}};
      style::PreferredSize EffectiveLogicalWidth {css::keywords::Auto {}};
      float MinLogicalWidth {0};
      float MaxLogicalWidth {0};
      float EffectiveMinLogicalWidth {0};
      float EffectiveMaxLogicalWidth {0};
      float ComputedLogicalWidth {0};
      bool EmptyCellsOnly {true};
      float UsedZoom {1.0f};
    };

    List<ColumnLayout> _layoutStruct;
    // The cells that span more than one column, by increasing span.
    List<RenderTableCell *> _spanCells;
    bool _hasPercent : 1 {false};
    mutable bool _effectiveLogicalWidthDirty : 1 {true};
    LayoutUnit _scaledWidthFromPercentColumns;

  public:
    explicit AutoTableLayout(RenderTable *table) noexcept;
    ~AutoTableLayout() noexcept override;

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minWidth, LayoutUnit &maxWidth,
                                       TableIntrinsics intrinsics) noexcept override;
    KRYS_NODISCARD LayoutUnit ScaledWidthFromPercentColumns() const noexcept override
    {
      return _scaledWidthFromPercentColumns;
    }
    void ApplyPreferredLogicalWidthQuirks(LayoutUnit &minWidth, LayoutUnit &maxWidth) const noexcept override;
    void Layout() noexcept override;

  private:
    void FullRecalc() noexcept;
    void RecalcColumn(uint32 effCol) noexcept;

    KRYS_NODISCARD float CalcEffectiveLogicalWidth() noexcept;
    template <typename T>
    KRYS_NODISCARD float ShrinkCellWidthForType(float available) noexcept;

    void InsertSpanCell(RenderTableCell *cell) noexcept;
  };
}
