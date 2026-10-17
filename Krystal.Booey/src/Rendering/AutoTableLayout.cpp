#include "Krystal.Booey/Rendering/AutoTableLayout.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableCol.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderTableSection.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool ShouldScaleColumnsForParent(const RenderTable &table) noexcept
    {
      const auto *containingBlock = table.ContainingBlock();
      while (containingBlock && !Is<RenderView>(containingBlock))
      {
        // It doesn't matter if our table is auto or fixed: auto means we don't
        // scale. Fixed doesn't care if we do or not because it doesn't depend
        // on the cell contents' preferred widths.
        if (Is<RenderTableCell>(containingBlock))
        {
          return false;
        }

        // The max logical width of a table may be "infinity" (or TableMaxWidth, to be more exact) if the sum
        // if the columns' percentages is 100% or more, AND there is at least one column that has a
        // non-percentage-based positive logical width. In such situations no table logical width will be
        // large enough to satisfy the constraint set by the contents. So the idea is to use ~infinity to make
        // sure we use all available size in the containing block. However, this just doesn't work if this is
        // a flex or grid item, so disallow scaling in that case.
        if (Is<RenderFlexibleBox>(containingBlock) || Is<RenderGrid>(containingBlock))
        {
          return false;
        }
        containingBlock = containingBlock->ContainingBlock();
      }
      return true;
    }
  }

  AutoTableLayout::AutoTableLayout(RenderTable *table) noexcept : TableLayout(table)
  {
  }

  AutoTableLayout::~AutoTableLayout() noexcept = default;

  void AutoTableLayout::RecalcColumn(uint32 effCol) noexcept
  {
    auto &columnLayout = _layoutStruct[effCol];

    RenderTableCell *fixedContributor = nullptr;
    RenderTableCell *maxContributor = nullptr;

    for (auto &child : ChildrenOfType<RenderObject>(*_table))
    {
      if (auto *column = DynamicDowncast<RenderTableCol>(child))
      {
        // RenderTableCols don't have the concept of preferred logical width, but we need to clear their dirty
        // bits so that if we call SetNeedsPreferredWidthsUpdate() on a col or one of its descendants, we'll
        // mark its ancestors as dirty.
        column->ClearNeedsPreferredLogicalWidthsUpdate();
      }
      else if (auto *section = DynamicDowncast<RenderTableSection>(child))
      {
        auto numRows = section->NumRows();
        for (uint32 i = 0; i < numRows; ++i)
        {
          const auto &current = section->CellAt(i, effCol);
          auto *cell = current.PrimaryCell();

          if (current.InColSpan || !cell)
          {
            continue;
          }

          const auto &cellStyle = cell->Style();
          bool cellHasContent = cell->FirstChild() || cellStyle.HasBorder()
                                || !cellStyle.GetPaddingBox().AllOf([](const auto &paddingEdge)
                                                                    { return paddingEdge.IsKnownZero(); })
                                || cellStyle.HasBackground();
          if (cellHasContent)
          {
            columnLayout.EmptyCellsOnly = false;
          }

          // A cell originates in this column. Ensure we have
          // a min/max width of at least 1px for this column now.
          columnLayout.MinLogicalWidth = std::max(columnLayout.MinLogicalWidth, 0.f);
          columnLayout.MaxLogicalWidth = std::max(columnLayout.MaxLogicalWidth, 0.f);

          if (cell->IsOrthogonal())
          {
            cell->ClearIntrinsicPadding();
          }

          if (cell->ColSpan() == 1)
          {
            columnLayout.MinLogicalWidth =
              std::max(cell->MinLogicalWidthForColumnSizing().CeilToFloat(), columnLayout.MinLogicalWidth);
            auto maxPreferredWidth = cell->MaxLogicalWidthForColumnSizing().CeilToFloat();
            if (maxPreferredWidth > columnLayout.MaxLogicalWidth)
            {
              columnLayout.MaxLogicalWidth = maxPreferredWidth;
              maxContributor = cell;
            }

            // All browsers implement a size limit on the cell's max width.
            // Our limit is based on KHTML's representation that used 16 bits widths.
            // FIXME: Other browsers have a lower limit for the cell's max width.
            constexpr float CellMaxWidth = 32'760;
            auto [cellLogicalWidth, cellUsedZoom] = cell->StyleOrColLogicalWidth();
            if (auto fixedCellLogicalWidth = cellLogicalWidth.TryFixed())
            {
              if (style::Evaluate<float>(*fixedCellLogicalWidth, cellUsedZoom) > CellMaxWidth)
              {
                cellLogicalWidth = style::PreferredSize::Fixed {CellMaxWidth};
              }
            }

            cellLogicalWidth.SwitchOn(
              [&](const style::PreferredSize::Fixed &fixedCellLogicalWidth)
              {
                // ignore width=0
                if (fixedCellLogicalWidth.IsPositive() && !columnLayout.LogicalWidth.IsPercentOrCalculated())
                {
                  auto borderBoxLogicalWidth =
                    cell->AdjustBorderBoxLogicalWidthForBoxSizing(fixedCellLogicalWidth).ToFloat();
                  if (auto fixedColumnLayoutLogicalWidth = columnLayout.LogicalWidth.TryFixed())
                  {
                    auto columnLogicalWidth =
                      style::Evaluate<float>(*fixedColumnLayoutLogicalWidth, cellUsedZoom);
                    // Nav/IE weirdness
                    if ((borderBoxLogicalWidth > columnLogicalWidth)
                        || ((columnLogicalWidth == borderBoxLogicalWidth) && (maxContributor == cell)))
                    {
                      columnLayout.LogicalWidth = style::PreferredSize::Fixed {borderBoxLogicalWidth};
                      fixedContributor = cell;
                    }
                  }
                  else
                  {
                    columnLayout.LogicalWidth = style::PreferredSize::Fixed {borderBoxLogicalWidth};
                    fixedContributor = cell;
                  }
                }
              },
              [&](const style::PreferredSize::Percentage &percentageCellLogicalWidth)
              {
                _hasPercent = true;
                if (auto percentageColumnLayoutLogicalWidth = columnLayout.LogicalWidth.TryPercentage();
                    percentageCellLogicalWidth.Value > 0
                    && (!percentageColumnLayoutLogicalWidth
                        || percentageCellLogicalWidth.Value > percentageColumnLayoutLogicalWidth->Value))
                {
                  columnLayout.LogicalWidth = cellLogicalWidth;
                }
              },
              [&](const style::PreferredSize::Calc &) { columnLayout.LogicalWidth = css::keywords::Auto {}; },
              [&](const auto &) {});
          }
          else if (!effCol || section->PrimaryCellAt(i, effCol - 1) != cell)
          {
            // If a cell originates in this spanning column ensure we have a min/max width of at least 1px for
            // it.
            columnLayout.MinLogicalWidth =
              std::max(columnLayout.MinLogicalWidth, cell->MaxLogicalWidthForColumnSizing() ? 1.f : 0.f);

            // This spanning cell originates in this column. Insert the cell into spanning cells list.
            InsertSpanCell(cell);
          }
        }
      }
    }

    // Nav/IE weirdness
    if (auto fixedColumnLayoutLogicalWidth = columnLayout.LogicalWidth.TryFixed())
    {
      auto columnZoom = style::ZoomFactor {columnLayout.UsedZoom, _table->Style().DeviceScaleFactor()};
      if (_table->GetDocument().InQuirksMode()
          && columnLayout.MaxLogicalWidth > style::Evaluate<float>(*fixedColumnLayoutLogicalWidth, columnZoom)
          && fixedContributor != maxContributor)
      {
        columnLayout.LogicalWidth = css::keywords::Auto {};
        fixedContributor = nullptr;
      }
    }

    columnLayout.MaxLogicalWidth = std::max(columnLayout.MaxLogicalWidth, columnLayout.MinLogicalWidth);
  }

  void AutoTableLayout::FullRecalc() noexcept
  {
    _hasPercent = false;
    _effectiveLogicalWidthDirty = true;

    auto nEffCols = _table->NumEffCols();
    _layoutStruct.assign(nEffCols, ColumnLayout());
    _spanCells.clear();

    style::PreferredSize groupLogicalWidth {css::keywords::Auto {}};
    uint32 currentColumn = 0;
    for (auto *column = _table->FirstColumn(); column; column = column->NextColumn())
    {
      if (column->IsTableColumnGroupWithColumnChildren())
      {
        groupLogicalWidth = column->Style().LogicalWidth();
      }
      else
      {
        auto colLogicalWidth = column->Style().LogicalWidth();
        // FIXME: calc() on tables should be handled consistently with other lengths.
        if (colLogicalWidth.IsCalculated() || colLogicalWidth.IsAuto())
        {
          colLogicalWidth = groupLogicalWidth;
        }
        if (colLogicalWidth.IsSpecified() && colLogicalWidth.IsKnownZero())
        {
          colLogicalWidth = css::keywords::Auto {};
        }
        auto effCol = _table->ColToEffCol(currentColumn);
        auto span = column->Span();
        if (!colLogicalWidth.IsAuto() && span == 1 && effCol < nEffCols && _table->SpanOfEffCol(effCol) == 1)
        {
          _layoutStruct[effCol].UsedZoom = column->Style().UsedZoom();
          _layoutStruct[effCol].LogicalWidth = colLogicalWidth;
          if (auto fixedColLogicalWidth = colLogicalWidth.TryFixed())
          {
            auto fixedWidth =
              style::Evaluate<float>(*fixedColLogicalWidth, column->Style().UsedZoomForLength());
            if (_layoutStruct[effCol].MaxLogicalWidth < fixedWidth)
            {
              _layoutStruct[effCol].MaxLogicalWidth = fixedWidth;
            }
          }
        }
        currentColumn += span;
      }

      // For the last column in a column-group, we invalidate our group logical width.
      if (column->IsTableColumn() && !column->NextSibling())
      {
        groupLogicalWidth = css::keywords::Auto {};
      }
    }

    for (uint32 i = 0; i < nEffCols; i++)
    {
      RecalcColumn(i);
    }

    for (auto &section : ChildrenOfType<RenderTableSection>(*_table))
    {
      section.ClearNeedsPreferredWidthsUpdate();
      for (auto *row = section.FirstRow(); row; row = row->NextRow())
      {
        row->ClearNeedsPreferredWidthsUpdate();
      }
    }
  }

  void AutoTableLayout::ComputeIntrinsicLogicalWidths(LayoutUnit &minWidth, LayoutUnit &maxWidth,
                                                      TableIntrinsics intrinsics) noexcept
  {
    FullRecalc();

    auto spanMaxLogicalWidth = CalcEffectiveLogicalWidth();
    minWidth = 0_lu;
    maxWidth = 0_lu;
    float maxPercent = 0;
    float maxNonPercent = 0;
    bool scaleColumnsForSelf = intrinsics == TableIntrinsics::ForLayout;

    float remainingPercent = 100;
    for (auto &columnLayout : _layoutStruct)
    {
      minWidth += LayoutUnit(columnLayout.EffectiveMinLogicalWidth);
      maxWidth += LayoutUnit(columnLayout.EffectiveMaxLogicalWidth);
      if (scaleColumnsForSelf)
      {
        if (auto percentageEffectiveLogicalWidth = columnLayout.EffectiveLogicalWidth.TryPercentage())
        {
          auto percent = std::min(percentageEffectiveLogicalWidth->Value, remainingPercent);
          // When percent columns meet or exceed 100% and there are remaining
          // columns, the other browsers (FF, Edge) use an artificially high max
          // width, so we do too. Instead of division by zero, logicalWidth and
          // maxNonPercent are set to TableMaxWidth.
          // Issue: https://github.com/w3c/csswg-drafts/issues/1501
          auto logicalWidth = (percent > 0) ? columnLayout.EffectiveMaxLogicalWidth * 100 / percent
                                            : static_cast<float>(TableMaxWidth);
          maxPercent = std::max(logicalWidth, maxPercent);
          remainingPercent -= percent;
        }
        else
        {
          maxNonPercent += columnLayout.EffectiveMaxLogicalWidth;
        }
      }
    }

    if (scaleColumnsForSelf)
    {
      if (maxNonPercent > 0)
      {
        maxNonPercent =
          (remainingPercent > 0) ? maxNonPercent * 100 / remainingPercent : static_cast<float>(TableMaxWidth);
      }
      _scaledWidthFromPercentColumns =
        std::min(LayoutUnit(TableMaxWidth), LayoutUnit(std::max(maxPercent, maxNonPercent)));
      if (_scaledWidthFromPercentColumns > maxWidth && ShouldScaleColumnsForParent(*_table))
      {
        maxWidth = _scaledWidthFromPercentColumns;
      }
    }

    if (intrinsics == TableIntrinsics::ForKeyword && _layoutStruct.empty())
    {
      krys_debug_assert(!minWidth);
      krys_debug_assert(!maxWidth);
      minWidth = _table->BordersPaddingAndSpacingInRowDirection();
      maxWidth = minWidth;
    }

    maxWidth = std::max(maxWidth, LayoutUnit(spanMaxLogicalWidth));
  }

  void AutoTableLayout::ApplyPreferredLogicalWidthQuirks(LayoutUnit &minWidth,
                                                         LayoutUnit &maxWidth) const noexcept
  {
    const auto &tableStyle = _table->Style();
    if (auto fixedTableLogicalWidth = tableStyle.LogicalWidth().TryFixed();
        fixedTableLogicalWidth && fixedTableLogicalWidth->IsPositive())
    {
      auto minContentWidth = minWidth;
      auto tableFixedWidth = _table->OverridingBorderBoxLogicalWidth().value_or(
        style::Evaluate<LayoutUnit>(*fixedTableLogicalWidth, tableStyle.UsedZoomForLength()));
      auto clampedWidth = std::max(minContentWidth, std::max(minWidth, tableFixedWidth));
      minWidth = clampedWidth;
      maxWidth = clampedWidth;
      if (auto fixedMaxWidth = tableStyle.LogicalMaxWidth().TryFixed())
      {
        auto maxAllowedWidth = style::Evaluate<LayoutUnit>(*fixedMaxWidth, tableStyle.UsedZoomForLength());
        minWidth = std::min(minWidth, maxAllowedWidth);
        minWidth = std::max(minWidth, minContentWidth);
        maxWidth = minWidth;
      }
    }
  }

  // This method takes care of colspans.
  // EffectiveLogicalWidth is the same as LogicalWidth for cells without colspans. If we have colspans, they
  // get modified.
  float AutoTableLayout::CalcEffectiveLogicalWidth() noexcept
  {
    float maxLogicalWidth = 0;

    auto nEffCols = _layoutStruct.size();
    auto spacingInRowDirection = _table->HBorderSpacing().ToFloat();

    for (auto &columnLayout : _layoutStruct)
    {
      columnLayout.EffectiveLogicalWidth = columnLayout.LogicalWidth;
      columnLayout.EffectiveMinLogicalWidth = columnLayout.MinLogicalWidth;
      columnLayout.EffectiveMaxLogicalWidth = columnLayout.MaxLogicalWidth;
    }

    for (auto *cell : _spanCells)
    {
      auto span = cell->ColSpan();

      auto [cellLogicalWidth, cellUsedZoom] = cell->StyleOrColLogicalWidth();
      if (cellLogicalWidth.IsKnownZero())
      {
        cellLogicalWidth = css::keywords::Auto {};
      }

      auto effCol = _table->ColToEffCol(cell->Col());
      size_t lastCol = effCol;
      if (cell->IsOrthogonal())
      {
        cell->ClearIntrinsicPadding();
      }
      auto cellMinLogicalWidth = cell->MinLogicalWidthForColumnSizing().ToFloat() + spacingInRowDirection;
      auto cellMaxLogicalWidth = cell->MaxLogicalWidthForColumnSizing().ToFloat() + spacingInRowDirection;
      float totalPercent = 0;
      float spanMinLogicalWidth = 0;
      float spanMaxLogicalWidth = 0;
      bool allColsArePercent = true;
      bool allColsAreFixed = true;
      bool haveAuto = false;
      bool spanHasEmptyCellsOnly = true;
      float fixedWidth = 0;
      while (lastCol < nEffCols && span > 0)
      {
        auto &columnLayout = _layoutStruct[lastCol];
        auto fallbackCase = [&]
        {
          // If the column is a percentage width, do not let the spanning cell overwrite the
          // width value.  This caused a mis-rendering on amazon.com.
          // Sample snippet:
          // <table border=2 width=100%><
          //   <tr><td>1</td><td colspan=2>2-3</tr>
          //   <tr><td>1</td><td colspan=2 width=100%>2-3</td></tr>
          // </table>
          if (auto percentageEffectiveLogicalWidth = columnLayout.EffectiveLogicalWidth.TryPercentage())
          {
            totalPercent += percentageEffectiveLogicalWidth->Value;
          }
          else
          {
            columnLayout.EffectiveLogicalWidth = css::keywords::Auto {};
            allColsArePercent = false;
          }
          allColsAreFixed = false;
        };

        columnLayout.LogicalWidth.SwitchOn(
          [&](const style::PreferredSize::Percentage &percentage)
          {
            totalPercent += percentage.Value;
            allColsAreFixed = false;
          },
          [&](const style::PreferredSize::Fixed &fixed)
          {
            if (fixed.IsPositive())
            {
              fixedWidth += style::Evaluate<float>(
                fixed, style::ZoomFactor {columnLayout.UsedZoom, _table->Style().DeviceScaleFactor()});
              allColsArePercent = false;
              // IE resets effWidth to Auto here, but this breaks the konqueror about page and seems to be
              // some bad legacy behavior anyway. mozilla doesn't do this so I decided we don't neither.
              return;
            }
            haveAuto = true;
            fallbackCase();
          },
          [&](const css::keywords::Auto &)
          {
            haveAuto = true;
            fallbackCase();
          },
          [&](const auto &) { fallbackCase(); });

        if (!columnLayout.EmptyCellsOnly)
        {
          spanHasEmptyCellsOnly = false;
        }
        span -= std::min(span, _table->SpanOfEffCol(static_cast<uint32>(lastCol)));
        spanMinLogicalWidth += columnLayout.EffectiveMinLogicalWidth;
        spanMaxLogicalWidth += columnLayout.EffectiveMaxLogicalWidth;
        lastCol++;
        cellMinLogicalWidth -= spacingInRowDirection;
        cellMaxLogicalWidth -= spacingInRowDirection;
      }

      // adjust table max width if needed
      if (auto percentageCellLogicalWidth = cellLogicalWidth.TryPercentage())
      {
        if (totalPercent > percentageCellLogicalWidth->Value || allColsArePercent)
        {
          // can't satisfy this condition, treat as variable
          cellLogicalWidth = css::keywords::Auto {};
        }
        else
        {
          maxLogicalWidth = std::max(maxLogicalWidth, std::max(spanMaxLogicalWidth, cellMaxLogicalWidth) * 100
                                                        / percentageCellLogicalWidth->Value);

          // all non percent columns in the span get percent values to sum up correctly.
          auto percentMissing = percentageCellLogicalWidth->Value - totalPercent;
          float totalWidth = 0;
          for (auto pos = static_cast<size_t>(effCol); pos < lastCol; ++pos)
          {
            if (!_layoutStruct[pos].EffectiveLogicalWidth.IsPercentOrCalculated())
            {
              totalWidth += _layoutStruct[pos].EffectiveMaxLogicalWidth;
            }
          }

          for (auto pos = static_cast<size_t>(effCol); pos < lastCol; ++pos)
          {
            if (!_layoutStruct[pos].EffectiveLogicalWidth.IsPercentOrCalculated())
            {
              // Handle the case when there's only one cell with 'width: percent' and it's empty.
              auto percent =
                percentMissing * (totalWidth ? _layoutStruct[pos].EffectiveMaxLogicalWidth / totalWidth : 1);
              totalWidth -= _layoutStruct[pos].EffectiveMaxLogicalWidth;
              percentMissing -= percent;
              if (percent > 0)
              {
                _layoutStruct[pos].EffectiveLogicalWidth = style::PreferredSize::Percentage {percent};
              }
              else
              {
                _layoutStruct[pos].EffectiveLogicalWidth = css::keywords::Auto {};
              }
            }
            if (totalWidth <= 0)
            {
              break;
            }
          }
        }
      }

      // make sure minWidth and maxWidth of the spanning cell are honored
      if (cellMinLogicalWidth > spanMinLogicalWidth)
      {
        if (allColsAreFixed)
        {
          for (auto pos = static_cast<size_t>(effCol); fixedWidth > 0 && pos < lastCol; ++pos)
          {
            // NOTE: The unchecked use of TryFixed() here is allowed because `allColsAreFixed` is true.
            // FIXME: Find a more type safe way to enforce this invariant.
            auto fixedLogicalWidth = style::Evaluate<float>(
              *_layoutStruct[pos].LogicalWidth.TryFixed(),
              style::ZoomFactor {_layoutStruct[pos].UsedZoom, _table->Style().DeviceScaleFactor()});
            auto columnLogicalWidth = std::max(_layoutStruct[pos].EffectiveMinLogicalWidth,
                                               cellMinLogicalWidth * fixedLogicalWidth / fixedWidth);
            fixedWidth -= fixedLogicalWidth;
            cellMinLogicalWidth -= columnLogicalWidth;
            _layoutStruct[pos].EffectiveMinLogicalWidth = columnLogicalWidth;
          }
        }
        else if (allColsArePercent)
        {
          // In this case, we just split the colspan's min and max widths following the percentage.
          float allocatedMaxLogicalWidth = 0;
          for (auto pos = static_cast<size_t>(effCol); pos < lastCol; ++pos)
          {
            // |allColsArePercent| means that either the LogicalWidth *or* the EffectiveLogicalWidth are
            // percents, handle both of them here.
            auto percentageLogicalWidth = _layoutStruct[pos].LogicalWidth.TryPercentage();
            auto percentageEffectiveLogicalWidth = _layoutStruct[pos].EffectiveLogicalWidth.TryPercentage();
            krys_debug_assert(percentageLogicalWidth || percentageEffectiveLogicalWidth);
            auto percent =
              percentageLogicalWidth ? percentageLogicalWidth->Value : percentageEffectiveLogicalWidth->Value;
            auto columnMinLogicalWidth = percent * cellMinLogicalWidth / totalPercent;
            auto columnMaxLogicalWidth = percent * cellMaxLogicalWidth / totalPercent;
            _layoutStruct[pos].EffectiveMinLogicalWidth =
              std::max(_layoutStruct[pos].EffectiveMinLogicalWidth, columnMinLogicalWidth);
            _layoutStruct[pos].EffectiveMaxLogicalWidth = columnMaxLogicalWidth;
            allocatedMaxLogicalWidth += columnMaxLogicalWidth;
          }
          cellMaxLogicalWidth -= allocatedMaxLogicalWidth;
        }
        else
        {
          auto remainingMaxLogicalWidth = spanMaxLogicalWidth;
          auto remainingMinLogicalWidth = spanMinLogicalWidth;

          // Give min to variable first, to fixed second, and to others third.
          for (auto pos = static_cast<size_t>(effCol); remainingMaxLogicalWidth >= 0 && pos < lastCol; ++pos)
          {
            if (auto fixedLogicalWidth = _layoutStruct[pos].LogicalWidth.TryFixed();
                fixedLogicalWidth && haveAuto && fixedWidth <= cellMinLogicalWidth)
            {
              auto fixedColumnWidth = style::Evaluate<float>(
                *fixedLogicalWidth,
                style::ZoomFactor {_layoutStruct[pos].UsedZoom, _table->Style().DeviceScaleFactor()});
              auto colMinLogicalWidth =
                std::max(_layoutStruct[pos].EffectiveMinLogicalWidth, fixedColumnWidth);
              fixedWidth -= fixedColumnWidth;
              remainingMinLogicalWidth -= _layoutStruct[pos].EffectiveMinLogicalWidth;
              remainingMaxLogicalWidth -= _layoutStruct[pos].EffectiveMaxLogicalWidth;
              cellMinLogicalWidth -= colMinLogicalWidth;
              _layoutStruct[pos].EffectiveMinLogicalWidth = colMinLogicalWidth;
            }
          }

          for (auto pos = static_cast<size_t>(effCol); remainingMaxLogicalWidth >= 0 && pos < lastCol
                                                       && remainingMinLogicalWidth < cellMinLogicalWidth;
               ++pos)
          {
            if (!(_layoutStruct[pos].LogicalWidth.IsFixed() && haveAuto && fixedWidth <= cellMinLogicalWidth))
            {
              auto colMinLogicalWidth = std::max(
                _layoutStruct[pos].EffectiveMinLogicalWidth,
                remainingMaxLogicalWidth ? cellMinLogicalWidth * _layoutStruct[pos].EffectiveMaxLogicalWidth
                                             / remainingMaxLogicalWidth
                                         : cellMinLogicalWidth);
              colMinLogicalWidth = std::min(_layoutStruct[pos].EffectiveMinLogicalWidth
                                              + (cellMinLogicalWidth - remainingMinLogicalWidth),
                                            colMinLogicalWidth);
              remainingMaxLogicalWidth -= _layoutStruct[pos].EffectiveMaxLogicalWidth;
              remainingMinLogicalWidth -= _layoutStruct[pos].EffectiveMinLogicalWidth;
              cellMinLogicalWidth -= colMinLogicalWidth;
              _layoutStruct[pos].EffectiveMinLogicalWidth = colMinLogicalWidth;
            }
          }
        }
      }

      if (!cellLogicalWidth.IsPercentOrCalculated())
      {
        if (cellMaxLogicalWidth > spanMaxLogicalWidth)
        {
          for (auto pos = static_cast<size_t>(effCol); spanMaxLogicalWidth >= 0 && pos < lastCol; ++pos)
          {
            auto colMaxLogicalWidth =
              std::max(_layoutStruct[pos].EffectiveMaxLogicalWidth,
                       spanMaxLogicalWidth ? cellMaxLogicalWidth * _layoutStruct[pos].EffectiveMaxLogicalWidth
                                               / spanMaxLogicalWidth
                                           : cellMaxLogicalWidth);
            spanMaxLogicalWidth -= _layoutStruct[pos].EffectiveMaxLogicalWidth;
            cellMaxLogicalWidth -= colMaxLogicalWidth;
            _layoutStruct[pos].EffectiveMaxLogicalWidth = colMaxLogicalWidth;
          }
        }
      }
      else
      {
        for (auto pos = static_cast<size_t>(effCol); pos < lastCol; ++pos)
        {
          _layoutStruct[pos].MaxLogicalWidth =
            std::max(_layoutStruct[pos].MaxLogicalWidth, _layoutStruct[pos].MinLogicalWidth);
        }
      }

      // treat span ranges consisting of empty cells only as if they had content
      if (spanHasEmptyCellsOnly)
      {
        for (auto pos = static_cast<size_t>(effCol); pos < lastCol; ++pos)
        {
          _layoutStruct[pos].EmptyCellsOnly = false;
        }
      }
    }
    _effectiveLogicalWidthDirty = false;

    return std::min<float>(maxLogicalWidth, TableMaxWidth);
  }

  // Gets all cells that originate in a column and have a cellspan > 1, sorted by increasing cellspan.
  void AutoTableLayout::InsertSpanCell(RenderTableCell *cell) noexcept
  {
    krys_debug_assert(cell && cell->ColSpan() != 1);
    if (!cell || cell->ColSpan() == 1)
    {
      return;
    }

    // Cells with the same span are kept in the reverse order they were added in.
    auto span = cell->ColSpan();
    auto position = std::ranges::find_if(_spanCells, [&](const RenderTableCell *spanCell)
                                         { return span <= spanCell->ColSpan(); });
    _spanCells.insert(position, cell);
  }

  void AutoTableLayout::Layout() noexcept
  {
    // table layout based on the values collected in the layout structure.
    auto tableLogicalWidth =
      (_table->LogicalWidth() - _table->BordersPaddingAndSpacingInRowDirection()).ToFloat();
    auto available = tableLogicalWidth;
    size_t nEffCols = _table->NumEffCols();

    // FIXME: It is possible to be called without having properly updated our internal representation.
    // This means that our preferred logical widths were not recomputed as expected.
    if (nEffCols != _layoutStruct.size())
    {
      FullRecalc();
      // FIXME: Table layout shouldn't modify our table structure (but does due to columns and column-groups).
      nEffCols = _table->NumEffCols();
    }

    if (_effectiveLogicalWidthDirty)
    {
      (void)CalcEffectiveLogicalWidth();
    }

    bool havePercent = false;
    int32 numFixed = 0;
    size_t numberOfNonEmptyAuto = 0;
    Maybe<float> totalAuto;
    float totalFixed = 0;
    float totalPercent = 0;
    float allocAuto = 0;
    uint32 numAutoEmptyCellsOnly = 0;

    // fill up every cell with its minWidth
    for (size_t i = 0; i < nEffCols; ++i)
    {
      auto cellLogicalWidth = _layoutStruct[i].EffectiveMinLogicalWidth;
      _layoutStruct[i].ComputedLogicalWidth = cellLogicalWidth;
      available -= cellLogicalWidth;
      _layoutStruct[i].EffectiveLogicalWidth.SwitchOn(
        [&](const style::PreferredSize::Fixed &)
        {
          numFixed++;
          totalFixed += _layoutStruct[i].EffectiveMaxLogicalWidth;
        },
        [&](const style::PreferredSize::Percentage &percentageLogicalWidth)
        {
          havePercent = true;
          totalPercent += percentageLogicalWidth.Value;
        },
        [&](const css::keywords::Auto &)
        {
          if (_layoutStruct[i].EmptyCellsOnly)
          {
            numAutoEmptyCellsOnly++;
          }
          else
          {
            ++numberOfNonEmptyAuto;
            totalAuto = totalAuto.value_or(0.f) + _layoutStruct[i].EffectiveMaxLogicalWidth;
            allocAuto += cellLogicalWidth;
          }
        },
        [&](const auto &) {});
    }

    // allocate width to percent cols
    if (available > 0 && havePercent)
    {
      for (auto &columnLayout : _layoutStruct)
      {
        const auto &logicalWidth = columnLayout.EffectiveLogicalWidth;
        if (logicalWidth.IsPercentOrCalculated())
        {
          auto cellLogicalWidth = std::max<float>(
            columnLayout.EffectiveMinLogicalWidth,
            style::EvaluateMinimum<float>(
              logicalWidth, tableLogicalWidth,
              style::ZoomFactor {columnLayout.UsedZoom, _table->Style().DeviceScaleFactor()}));
          available += columnLayout.ComputedLogicalWidth - cellLogicalWidth;
          columnLayout.ComputedLogicalWidth = cellLogicalWidth;
        }
      }
      if (totalPercent > 100)
      {
        // remove overallocated space from the last columns
        auto excess = tableLogicalWidth * (totalPercent - 100) / 100;
        for (auto i = nEffCols; i;)
        {
          --i;
          if (_layoutStruct[i].EffectiveLogicalWidth.IsPercentOrCalculated())
          {
            auto cellLogicalWidth = _layoutStruct[i].ComputedLogicalWidth;
            auto reduction = std::min(cellLogicalWidth, excess);
            // the lines below might look inconsistent, but that's the way it's handled in mozilla
            excess -= reduction;
            auto newLogicalWidth =
              std::max(_layoutStruct[i].EffectiveMinLogicalWidth, cellLogicalWidth - reduction);
            available += cellLogicalWidth - newLogicalWidth;
            _layoutStruct[i].ComputedLogicalWidth = newLogicalWidth;
          }
        }
      }
    }

    // then allocate width to fixed cols
    if (available > 0)
    {
      for (auto &columnLayout : _layoutStruct)
      {
        if (auto fixedLogicalWidth = columnLayout.EffectiveLogicalWidth.TryFixed())
        {
          auto fixedWidth = style::Evaluate<float>(
            *fixedLogicalWidth,
            style::ZoomFactor {columnLayout.UsedZoom, _table->Style().DeviceScaleFactor()});
          if (fixedWidth > columnLayout.ComputedLogicalWidth)
          {
            available += columnLayout.ComputedLogicalWidth - fixedWidth;
            columnLayout.ComputedLogicalWidth = fixedWidth;
          }
        }
      }
    }

    // now satisfy variable
    if (available > 0 && numberOfNonEmptyAuto)
    {
      krys_debug_assert(totalAuto);
      available += allocAuto; // this gets redistributed.
      Maybe<float> equalWidthForZeroLengthColumns;
      if (!*totalAuto)
      {
        // All columns in this table are (non-empty)zero length with 'width: auto'.
        equalWidthForZeroLengthColumns = available / static_cast<float>(numberOfNonEmptyAuto);
      }

      for (auto &column : _layoutStruct)
      {
        if (!column.EffectiveLogicalWidth.IsAuto() || column.EmptyCellsOnly)
        {
          continue;
        }

        auto columnWidthCandidate = equalWidthForZeroLengthColumns
                                      ? *equalWidthForZeroLengthColumns
                                      : available * column.EffectiveMaxLogicalWidth / *totalAuto;
        column.ComputedLogicalWidth = std::max(column.ComputedLogicalWidth, columnWidthCandidate);
        available -= column.ComputedLogicalWidth;
        if (!equalWidthForZeroLengthColumns)
        {
          *totalAuto -= column.EffectiveMaxLogicalWidth;
          if (*totalAuto <= 0)
          {
            break;
          }
        }
      }
    }

    // spread over fixed columns
    if (available > 0 && numFixed)
    {
      for (auto &columnLayout : _layoutStruct)
      {
        if (columnLayout.EffectiveLogicalWidth.IsFixed())
        {
          auto cellLogicalWidth = available * columnLayout.EffectiveMaxLogicalWidth / totalFixed;
          available -= cellLogicalWidth;
          totalFixed -= columnLayout.EffectiveMaxLogicalWidth;
          columnLayout.ComputedLogicalWidth += cellLogicalWidth;
        }
      }
    }

    // spread over percent columns
    if (available > 0 && _hasPercent && totalPercent < 100)
    {
      for (auto &columnLayout : _layoutStruct)
      {
        if (auto percentageLogicalWidth = columnLayout.EffectiveLogicalWidth.TryPercentage())
        {
          auto cellLogicalWidth = available * percentageLogicalWidth->Value / totalPercent;
          available -= cellLogicalWidth;
          totalPercent -= percentageLogicalWidth->Value;
          columnLayout.ComputedLogicalWidth += cellLogicalWidth;
          if (!available || !totalPercent)
          {
            break;
          }
        }
      }
    }

    // spread over the rest
    if (available > 0 && nEffCols > numAutoEmptyCellsOnly)
    {
      auto total = static_cast<uint32>(nEffCols - numAutoEmptyCellsOnly);
      // still have some width to spread
      for (auto i = nEffCols; i;)
      {
        --i;
        // variable columns with empty cells only don't get any width
        if (_layoutStruct[i].EffectiveLogicalWidth.IsAuto() && _layoutStruct[i].EmptyCellsOnly)
        {
          continue;
        }
        auto cellLogicalWidth = available / static_cast<float>(total);
        available -= cellLogicalWidth;
        total--;
        _layoutStruct[i].ComputedLogicalWidth += cellLogicalWidth;
      }
    }

    if (available > 0 && numAutoEmptyCellsOnly && nEffCols == numAutoEmptyCellsOnly)
    {
      // All columns in this table are empty with 'width: auto'.
      auto equalWidthForColumns = available / static_cast<float>(numAutoEmptyCellsOnly);
      for (auto &column : _layoutStruct)
      {
        column.ComputedLogicalWidth = equalWidthForColumns;
        available -= column.ComputedLogicalWidth;
      }
    }

    // If we have over-allocated, reduce every cell according to the difference between desired width and
    // min-width this seems to produce to the pixel exact results with IE. Wonder if some of this also holds
    // for width distributing. Need to reduce cells with the following prioritization: This is basically the
    // reverse of how we grew the cells.
    if (available < 0)
    {
      available = ShrinkCellWidthForType<css::keywords::Auto>(available);
    }
    if (available < 0)
    {
      available = ShrinkCellWidthForType<style::PreferredSize::Fixed>(available);
    }
    if (available < 0)
    {
      available = ShrinkCellWidthForType<style::PreferredSize::Percentage>(available);
    }

    LayoutUnit pos;
    for (size_t i = 0; i < nEffCols; ++i)
    {
      _table->SetColumnPosition(static_cast<uint32>(i), pos);
      pos += LayoutUnit::FromFloatCeil(_layoutStruct[i].ComputedLogicalWidth) + _table->HBorderSpacing();
    }
    _table->SetColumnPosition(static_cast<uint32>(_table->ColumnPositions().size() - 1), pos);
  }

  template <typename T>
  float AutoTableLayout::ShrinkCellWidthForType(float available) noexcept
  {
    auto nEffCols = _table->NumEffCols();
    float logicalWidthBeyondMin = 0;
    for (auto i = nEffCols; i;)
    {
      --i;
      if (_layoutStruct[i].EffectiveLogicalWidth.template HoldsAlternative<T>())
      {
        logicalWidthBeyondMin +=
          _layoutStruct[i].ComputedLogicalWidth - _layoutStruct[i].EffectiveMinLogicalWidth;
      }
    }

    for (auto i = nEffCols; i && logicalWidthBeyondMin > 0;)
    {
      --i;
      if (_layoutStruct[i].EffectiveLogicalWidth.template HoldsAlternative<T>())
      {
        auto minMaxDiff = _layoutStruct[i].ComputedLogicalWidth - _layoutStruct[i].EffectiveMinLogicalWidth;
        auto reduce = available * minMaxDiff / logicalWidthBeyondMin;
        _layoutStruct[i].ComputedLogicalWidth += reduce;
        available -= reduce;
        logicalWidthBeyondMin -= minMaxDiff;
        if (available >= 0)
        {
          break;
        }
      }
    }
    return available;
  }
}
