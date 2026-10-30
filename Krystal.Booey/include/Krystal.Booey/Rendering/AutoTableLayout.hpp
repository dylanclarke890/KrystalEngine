#pragma once

#include "Krystal.Booey/Rendering/TableLayout.hpp"
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
