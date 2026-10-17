#include "Krystal.Booey/Rendering/RenderTableSection.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableCol.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo
{
  namespace
  {
    inline void SetRowLogicalHeightToRowStyleLogicalHeight(RenderTableSection::RowStruct &row) noexcept
    {
      krys_debug_assert(row.RowRenderer);
      row.LogicalHeight = row.RowRenderer->Style().LogicalHeight();
    }

    inline void UpdateLogicalHeightForCell(RenderTableSection::RowStruct &row,
                                           const RenderTableCell *cell) noexcept
    {
      // We ignore height settings on rowspan cells.
      if (cell->RowSpan() != 1)
      {
        return;
      }

      const auto &logicalHeight =
        !cell->IsOrthogonal() ? cell->Style().LogicalHeight() : cell->Style().LogicalWidth();
      if (logicalHeight.IsPossiblyPositive())
      {
        if (auto percentageLogicalHeight = logicalHeight.TryPercentage())
        {
          if (auto percentageRowLogicalHeight = row.LogicalHeight.TryPercentage();
              !percentageRowLogicalHeight
              || percentageRowLogicalHeight->Value < percentageLogicalHeight->Value)
          {
            row.LogicalHeight = logicalHeight;
          }
        }
        else if (auto fixedLogicalHeight = logicalHeight.TryFixed())
        {
          auto usedZoom = cell->Style().UsedZoomForLength();
          if (auto fixedRowLogicalHeight = row.LogicalHeight.TryFixed();
              row.LogicalHeight.IsAuto()
              || (fixedRowLogicalHeight
                  && style::Evaluate<float>(*fixedRowLogicalHeight, usedZoom)
                       < style::Evaluate<float>(*fixedLogicalHeight, usedZoom)))
          {
            row.LogicalHeight = logicalHeight;
          }
        }
      }
    }

    KRYS_NODISCARD LayoutUnit ResolveLogicalHeightForRow(const style::PreferredSize &rowLogicalHeight,
                                                         style::ZoomFactor usedZoom) noexcept
    {
      if (auto fixedRowLogicalHeight = rowLogicalHeight.TryFixed())
      {
        return style::Evaluate<LayoutUnit>(*fixedRowLogicalHeight, usedZoom);
      }
      if (rowLogicalHeight.IsCalculated())
      {
        return style::Evaluate<LayoutUnit>(rowLogicalHeight, 0_lu, usedZoom);
      }
      return 0_lu;
    }

    KRYS_NODISCARD bool ShouldFlexCellChild(const RenderTableCell &cell,
                                            const RenderBox &cellDescendant) noexcept
    {
      if (!cell.Style().LogicalHeight().IsSpecified())
      {
        return false;
      }
      if (cellDescendant.ScrollsOverflowY())
      {
        return true;
      }
      // FIXME: Form controls other than fieldsets (HTMLFormControlElement), once there are those.
      return cellDescendant.IsBlockLevelReplacedOrAtomicInline();
    }
  }

  RenderTableSection::RenderTableSection(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBox(Type::TableSection, element, krys::move(style))
  {
    SetInline(false);
    krys_debug_assert(IsRenderTableSection());
  }

  RenderTableSection::RenderTableSection(dom::Document &document, RenderStyle &&style) noexcept
      : RenderBox(Type::TableSection, document, krys::move(style))
  {
    SetInline(false);
    krys_debug_assert(IsRenderTableSection());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableSection::~RenderTableSection() noexcept = default;

  RenderTableRow *RenderTableSection::FirstRow() const noexcept
  {
    return Downcast<RenderTableRow>(FirstChild());
  }

  RenderTableRow *RenderTableSection::LastRow() const noexcept
  {
    return Downcast<RenderTableRow>(LastChild());
  }

  RenderTable *RenderTableSection::Table() const noexcept
  {
    return Downcast<RenderTable>(Parent());
  }

  void RenderTableSection::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBox::StyleDidChange(diff, oldStyle);
    PropagateStyleToAnonymousChildren(StylePropagationType::AllChildren);

    if (auto *table = Table(); table && oldStyle)
    {
      table->InvalidateCollapsedBordersAfterStyleChangeIfNeeded(*oldStyle, Style());
    }
  }

  void RenderTableSection::WillBeRemovedFromTree() noexcept
  {
    RenderBox::WillBeRemovedFromTree();

    // Preventively invalidate our cells as we may be re-inserted into
    // a new table which would require us to rebuild our structure.
    SetNeedsCellRecalc();
  }

  void RenderTableSection::WillInsertTableRow(RenderTableRow &child, RenderObject *beforeChild) noexcept
  {
    if (beforeChild)
    {
      SetNeedsCellRecalc();
    }

    auto insertionRow = _cRow;
    ++_cRow;
    _cCol = 0;

    EnsureRows(_cRow);

    _grid[insertionRow].RowRenderer = &child;
    child.SetRowIndex(insertionRow);

    if (!beforeChild)
    {
      SetRowLogicalHeightToRowStyleLogicalHeight(_grid[insertionRow]);
    }
  }

  void RenderTableSection::EnsureRows(uint32 numRows) noexcept
  {
    if (numRows <= _grid.size())
    {
      return;
    }

    auto oldSize = _grid.size();
    _grid.resize(numRows);

    auto effectiveColumnCount = std::max(1u, Table()->NumEffCols());
    for (auto row = oldSize; row < _grid.size(); ++row)
    {
      _grid[row].Row.resize(effectiveColumnCount);
    }
  }

  void RenderTableSection::AddCell(RenderTableCell *cell, RenderTableRow *row) noexcept
  {
    // We don't insert the cell if we need cell recalc as our internal columns' representation
    // will have drifted from the table's representation. Also RecalcCells will call AddCell
    // at a later time after sync'ing our columns' with the table's.
    if (NeedsCellRecalc())
    {
      return;
    }

    auto rSpan = cell->RowSpan();
    auto cSpan = cell->ColSpan();
    const auto &columns = Table()->Columns();
    auto nCols = static_cast<uint32>(columns.size());
    auto insertionRow = row->RowIndex();

    // ### mozilla still seems to do the old HTML way, even for strict DTD
    // (see the annotation on table cell layouting in the CSS specs and the testcase below:
    // <TABLE border>
    // <TR><TD>1 <TD rowspan="2">2 <TD>3 <TD>4
    // <TR><TD colspan="2">5
    // </TABLE>
    while (_cCol < nCols && (CellAt(insertionRow, _cCol).HasCells() || CellAt(insertionRow, _cCol).InColSpan))
    {
      _cCol++;
    }

    UpdateLogicalHeightForCell(_grid[insertionRow], cell);

    EnsureRows(insertionRow + rSpan);

    _grid[insertionRow].RowRenderer = row;

    auto col = _cCol;
    // tell the cell where it is
    bool inColSpan = false;
    while (cSpan)
    {
      uint32 currentSpan;
      if (_cCol >= nCols)
      {
        Table()->AppendColumn(cSpan);
        currentSpan = cSpan;
      }
      else
      {
        if (cSpan < columns[_cCol].Span)
        {
          Table()->SplitColumn(_cCol, cSpan);
        }
        currentSpan = columns[_cCol].Span;
      }
      for (uint32 r = 0; r < rSpan; r++)
      {
        auto &c = CellAt(insertionRow + r, _cCol);
        krys_debug_assert(cell);
        c.Cells.push_back(cell);
        // If cells overlap then we take the slow path for painting.
        if (c.Cells.size() > 1)
        {
          _hasMultipleCellLevels = true;
        }
        if (inColSpan)
        {
          c.InColSpan = true;
        }
      }
      _cCol++;
      cSpan -= currentSpan;
      inColSpan = true;
    }
    cell->SetCol(Table()->EffColToCol(col));
  }

  LayoutUnit RenderTableSection::CalcRowLogicalHeight() noexcept
  {
    SetLayoutNeededForbiddenScope layoutForbiddenScope(*this);

    krys_debug_assert(!NeedsLayout());

    // We ignore the border-spacing on any non-top section as it is already included in the previous section's
    // last row position.
    LayoutUnit spacing;
    if (this == Table()->TopSection())
    {
      spacing = Table()->VBorderSpacing();
    }

    // There are no reflections (-webkit-box-reflect was dropped).
    LayoutStateMaintainer statePusher(*this, LocationOffset(),
                                      IsTransformed() || GetWritingMode().IsBlockFlipped());

    _rowPos.resize(_grid.size() + 1);
    _rowPos[0] = spacing;

    auto totalRows = static_cast<uint32>(_grid.size());

    for (uint32 r = 0; r < totalRows; r++)
    {
      _grid[r].Baseline = 0_lu;
      LayoutUnit baselineDescent;

      if (_grid[r].LogicalHeight.IsSpecified())
      {
        // Our base size is the biggest logical height from our cells' styles (excluding row spanning cells).
        _rowPos[r + 1] =
          std::max(_rowPos[r]
                     + ResolveLogicalHeightForRow(_grid[r].LogicalHeight,
                                                  _grid[r].RowRenderer->Style().UsedZoomForLength()),
                   0_lu);
      }
      else
      {
        // Non-specified lengths are ignored because the row already accounts for the cells intrinsic logical
        // height.
        _rowPos[r + 1] = std::max(_rowPos[r], 0_lu);
      }

      auto &row = _grid[r].Row;
      auto totalCols = static_cast<uint32>(row.size());

      for (uint32 c = 0; c < totalCols; c++)
      {
        auto &current = CellAt(r, c);
        for (auto *cell : current.Cells)
        {
          if (current.InColSpan && cell->RowSpan() == 1)
          {
            continue;
          }

          // FIXME: We are always adding the height of a rowspan to the last rows which doesn't match
          // other browsers. See webkit.org/b/52185 for example.
          if ((cell->RowIndex() + cell->RowSpan() - 1) != r)
          {
            // We will apply the height of the rowspan to the current row if next row is not valid.
            if ((r + 1) < totalRows)
            {
              uint32 col = 0;
              const auto *nextRowCell = &CellAt(r + 1, col);

              // We are trying to find that next row is valid or not.
              while (!nextRowCell->Cells.empty() && nextRowCell->Cells[0]->RowSpan() > 1
                     && nextRowCell->Cells[0]->RowIndex() < (r + 1))
              {
                col++;
                if (col < totalCols)
                {
                  nextRowCell = &CellAt(r + 1, col);
                }
                else
                {
                  break;
                }
              }

              // We are adding the height of the rowspan to the current row if next row is not valid.
              if (col < totalCols && !nextRowCell->Cells.empty())
              {
                continue;
              }
            }
          }

          // For row spanning cells, |r| is the last row in the span.
          auto cellStartRow = cell->RowIndex();

          if (cell->OverridingBorderBoxLogicalHeight() && !cell->IsOrthogonal())
          {
            cell->ClearIntrinsicPadding();
            cell->ClearOverridingSize();
            cell->SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
            cell->LayoutIfNeeded();
          }

          auto cellLogicalHeight = cell->LogicalHeightForRowSizing();
          _rowPos[r + 1] = std::max(_rowPos[r + 1], _rowPos[cellStartRow] + cellLogicalHeight);

          // Find out the baseline. The baseline is set on the first row in a rowspan.
          if (cell->IsBaselineAligned())
          {
            auto baselinePosition = cell->CellBaselinePosition() - cell->IntrinsicPaddingBefore();
            auto borderAndComputedPaddingBefore =
              cell->BorderAndPaddingBefore() - cell->IntrinsicPaddingBefore();
            if (baselinePosition > borderAndComputedPaddingBefore)
            {
              _grid[cellStartRow].Baseline = std::max(_grid[cellStartRow].Baseline, baselinePosition);
              // The descent of a cell that spans multiple rows does not affect the height of the first row it
              // spans, so don't let it become the baseline descent applied to the rest of the row. Also we
              // don't account for the baseline descent of non-spanning cells when computing a spanning cell's
              // extent.
              LayoutUnit cellStartRowBaselineDescent;
              if (cell->RowSpan() == 1)
              {
                baselineDescent = std::max(baselineDescent, cellLogicalHeight - baselinePosition);
                cellStartRowBaselineDescent = baselineDescent;
              }
              _rowPos[cellStartRow + 1] =
                std::max(_rowPos[cellStartRow + 1],
                         _rowPos[cellStartRow] + _grid[cellStartRow].Baseline + cellStartRowBaselineDescent);
            }
          }
        }
      }

      // Add the border-spacing to our final position.
      // Use table border-spacing even in non-top sections
      spacing = Table()->VBorderSpacing();
      _rowPos[r + 1] += _grid[r].RowRenderer ? spacing : 0_lu;
      _rowPos[r + 1] = std::max(_rowPos[r + 1], _rowPos[r]);
    }

    for (size_t rowIndex = 0; rowIndex < totalRows; ++rowIndex)
    {
      if (_grid[rowIndex].RowRenderer
          && _grid[rowIndex].RowRenderer->Style().GetVisibility() == Visibility::Collapse)
      {
        auto delta = _rowPos[rowIndex + 1] - _rowPos[rowIndex];
        if (delta > 0_lu)
        {
          // Reduce height of collapsed row to 0 without affecting other rows
          for (auto adjustedRowIndex = rowIndex + 1; adjustedRowIndex <= totalRows; ++adjustedRowIndex)
          {
            _rowPos[adjustedRowIndex] -= delta;
          }
        }
      }
    }

    krys_debug_assert(!NeedsLayout());

    return _rowPos[_grid.size()];
  }

  LayoutUnit RenderTableSection::CellLogicalWidthInTableDirectionIncludingColumnSpan(
    const RenderTableCell &cell, size_t startColumn, size_t numberOfColumns) const noexcept
  {
    krys_debug_assert(startColumn < numberOfColumns);

    auto endColumn = startColumn;
    auto columnSpan = cell.ColSpan();
    const auto &columns = Table()->Columns();
    while (columnSpan && endColumn < numberOfColumns)
    {
      krys_debug_assert(endColumn < columns.size());
      columnSpan -= columns[endColumn].Span;
      endColumn++;
    }

    const auto &columnPositions = Table()->ColumnPositions();
    if (startColumn >= columnPositions.size() || endColumn >= columnPositions.size())
    {
      krys_debug_assert(false);
      return {};
    }
    return columnPositions[endColumn] - columnPositions[startColumn] - Table()->HBorderSpacing();
  }

  void RenderTableSection::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());
    krys_debug_assert(!NeedsCellRecalc());
    krys_debug_assert(!Table()->NeedsSectionRecalc());

    // AddChild may over-grow _grid but we don't want to throw away the memory too early as AddChild
    // can be called in a loop (e.g during parsing). Doing it now ensures we have a stable-enough structure.
    _grid.shrink_to_fit();

    // There are no reflections (-webkit-box-reflect was dropped).
    LayoutStateMaintainer statePusher(*this, LocationOffset(),
                                      IsTransformed() || GetWritingMode().IsBlockFlipped());

    for (size_t rowIndex = 0; rowIndex < _grid.size(); ++rowIndex)
    {
      auto &columnList = _grid[rowIndex].Row;
      auto numberOfColumns = columnList.size();
      auto *rowRenderer = _grid[rowIndex].RowRenderer;

      // First, propagate our table layout's information to the cells. This will mark the row as needing
      // layout if there was a column logical width change.
      for (size_t startColumn = 0; startColumn < numberOfColumns; ++startColumn)
      {
        auto &currentColumn = columnList[startColumn];
        auto *cell = currentColumn.PrimaryCell();
        if (!cell || currentColumn.InColSpan)
        {
          continue;
        }

        auto cellHadSelfNeedsLayout = cell->SelfNeedsLayout();
        cell->SetCellLogicalWidth(
          CellLogicalWidthInTableDirectionIncludingColumnSpan(*cell, startColumn, numberOfColumns));
        if (!cellHadSelfNeedsLayout && cell->SelfNeedsLayout() && rowRenderer)
        {
          rowRenderer->SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
        }
      }

      if (rowRenderer)
      {
        // FIXME: Lay out the row again when the page height changed (pageLogicalHeightChanged), once there's
        // pagination.
        rowRenderer->LayoutIfNeeded();
      }
    }

    ClearNeedsLayout();
  }

  void RenderTableSection::DistributeExtraLogicalHeightToPercentRows(LayoutUnit &extraLogicalHeight,
                                                                     int32 totalPercent) noexcept
  {
    if (!totalPercent)
    {
      return;
    }

    auto totalRows = static_cast<uint32>(_grid.size());
    auto totalHeight = _rowPos[totalRows] + extraLogicalHeight;
    LayoutUnit totalLogicalHeightAdded;
    totalPercent = std::min(totalPercent, 100);
    auto rowHeight = _rowPos[1] - _rowPos[0];
    for (uint32 r = 0; r < totalRows; ++r)
    {
      if (auto percentageLogicalHeight = _grid[r].LogicalHeight.TryPercentage();
          totalPercent > 0 && percentageLogicalHeight)
      {
        auto toAdd = std::min(extraLogicalHeight,
                              LayoutUnit((totalHeight * percentageLogicalHeight->Value / 100) - rowHeight));
        // If toAdd is negative, then we don't want to shrink the row (this bug
        // affected Outlook Web Access).
        toAdd = std::max(0_lu, toAdd);
        totalLogicalHeightAdded += toAdd;
        extraLogicalHeight -= toAdd;
        totalPercent -= static_cast<int32>(percentageLogicalHeight->Value);
      }
      krys_debug_assert(totalRows >= 1);
      if (r < totalRows - 1)
      {
        rowHeight = _rowPos[r + 2] - _rowPos[r + 1];
      }
      _rowPos[r + 1] += totalLogicalHeightAdded;
    }
  }

  void RenderTableSection::DistributeExtraLogicalHeightToAutoRows(LayoutUnit &extraLogicalHeight,
                                                                  uint32 autoRowsCount) noexcept
  {
    if (!autoRowsCount)
    {
      return;
    }

    LayoutUnit totalLogicalHeightAdded;
    for (size_t r = 0; r < _grid.size(); ++r)
    {
      if (autoRowsCount > 0 && _grid[r].LogicalHeight.IsAuto())
      {
        // Recomputing |extraLogicalHeightForRow| guarantees that we properly ditribute round
        // |extraLogicalHeight|.
        auto extraLogicalHeightForRow = extraLogicalHeight / static_cast<int32>(autoRowsCount);
        totalLogicalHeightAdded += extraLogicalHeightForRow;
        extraLogicalHeight -= extraLogicalHeightForRow;
        --autoRowsCount;
      }
      _rowPos[r + 1] += totalLogicalHeightAdded;
    }
  }

  void RenderTableSection::DistributeRemainingExtraLogicalHeight(LayoutUnit &extraLogicalHeight) noexcept
  {
    auto totalRows = _grid.size();

    if (extraLogicalHeight <= 0 || !_rowPos[totalRows])
    {
      return;
    }

    // FIXME: _rowPos[totalRows] - _rowPos[0] is the total rows' size.
    auto totalRowSize = _rowPos[totalRows];
    LayoutUnit totalLogicalHeightAdded;
    auto previousRowPosition = _rowPos[0];
    for (size_t r = 0; r < totalRows; r++)
    {
      // weight with the original height
      totalLogicalHeightAdded += extraLogicalHeight * (_rowPos[r + 1] - previousRowPosition) / totalRowSize;
      previousRowPosition = _rowPos[r + 1];
      _rowPos[r + 1] += totalLogicalHeightAdded;
    }

    extraLogicalHeight -= totalLogicalHeightAdded;
  }

  LayoutUnit RenderTableSection::DistributeExtraLogicalHeightToRows(LayoutUnit extraLogicalHeight) noexcept
  {
    if (!extraLogicalHeight)
    {
      return extraLogicalHeight;
    }

    auto totalRows = static_cast<uint32>(_grid.size());
    if (!totalRows)
    {
      return extraLogicalHeight;
    }

    if (!_rowPos[totalRows] && NextSibling())
    {
      return extraLogicalHeight;
    }

    uint32 autoRowsCount = 0;
    int32 totalPercent = 0;
    for (uint32 r = 0; r < totalRows; r++)
    {
      if (_grid[r].LogicalHeight.IsAuto())
      {
        ++autoRowsCount;
      }
      else if (auto percentageLogicalHeight = _grid[r].LogicalHeight.TryPercentage())
      {
        totalPercent += static_cast<int32>(percentageLogicalHeight->Value);
      }
    }

    auto remainingExtraLogicalHeight = extraLogicalHeight;
    DistributeExtraLogicalHeightToPercentRows(remainingExtraLogicalHeight, totalPercent);
    DistributeExtraLogicalHeightToAutoRows(remainingExtraLogicalHeight, autoRowsCount);
    DistributeRemainingExtraLogicalHeight(remainingExtraLogicalHeight);
    return extraLogicalHeight - remainingExtraLogicalHeight;
  }

  void RenderTableSection::RelayoutCellIfFlexed(RenderTableCell &cell, uint32 rowIndex,
                                                LayoutUnit rowHeight) noexcept
  {
    // Force percent height children to lay themselves out again.
    // This will cause these children to grow to fill the cell.
    // FIXME: There is still more work to do here to fully match WinIE (should
    // it become necessary to do so). In quirks mode, WinIE behaves like we
    // do, but it will clip the cells that spill out of the table section. In
    // strict mode, Mozilla and WinIE both regrow the table to accommodate the
    // new height of the cell (thus letting the percentages cause growth one
    // time only). We may also not be handling row-spanning cells correctly.
    //
    // Note also the oddity where replaced elements always flex, and yet blocks/tables do
    // not necessarily flex. WinIE is crazy and inconsistent, and we can't hope to
    // match the behavior perfectly, but we'll continue to refine it as we discover new
    // bugs. :)
    bool cellChildrenFlex = false;
    bool flexAllChildren =
      cell.Style().LogicalHeight().IsFixed()
      || (!Table()->Style().LogicalHeight().IsAuto() && rowHeight != cell.LogicalHeight());

    for (auto &renderer : ChildrenOfType<RenderBox>(cell))
    {
      if (renderer.Style().LogicalHeight().IsPercentOrCalculated()
          && (flexAllChildren || ShouldFlexCellChild(cell, renderer)))
      {
        auto *renderTable = DynamicDowncast<RenderTable>(renderer);
        if (!renderTable || renderTable->HasSections())
        {
          cellChildrenFlex = true;
          break;
        }
      }
    }

    if (!cellChildrenFlex)
    {
      if (auto *percentHeightDescendants = cell.PercentHeightDescendants())
      {
        for (auto &descendant : *percentHeightDescendants)
        {
          if (flexAllChildren || ShouldFlexCellChild(cell, *descendant))
          {
            cellChildrenFlex = true;
            break;
          }
        }
      }
    }

    if (!cellChildrenFlex)
    {
      return;
    }

    cell.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    // Alignment within a cell is based off the calculated
    // height, which becomes irrelevant once the cell has
    // been resized based off its percentage.
    cell.SetOverridingLogicalHeightFromRowHeight(rowHeight);
    cell.LayoutIfNeeded();

    if (!cell.IsBaselineAligned())
    {
      return;
    }

    // If the baseline moved, we may have to update the data for our row. Find out the new baseline.
    auto baseline = cell.CellBaselinePosition();
    if (baseline > cell.BorderAndPaddingBefore())
    {
      _grid[rowIndex].Baseline = std::max(_grid[rowIndex].Baseline, baseline);
    }
  }

  void RenderTableSection::LayoutRows() noexcept
  {
    SetLayoutNeededForbiddenScope layoutForbiddenScope(*this);

    krys_debug_assert(!NeedsLayout());

    auto numberOfRows = _grid.size();

    // Set the width of our section now.  The rows will also be this width.
    SetLogicalWidth(Table()->ContentBoxLogicalWidth());

    auto vspacing = Table()->VBorderSpacing();
    size_t numberOfEffectiveColumns = Table()->NumEffCols();

    LayoutStateMaintainer statePusher(*this, LocationOffset(),
                                      IsTransformed() || GetWritingMode().IsBlockFlipped());

    for (size_t rowIndex = 0; rowIndex < numberOfRows; rowIndex++)
    {
      // Set the row's x/y position and width/height.
      if (auto *rowRenderer = _grid[rowIndex].RowRenderer)
      {
        // FIXME: the x() position of the row should be table()->hBorderSpacing() so that it can
        // report the correct offsetLeft. However, that will require a lot of rebaselining of test results.
        rowRenderer->SetLogicalLocation({0_lu, _rowPos[rowIndex]});
        rowRenderer->SetLogicalWidth(LogicalWidth());

        LayoutUnit rowLogicalHeight;
        // If the row is collapsed then it has 0 height. vspacing was implicitly
        // removed earlier, when _rowPos[rowIndex+1] was set to _rowPos[rowIndex].
        auto rowHasVisibilityCollapse = [&](size_t row)
        {
          return (_grid[row].RowRenderer
                  && _grid[row].RowRenderer->Style().GetVisibility() == Visibility::Collapse)
                 || Style().GetVisibility() == Visibility::Collapse;
        };
        if (!rowHasVisibilityCollapse(rowIndex))
        {
          rowLogicalHeight = _rowPos[rowIndex + 1] - _rowPos[rowIndex] - vspacing;
        }
        krys_debug_assert(rowLogicalHeight >= 0);
        rowRenderer->SetLogicalHeight(rowLogicalHeight);
        rowRenderer->UpdateLayerTransform();
        rowRenderer->ClearOverflow();
        rowRenderer->AddVisualEffectOverflow();
      }

      for (size_t columnIndex = 0; columnIndex < numberOfEffectiveColumns; columnIndex++)
      {
        auto &cs = CellAt(static_cast<uint32>(rowIndex), static_cast<uint32>(columnIndex));
        auto *cell = cs.PrimaryCell();

        if (!cell || cs.InColSpan)
        {
          continue;
        }

        auto cellRowIndex = cell->RowIndex();
        auto rowHeight = _rowPos[cellRowIndex + cell->RowSpan()] - _rowPos[cellRowIndex] - vspacing;
        RelayoutCellIfFlexed(*cell, cellRowIndex, rowHeight);

        auto logicalHeightForIntrinsicPadding = !cell->IsOrthogonal()
                                                  ? rowHeight
                                                  : CellLogicalWidthInTableDirectionIncludingColumnSpan(
                                                      *cell, columnIndex, numberOfEffectiveColumns);
        if (cell->ComputeIntrinsicPadding(logicalHeightForIntrinsicPadding))
        {
          // FIXME: Changing an intrinsic padding shouldn't trigger a relayout as it only shifts the cell
          // inside the row but doesn't change the logical height.
          cell->SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
        }

        auto oldCellRect = cell->FrameRect();

        SetLogicalPositionForCell(cell, static_cast<uint32>(columnIndex));

        // FIXME: Lay out cells whose page offset changed, and fit cells that pagination resized into their
        // row (rowHeightIncreaseForPagination), once there's pagination.

        if (cell->IsOrthogonal())
        {
          cell->SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
          cell->SetOverridingBorderBoxLogicalWidth(rowHeight);
        }
        cell->LayoutIfNeeded();

        auto childOffset = LayoutSize(cell->Location() - oldCellRect.Location());
        if (childOffset.Width() || childOffset.Height())
        {
          View().FrameView().LayoutContext().AddLayoutDelta(childOffset);
          // FIXME: Repaint the cell if it moved (repaintDuringLayoutIfMoved), once there's painting.
        }
      }
    }

    krys_debug_assert(!NeedsLayout());

    SetLogicalHeight(_rowPos[numberOfRows]);

    UpdateLayerTransform();

    ComputeOverflowFromCells(static_cast<uint32>(numberOfRows),
                             static_cast<uint32>(numberOfEffectiveColumns));
  }

  void RenderTableSection::ComputeOverflowFromCells() noexcept
  {
    auto totalRows = static_cast<uint32>(_grid.size());
    auto nEffCols = Table()->NumEffCols();
    ComputeOverflowFromCells(totalRows, nEffCols);
  }

  void RenderTableSection::ComputeOverflowFromCells(uint32 totalRows, uint32 nEffCols) noexcept
  {
    ClearOverflow();

    // Now that our height has been determined, add in overflow from cells.
    for (uint32 r = 0; r < totalRows; r++)
    {
      for (uint32 c = 0; c < nEffCols; c++)
      {
        auto &cs = CellAt(r, c);
        auto *cell = cs.PrimaryCell();
        if (!cell || cs.InColSpan)
        {
          continue;
        }
        if (r < totalRows - 1 && cell == PrimaryCellAt(r + 1, c))
        {
          continue;
        }
        AddOverflowFromContainedBox(*cell);
        // FIXME: Keep the overflowing cells for painting and hit testing (m_overflowingCells,
        // m_forceSlowPaintPathWithOverflowingCell), once there are those.
      }
    }
  }

  LayoutUnit RenderTableSection::CalcBlockDirectionOuterBorder(BlockBorderSide side) const noexcept
  {
    auto totalCols = Table()->NumEffCols();
    if (_grid.empty() || !totalCols)
    {
      return 0_lu;
    }

    float borderWidth = 0;
    auto writingMode = Table()->GetWritingMode();

    const auto &sectionBorder = (side == BlockBorderSide::BorderBefore) ? Style().BorderBefore(writingMode)
                                                                        : Style().BorderAfter(writingMode);
    if (sectionBorder.HasHiddenStyle())
    {
      return -1_lu;
    }
    if (sectionBorder.HasVisibleStyle())
    {
      borderWidth = style::Evaluate<float>(sectionBorder.Width, style::ZoomNeeded {});
    }

    const auto *row = (side == BlockBorderSide::BorderBefore) ? FirstRow() : LastRow();
    const auto &rowStyle = row->Style();
    const auto &rowBorder = (side == BlockBorderSide::BorderBefore) ? rowStyle.BorderBefore(writingMode)
                                                                    : rowStyle.BorderAfter(writingMode);
    if (rowBorder.HasHiddenStyle())
    {
      return -1_lu;
    }
    if (rowBorder.HasVisibleStyle())
    {
      auto rowBorderWidth = style::Evaluate<float>(rowBorder.Width, style::ZoomNeeded {});
      if (rowBorderWidth > borderWidth)
      {
        borderWidth = rowBorderWidth;
      }
    }

    bool allHidden = true;
    auto rowIndex = (side == BlockBorderSide::BorderBefore) ? 0u : static_cast<uint32>(_grid.size() - 1);
    for (uint32 c = 0; c < totalCols; c++)
    {
      const auto &current = CellAt(rowIndex, c);
      if (current.InColSpan || !current.HasCells())
      {
        continue;
      }

      // FIXME: Make this work with perpendicular and flipped cells.
      const auto &cellBorderStyle = current.PrimaryCell()->Style();
      const auto &cellBorder = (side == BlockBorderSide::BorderBefore)
                                 ? cellBorderStyle.BorderBefore(writingMode)
                                 : cellBorderStyle.BorderAfter(writingMode);

      // FIXME: Don't repeat for the same col group
      if (auto *colGroup = Table()->ColElement(c))
      {
        const auto &colGroupStyle = colGroup->Style();
        const auto &colBorder = (side == BlockBorderSide::BorderBefore)
                                  ? colGroupStyle.BorderBefore(writingMode)
                                  : colGroupStyle.BorderAfter(writingMode);
        if (colBorder.HasHiddenStyle() || cellBorder.HasHiddenStyle())
        {
          continue;
        }
        allHidden = false;
        if (colBorder.HasVisibleStyle())
        {
          auto colBorderWidth = style::Evaluate<float>(colBorder.Width, style::ZoomNeeded {});
          if (colBorderWidth > borderWidth)
          {
            borderWidth = colBorderWidth;
          }
        }
        if (cellBorder.HasVisibleStyle())
        {
          auto cellBorderWidth = style::Evaluate<float>(cellBorder.Width, style::ZoomNeeded {});
          if (cellBorderWidth > borderWidth)
          {
            borderWidth = cellBorderWidth;
          }
        }
      }
      else
      {
        if (cellBorder.HasHiddenStyle())
        {
          continue;
        }
        allHidden = false;
        if (cellBorder.HasVisibleStyle())
        {
          auto cellBorderWidth = style::Evaluate<float>(cellBorder.Width, style::ZoomNeeded {});
          if (cellBorderWidth > borderWidth)
          {
            borderWidth = cellBorderWidth;
          }
        }
      }
    }
    if (allHidden)
    {
      return -1_lu;
    }

    return CollapsedBorderValue::AdjustedCollapsedBorderWidth(borderWidth, GetDocument().DeviceScaleFactor(),
                                                              side == BlockBorderSide::BorderAfter);
  }

  LayoutUnit RenderTableSection::CalcInlineDirectionOuterBorder(InlineBorderSide side) const noexcept
  {
    auto totalCols = Table()->NumEffCols();
    if (_grid.empty() || !totalCols)
    {
      return 0_lu;
    }
    auto colIndex = (side == InlineBorderSide::BorderStart) ? 0u : totalCols - 1;

    float borderWidth = 0;
    auto writingMode = Table()->GetWritingMode();

    const auto &sectionBorder = (side == InlineBorderSide::BorderStart) ? Style().BorderStart(writingMode)
                                                                        : Style().BorderEnd(writingMode);
    if (sectionBorder.HasHiddenStyle())
    {
      return -1_lu;
    }
    if (sectionBorder.HasVisibleStyle())
    {
      borderWidth = style::Evaluate<float>(sectionBorder.Width, style::ZoomNeeded {});
    }

    if (auto *colGroup = Table()->ColElement(colIndex))
    {
      const auto &colGroupStyle = colGroup->Style();
      const auto &colBorder = (side == InlineBorderSide::BorderStart) ? colGroupStyle.BorderStart(writingMode)
                                                                      : colGroupStyle.BorderEnd(writingMode);
      if (colBorder.HasHiddenStyle())
      {
        return -1_lu;
      }
      if (colBorder.HasVisibleStyle())
      {
        auto colBorderWidth = style::Evaluate<float>(colBorder.Width, style::ZoomNeeded {});
        if (colBorderWidth > borderWidth)
        {
          borderWidth = colBorderWidth;
        }
      }
    }

    bool allHidden = true;
    for (uint32 r = 0; r < _grid.size(); r++)
    {
      const auto &current = CellAt(r, colIndex);
      if (!current.HasCells())
      {
        continue;
      }

      // FIXME: Make this work with perpendicular and flipped cells.
      const auto &cellBorderStyle = current.PrimaryCell()->Style();
      const auto &cellBorder = (side == InlineBorderSide::BorderStart)
                                 ? cellBorderStyle.BorderStart(writingMode)
                                 : cellBorderStyle.BorderEnd(writingMode);
      const auto &rowBorderStyle = current.PrimaryCell()->Parent()->Style();
      const auto &rowBorder = (side == InlineBorderSide::BorderStart)
                                ? rowBorderStyle.BorderStart(writingMode)
                                : rowBorderStyle.BorderEnd(writingMode);
      // FIXME: Don't repeat for the same cell
      if (cellBorder.HasHiddenStyle() || rowBorder.HasHiddenStyle())
      {
        continue;
      }
      allHidden = false;
      if (cellBorder.HasVisibleStyle())
      {
        auto cellBorderWidth = style::Evaluate<float>(cellBorder.Width, style::ZoomNeeded {});
        if (cellBorderWidth > borderWidth)
        {
          borderWidth = cellBorderWidth;
        }
      }
      if (rowBorder.HasVisibleStyle())
      {
        auto rowBorderWidth = style::Evaluate<float>(rowBorder.Width, style::ZoomNeeded {});
        if (rowBorderWidth > borderWidth)
        {
          borderWidth = rowBorderWidth;
        }
      }
    }
    if (allHidden)
    {
      return -1_lu;
    }

    return CollapsedBorderValue::AdjustedCollapsedBorderWidth(borderWidth, GetDocument().DeviceScaleFactor(),
                                                              (side == InlineBorderSide::BorderStart)
                                                                ? writingMode.IsInlineFlipped()
                                                                : !writingMode.IsInlineFlipped());
  }

  void RenderTableSection::RecalcOuterBorder() noexcept
  {
    _outerBorderBefore = CalcBlockDirectionOuterBorder(BlockBorderSide::BorderBefore);
    _outerBorderAfter = CalcBlockDirectionOuterBorder(BlockBorderSide::BorderAfter);
    _outerBorderStart = CalcInlineDirectionOuterBorder(InlineBorderSide::BorderStart);
    _outerBorderEnd = CalcInlineDirectionOuterBorder(InlineBorderSide::BorderEnd);
  }

  LayoutUnit RenderTableSection::OuterBorderBottom(WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsBlockTopToBottom() ? OuterBorderAfter() : OuterBorderBefore();
    }
    return writingMode.IsInlineTopToBottom() ? OuterBorderEnd() : OuterBorderStart();
  }

  LayoutUnit RenderTableSection::OuterBorderLeft(WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight() ? OuterBorderStart() : OuterBorderEnd();
    }
    return writingMode.IsBlockLeftToRight() ? OuterBorderBefore() : OuterBorderAfter();
  }

  LayoutUnit RenderTableSection::OuterBorderRight(WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight() ? OuterBorderEnd() : OuterBorderStart();
    }
    return writingMode.IsBlockLeftToRight() ? OuterBorderAfter() : OuterBorderBefore();
  }

  LayoutUnit RenderTableSection::OuterBorderTop(WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsBlockTopToBottom() ? OuterBorderBefore() : OuterBorderAfter();
    }
    return writingMode.IsInlineTopToBottom() ? OuterBorderStart() : OuterBorderEnd();
  }

  Maybe<LayoutUnit> RenderTableSection::FirstLineBaseline() const noexcept
  {
    if (_grid.empty())
    {
      return null;
    }

    if (auto firstLineBaseline = _grid.front().Baseline)
    {
      return _rowPos.front() + firstLineBaseline;
    }

    return BaselineFromCellContentEdges(ItemPosition::Baseline);
  }

  Maybe<LayoutUnit> RenderTableSection::LastLineBaseline() const noexcept
  {
    if (_grid.empty())
    {
      return null;
    }

    if (auto lastLineBaseline = _grid.back().Baseline)
    {
      return _rowPos[_grid.size() - 1] + lastLineBaseline;
    }

    return BaselineFromCellContentEdges(ItemPosition::LastBaseline);
  }

  Maybe<LayoutUnit> RenderTableSection::BaselineFromCellContentEdges(ItemPosition alignment) const noexcept
  {
    krys_debug_assert(alignment == ItemPosition::Baseline || alignment == ItemPosition::LastBaseline);
    const auto &row = alignment == ItemPosition::Baseline ? _grid.front().Row : _grid.back().Row;
    Maybe<LayoutUnit> result;
    for (const auto &cs : row)
    {
      const auto *cell = cs.PrimaryCell();
      // Only cells with content have a baseline
      if (cell && cell->ContentBoxLogicalHeight())
      {
        auto candidate =
          cell->LogicalTop() + cell->BorderAndPaddingBefore() + cell->ContentBoxLogicalHeight();
        result = std::max(result.value_or(candidate), candidate);
      }
    }
    return result;
  }

  void RenderTableSection::RecalcCells() noexcept
  {
    krys_debug_assert(_needsCellRecalc);
    // We reset the flag here to ensure that AddCell() works. This is safe to do because we clear the grid
    // and update its dimensions to be consistent with the table's column representation before we rebuild
    // the grid using AddCell().
    _needsCellRecalc = false;

    _cCol = 0;
    _cRow = 0;
    _grid.clear();

    for (auto *row = FirstRow(); row; row = row->NextRow())
    {
      auto insertionRow = _cRow;
      _cRow++;
      _cCol = 0;
      EnsureRows(_cRow);

      _grid[insertionRow].RowRenderer = row;
      row->SetRowIndex(insertionRow);
      SetRowLogicalHeightToRowStyleLogicalHeight(_grid[insertionRow]);

      for (auto *cell = row->FirstCell(); cell; cell = cell->NextCell())
      {
        AddCell(cell, row);
      }
    }

    _grid.shrink_to_fit();
    SetNeedsLayout();
  }

  void RenderTableSection::RemoveRedundantColumns() noexcept
  {
    auto maximumNumberOfColumns = Table()->NumEffCols();
    for (auto &rowItem : _grid)
    {
      if (rowItem.Row.size() <= maximumNumberOfColumns)
      {
        continue;
      }
      rowItem.Row.resize(maximumNumberOfColumns);
    }
  }

  // FIXME: This function could be made O(1) in certain cases (like for the non-most-constrainive cells'
  // case).
  void RenderTableSection::RowLogicalHeightChanged(uint32 rowIndex) noexcept
  {
    if (NeedsCellRecalc())
    {
      return;
    }

    SetRowLogicalHeightToRowStyleLogicalHeight(_grid[rowIndex]);

    for (auto *cell = _grid[rowIndex].RowRenderer->FirstCell(); cell; cell = cell->NextCell())
    {
      UpdateLogicalHeightForCell(_grid[rowIndex], cell);
    }
  }

  void RenderTableSection::SetNeedsCellRecalc() noexcept
  {
    _needsCellRecalc = true;

    // Clear the grid now to ensure that we don't hold onto any stale pointers (e.g. a cell renderer that is
    // being removed).
    _grid.clear();

    if (auto *table = Table())
    {
      table->SetNeedsSectionRecalc();
    }
  }

  uint32 RenderTableSection::NumColumns() const noexcept
  {
    krys_debug_assert(!_needsCellRecalc);
    uint32 result = 0;

    for (uint32 r = 0; r < _grid.size(); ++r)
    {
      for (auto c = result; c < Table()->NumEffCols(); ++c)
      {
        const auto &cell = CellAt(r, c);
        if (cell.HasCells() || cell.InColSpan)
        {
          result = c;
        }
      }
    }

    return result + 1;
  }

  const BorderValue &RenderTableSection::BorderAdjoiningTableStart() const noexcept
  {
    return Style().BorderStart(Table()->GetWritingMode());
  }

  const BorderValue &RenderTableSection::BorderAdjoiningTableEnd() const noexcept
  {
    return Style().BorderEnd(Table()->GetWritingMode());
  }

  const BorderValue &
    RenderTableSection::BorderAdjoiningStartCell(KRYS_MAYBE_UNUSED const RenderTableCell &cell) const noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(cell.IsFirstOrLastCellInRow());
#endif
    return Style().BorderStart(Table()->GetWritingMode());
  }

  const BorderValue &
    RenderTableSection::BorderAdjoiningEndCell(KRYS_MAYBE_UNUSED const RenderTableCell &cell) const noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(cell.IsFirstOrLastCellInRow());
#endif
    return Style().BorderEnd(Table()->GetWritingMode());
  }

  void RenderTableSection::AppendColumn(uint32 pos) noexcept
  {
    krys_debug_assert(!_needsCellRecalc);

    for (auto &row : _grid)
    {
      row.Row.resize(pos + 1);
    }
  }

  void RenderTableSection::SplitColumn(uint32 pos, uint32 first) noexcept
  {
    krys_debug_assert(!_needsCellRecalc);

    if (_cCol > pos)
    {
      _cCol++;
    }
    for (auto &rowStruct : _grid)
    {
      auto &r = rowStruct.Row;
      r.insert(r.begin() + pos + 1, CellStruct());
      if (r[pos].HasCells())
      {
        r[pos + 1].Cells.insert(r[pos + 1].Cells.end(), r[pos].Cells.begin(), r[pos].Cells.end());
        auto *cell = r[pos].PrimaryCell();
        krys_debug_assert(cell);
        krys_debug_assert(cell->ColSpan() >= (r[pos].InColSpan ? 1u : 0u));
        auto colleft = cell->ColSpan() - r[pos].InColSpan;
        if (first > colleft)
        {
          r[pos + 1].InColSpan = false;
        }
        else
        {
          r[pos + 1].InColSpan = first + r[pos].InColSpan;
        }
      }
      else
      {
        r[pos + 1].InColSpan = false;
      }
    }
  }

  void RenderTableSection::ClearCachedCollapsedBorders() noexcept
  {
    if (!Table()->CollapseBorders())
    {
      return;
    }
    _cellsCollapsedBorders.clear();
  }

  void RenderTableSection::RemoveCachedCollapsedBorders(const RenderTableCell &cell) noexcept
  {
    if (!Table()->CollapseBorders())
    {
      return;
    }

    _cellsCollapsedBorders.erase(&cell);
  }

  void RenderTableSection::SetCachedCollapsedBorder(const RenderTableCell &cell, CollapsedBorderSide side,
                                                    CollapsedBorderValue border) noexcept
  {
    krys_debug_assert(Table()->CollapseBorders());
    krys_debug_assert(border.Width());
    _cellsCollapsedBorders[&cell][static_cast<size_t>(side)] = border;
  }

  CollapsedBorderValue RenderTableSection::CachedCollapsedBorder(const RenderTableCell &cell,
                                                                 CollapsedBorderSide side) const noexcept
  {
    krys_debug_assert(Table()->CollapseBorders() && Table()->CollapsedBordersAreValid());

    // Only non-empty collapsed borders are in the hashmap.
    if (auto it = _cellsCollapsedBorders.find(&cell); it != _cellsCollapsedBorders.end())
    {
      if (const auto &border = it->second[static_cast<size_t>(side)])
      {
        return *border;
      }
    }
    return CollapsedBorderValue(BorderValue(), Color(), BorderPrecedence::Cell,
                                cell.Style().UsedZoomForLength());
  }

  void RenderTableSection::SetLogicalPositionForCell(RenderTableCell *cell,
                                                     uint32 effectiveColumn) const noexcept
  {
    auto oldCellLocation = cell->Location();

    auto cellLocation = LayoutPoint(0_lu, _rowPos[cell->RowIndex()]);
    auto horizontalBorderSpacing = Table()->HBorderSpacing();

    // The table's writing mode determines in which direction the rows flow.
    if (Table()->GetWritingMode().IsInlineFlipped())
    {
      cellLocation.SetX(Table()->ColumnPositions()[Table()->NumEffCols()]
                        - Table()->ColumnPositions()[Table()->ColToEffCol(cell->Col() + cell->ColSpan())]
                        + horizontalBorderSpacing);
    }
    else
    {
      cellLocation.SetX(Table()->ColumnPositions()[effectiveColumn] + horizontalBorderSpacing);
    }

    if (cell->IsOrthogonal())
    {
      cellLocation = cellLocation.TransposedPoint();
    }
    cell->SetLogicalLocation(cellLocation);
    View().FrameView().LayoutContext().AddLayoutDelta(oldCellLocation - cell->Location());
  }
}
