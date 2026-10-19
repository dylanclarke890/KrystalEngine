#pragma once

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/Style/CollapsedBorderValue.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderTableCell;
  class RenderTableRow;
  struct BorderValue;

  enum class CollapsedBorderSide : uint8
  {
    Before,
    After,
    Start,
    End
  };

  /// @brief A run of rows or columns, from `Start` up to `End`.
  struct CellSpan
  {
    uint32 Start;
    uint32 End;
  };

  /// @brief A table row group: the grid of its rows and cells, and their layout.
  class RenderTableSection final : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderTableSection);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTableSection);

  public:
    void Paint(PaintInfo &, const LayoutPoint &) noexcept final;
    void PaintObject(PaintInfo &, const LayoutPoint &) noexcept final;
    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction action) noexcept final;

    /// @brief The cells in a slot of the grid; more than one when cells overlap.
    struct CellStruct
    {
      List<RenderTableCell *> Cells;
      // true for columns after the first in a colspan
      bool InColSpan {false};

      KRYS_NODISCARD RenderTableCell *PrimaryCell() const noexcept
      {
        return HasCells() ? Cells.back() : nullptr;
      }
      KRYS_NODISCARD bool HasCells() const noexcept
      {
        return !Cells.empty();
      }
    };

    using GridRow = List<CellStruct>;

    /// @brief A row of the grid.
    struct RowStruct
    {
      GridRow Row;
      RenderTableRow *RowRenderer {nullptr};
      LayoutUnit Baseline;
      style::PreferredSize LogicalHeight {css::keywords::Auto {}};
    };

    enum class BlockBorderSide : bool
    {
      BorderBefore,
      BorderAfter
    };

    enum class InlineBorderSide : bool
    {
      BorderStart,
      BorderEnd
    };

  private:
    using CachedCollapsedBorders = Array<Maybe<CollapsedBorderValue>, 4>;

    List<RowStruct> _grid;
    List<LayoutUnit> _rowPos;

    // the current insertion position
    uint32 _cCol {0};
    uint32 _cRow {0};

    LayoutUnit _outerBorderStart;
    LayoutUnit _outerBorderEnd;
    LayoutUnit _outerBorderBefore;
    LayoutUnit _outerBorderAfter;

    // This map holds the collapsed border values for cells with collapsed borders.
    // It is held at RenderTableSection level to spare memory consumption by table cells.
    // Keyed by address: a cell's borders are removed when it leaves the section
    // (RemoveCachedCollapsedBorders).
    HashMap<const RenderTableCell *, CachedCollapsedBorders> _cellsCollapsedBorders;

    // The cells whose visual overflow reaches past their rows, painted whenever the section paints: kept only
    // while few enough, else every cell is painted (_forceSlowPaintPathWithOverflowingCell).
    HashSet<const RenderTableCell *> _overflowingCells;
    bool _forceSlowPaintPathWithOverflowingCell {false};
    bool _hasMultipleCellLevels {false};
    bool _needsCellRecalc {false};

  public:
    RenderTableSection(dom::Element &element, RenderStyle &&style) noexcept;
    RenderTableSection(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderTableSection() noexcept override;

    KRYS_NODISCARD RenderTableRow *FirstRow() const noexcept;
    KRYS_NODISCARD RenderTableRow *LastRow() const noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept override;
    KRYS_NODISCARD Maybe<LayoutUnit> LastLineBaseline() const noexcept override;
    KRYS_NODISCARD Maybe<LayoutUnit> BaselineFromCellContentEdges(ItemPosition alignment) const noexcept;

    void AddCell(RenderTableCell *cell, RenderTableRow *row) noexcept;

    KRYS_NODISCARD LayoutUnit CalcRowLogicalHeight() noexcept;
    void LayoutRows() noexcept;
    void ComputeOverflowFromCells() noexcept;

    KRYS_NODISCARD RenderTable *Table() const noexcept;

    KRYS_NODISCARD const BorderValue &BorderAdjoiningTableStart() const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningTableEnd() const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningStartCell(const RenderTableCell &cell) const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningEndCell(const RenderTableCell &cell) const noexcept;

    KRYS_NODISCARD CellStruct &CellAt(uint32 row, uint32 col) noexcept
    {
      RecalcCellsIfNeeded();
      return _grid[row].Row[col];
    }
    KRYS_NODISCARD const CellStruct &CellAt(uint32 row, uint32 col) const noexcept
    {
      krys_debug_assert(!_needsCellRecalc);
      return _grid[row].Row[col];
    }
    KRYS_NODISCARD RenderTableCell *PrimaryCellAt(uint32 row, uint32 col) noexcept
    {
      RecalcCellsIfNeeded();
      return _grid[row].Row[col].PrimaryCell();
    }
    KRYS_NODISCARD RenderTableRow *RowRendererAt(uint32 row) const noexcept
    {
      krys_debug_assert(!_needsCellRecalc);
      return _grid[row].RowRenderer;
    }

    void AppendColumn(uint32 pos) noexcept;
    void SplitColumn(uint32 pos, uint32 first) noexcept;

    KRYS_NODISCARD LayoutUnit CalcBlockDirectionOuterBorder(BlockBorderSide side) const noexcept;
    KRYS_NODISCARD LayoutUnit CalcInlineDirectionOuterBorder(InlineBorderSide side) const noexcept;
    void RecalcOuterBorder() noexcept;

    KRYS_NODISCARD LayoutUnit OuterBorderBefore() const noexcept
    {
      return _outerBorderBefore;
    }
    KRYS_NODISCARD LayoutUnit OuterBorderAfter() const noexcept
    {
      return _outerBorderAfter;
    }
    KRYS_NODISCARD LayoutUnit OuterBorderStart() const noexcept
    {
      return _outerBorderStart;
    }
    KRYS_NODISCARD LayoutUnit OuterBorderEnd() const noexcept
    {
      return _outerBorderEnd;
    }

    KRYS_NODISCARD LayoutUnit OuterBorderLeft(WritingMode writingMode) const noexcept;
    KRYS_NODISCARD LayoutUnit OuterBorderRight(WritingMode writingMode) const noexcept;
    KRYS_NODISCARD LayoutUnit OuterBorderTop(WritingMode writingMode) const noexcept;
    KRYS_NODISCARD LayoutUnit OuterBorderBottom(WritingMode writingMode) const noexcept;

    KRYS_NODISCARD uint32 NumRows() const noexcept
    {
      krys_debug_assert(!_needsCellRecalc);
      return static_cast<uint32>(_grid.size());
    }
    KRYS_NODISCARD uint32 NumColumns() const noexcept;
    void RecalcCells() noexcept;
    void RecalcCellsIfNeeded() noexcept
    {
      if (_needsCellRecalc)
      {
        RecalcCells();
      }
    }
    void RemoveRedundantColumns() noexcept;

    KRYS_NODISCARD bool NeedsCellRecalc() const noexcept
    {
      return _needsCellRecalc;
    }
    void SetNeedsCellRecalc() noexcept;

    KRYS_NODISCARD LayoutUnit RowBaseline(uint32 row) noexcept
    {
      RecalcCellsIfNeeded();
      return _grid[row].Baseline;
    }
    void RowLogicalHeightChanged(uint32 rowIndex) noexcept;

    void ClearCachedCollapsedBorders() noexcept;
    void RemoveCachedCollapsedBorders(const RenderTableCell &cell) noexcept;
    void SetCachedCollapsedBorder(const RenderTableCell &cell, CollapsedBorderSide side,
                                  CollapsedBorderValue border) noexcept;
    KRYS_NODISCARD CollapsedBorderValue CachedCollapsedBorder(const RenderTableCell &cell,
                                                              CollapsedBorderSide side) const noexcept;

    /// @return The *consumed* extra logical height.
    KRYS_NODISCARD bool HasOverflowingCell() const noexcept
    {
      return !_overflowingCells.empty() || _forceSlowPaintPathWithOverflowingCell;
    }

    KRYS_NODISCARD CellSpan FullTableRowSpan() const noexcept;
    KRYS_NODISCARD CellSpan FullTableColumnSpan() const noexcept;
    /// @brief The rows and columns @p damageRect (aligned with the table's rows and columns) reaches, and
    /// those whose borders may reach into it.
    KRYS_NODISCARD CellSpan DirtiedRows(const LayoutRect &damageRect) const noexcept;
    KRYS_NODISCARD CellSpan DirtiedColumns(const LayoutRect &damageRect) const noexcept;

    enum ShouldIncludeAllIntersectingCells
    {
      IncludeAllIntersectingCells,
      DoNotIncludeAllIntersectingCells
    };
    KRYS_NODISCARD CellSpan SpannedRows(const LayoutRect &flippedRect,
                                        ShouldIncludeAllIntersectingCells shouldInclude) const noexcept;
    KRYS_NODISCARD CellSpan SpannedColumns(const LayoutRect &flippedRect,
                                           ShouldIncludeAllIntersectingCells shouldInclude) const noexcept;

    /// @brief @p rect flipped to align with the coordinates of the row and column positions.
    KRYS_NODISCARD LayoutRect LogicalRectForWritingModeAndDirection(const LayoutRect &rect) const noexcept;

    // FIXME: We may want to introduce a structure holding the in-flux layout information.
    KRYS_NODISCARD LayoutUnit DistributeExtraLogicalHeightToRows(LayoutUnit extraLogicalHeight) noexcept;

    void WillInsertTableRow(RenderTableRow &child, RenderObject *beforeChild) noexcept;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return (IsAnonymous() || IsPseudoElement()) ? u8"RenderTableSection (anonymous)"
                                                  : u8"RenderTableSection";
    }

  private:
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;
    void WillBeRemovedFromTree() noexcept override;
    void Layout() noexcept override;

    void ComputeIntrinsicLogicalWidths(LayoutUnit &, LayoutUnit &) const noexcept override
    {
    }

    void EnsureRows(uint32 numRows) noexcept;

    void PaintCell(RenderTableCell *cell, PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept;
    void PaintRowGroupBorder(const PaintInfo &paintInfo, bool antialias, LayoutRect rect, BoxSide side,
                             css::PropertyId borderColor, BorderStyle borderStyle,
                             BorderStyle tableBorderStyle) noexcept;
    void PaintRowGroupBorderIfRequired(const PaintInfo &paintInfo, const LayoutPoint &paintOffset, uint32 row,
                                       uint32 column, BoxSide borderSide,
                                       RenderTableCell *cell = nullptr) noexcept;
    KRYS_NODISCARD LayoutUnit OffsetLeftForRowGroupBorder(RenderTableCell *cell,
                                                          const LayoutRect &rowGroupRect,
                                                          uint32 row) noexcept;
    KRYS_NODISCARD LayoutUnit OffsetTopForRowGroupBorder(RenderTableCell *cell, BoxSide borderSide,
                                                         uint32 row) noexcept;
    KRYS_NODISCARD LayoutUnit VerticalRowGroupBorderHeight(RenderTableCell *cell,
                                                           const LayoutRect &rowGroupRect,
                                                           uint32 row) noexcept;
    KRYS_NODISCARD LayoutUnit HorizontalRowGroupBorderWidth(RenderTableCell *cell,
                                                            const LayoutRect &rowGroupRect, uint32 row,
                                                            uint32 column) noexcept;

    void RelayoutCellIfFlexed(RenderTableCell &cell, uint32 rowIndex, LayoutUnit rowHeight) noexcept;

    void DistributeExtraLogicalHeightToPercentRows(LayoutUnit &extraLogicalHeight,
                                                   int32 totalPercent) noexcept;
    void DistributeExtraLogicalHeightToAutoRows(LayoutUnit &extraLogicalHeight,
                                                uint32 autoRowsCount) noexcept;
    void DistributeRemainingExtraLogicalHeight(LayoutUnit &extraLogicalHeight) noexcept;

    void ComputeOverflowFromCells(uint32 totalRows, uint32 nEffCols) noexcept;

    void SetLogicalPositionForCell(RenderTableCell *cell, uint32 effectiveColumn) const noexcept;
    KRYS_NODISCARD LayoutUnit CellLogicalWidthInTableDirectionIncludingColumnSpan(
      const RenderTableCell &cell, size_t startColumn, size_t numberOfColumns) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTableSection)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTableSection();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
