#include "Krystal.Booey/Rendering/FixedTableLayout.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableCol.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderTableSection.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

/*
  The text below is from the CSS 2.1 specs.

  Fixed table layout

  With this (fast) algorithm, the horizontal layout of the table does
  not depend on the contents of the cells; it only depends on the
  table's width, the width of the columns, and borders or cell
  spacing.

  The table's width may be specified explicitly with the 'width'
  property. A value of 'auto' (for both 'display: table' and 'display:
  inline-table') means use the automatic table layout algorithm.

  In the fixed table layout algorithm, the width of each column is
  determined as follows:

    1. A column element with a value other than 'auto' for the 'width'
    property sets the width for that column.

    2. Otherwise, a cell in the first row with a value other than
    'auto' for the 'width' property sets the width for that column. If
    the cell spans more than one column, the width is divided over the
    columns.

    3. Any remaining columns equally divide the remaining horizontal
    table space (minus borders or cell spacing).

  The width of the table is then the greater of the value of the
  'width' property for the table element and the sum of the column
  widths (plus cell spacing or borders). If the table is wider than
  the columns, the extra space should be distributed over the columns.

  In this manner, the user agent can begin to lay out the table once
  the entire first row has been received. Cells in subsequent rows do
  not affect column widths. Any cell that has content that overflows
  uses the 'overflow' property to determine whether to clip the
  overflow content.
*/

namespace krys::boo
{
  FixedTableLayout::FixedTableLayout(RenderTable *table) noexcept : TableLayout(table)
  {
  }

  float FixedTableLayout::CalcWidthArray() noexcept
  {
    // FIXME: We might want to wait until we have all of the first row before computing for the first time.
    float usedWidth = 0;

    // iterate over all <col> elements
    auto nEffCols = _table->NumEffCols();
    _width.assign(nEffCols, style::PreferredSize {css::keywords::Auto {}});

    uint32 currentEffectiveColumn = 0;
    for (auto *col = _table->FirstColumn(); col; col = col->NextColumn())
    {
      // RenderTableCols don't have the concept of preferred logical width, but we need to clear their dirty
      // bits so that if we call SetNeedsPreferredWidthsUpdate() on a col or one of its descendants, we'll
      // mark its ancestors as dirty.
      col->ClearNeedsPreferredLogicalWidthsUpdate();

      // Width specified by column-groups that have column child does not affect column width in fixed layout
      // tables
      if (col->IsTableColumnGroupWithColumnChildren())
      {
        continue;
      }

      auto colStyleLogicalWidth = col->Style().LogicalWidth();
      auto colUsedZoom = col->Style().UsedZoomForLength();
      float effectiveColWidth = 0;
      if (auto fixedColStyleLogicalWidth = colStyleLogicalWidth.TryFixed();
          fixedColStyleLogicalWidth && fixedColStyleLogicalWidth->IsPositive())
      {
        effectiveColWidth = style::Evaluate<float>(*fixedColStyleLogicalWidth, colUsedZoom);
      }
      else if (colStyleLogicalWidth.IsCalculated())
      {
        colStyleLogicalWidth = css::keywords::Auto {};
      }

      auto span = col->Span();
      while (span)
      {
        uint32 spanInCurrentEffectiveColumn;
        if (currentEffectiveColumn >= nEffCols)
        {
          _table->AppendColumn(span);
          nEffCols++;
          _width.emplace_back(css::keywords::Auto {});
          spanInCurrentEffectiveColumn = span;
        }
        else
        {
          if (span < _table->SpanOfEffCol(currentEffectiveColumn))
          {
            _table->SplitColumn(currentEffectiveColumn, span);
            nEffCols++;
            _width.emplace_back(css::keywords::Auto {});
          }
          spanInCurrentEffectiveColumn = _table->SpanOfEffCol(currentEffectiveColumn);
        }

        if (auto fixedColStyleLogicalWidth = colStyleLogicalWidth.TryFixed();
            fixedColStyleLogicalWidth && fixedColStyleLogicalWidth->IsPositive())
        {
          _width[currentEffectiveColumn] = style::PreferredSize::Fixed {
            style::Evaluate<float>(*fixedColStyleLogicalWidth, colUsedZoom) * spanInCurrentEffectiveColumn};
          usedWidth += effectiveColWidth * spanInCurrentEffectiveColumn;
        }
        else if (auto percentageColStyleLogicalWidth = colStyleLogicalWidth.TryPercentage();
                 percentageColStyleLogicalWidth && percentageColStyleLogicalWidth->Value > 0)
        {
          _width[currentEffectiveColumn] = style::PreferredSize::Percentage {
            percentageColStyleLogicalWidth->Value * spanInCurrentEffectiveColumn};
          usedWidth += effectiveColWidth * spanInCurrentEffectiveColumn;
        }
        span -= spanInCurrentEffectiveColumn;
        currentEffectiveColumn++;
      }
    }

    // Iterate over the first row in case some are unspecified.
    auto *section = _table->TopNonEmptySection();
    if (!section)
    {
      return usedWidth;
    }

    uint32 currentColumn = 0;

    auto *firstRow = section->FirstRow();
    for (auto *cell = firstRow->FirstCell(); cell; cell = cell->NextCell())
    {
      auto [logicalWidth, usedZoom] = cell->StyleOrColLogicalWidth();
      auto span = cell->ColSpan();
      float fixedBorderBoxLogicalWidth = 0;
      // FIXME: Support other length types. If the width is non-auto, it should probably just use
      // RenderBox::computeLogicalWidthInFragmentUsing to compute the width.
      if (auto fixedLogicalWidth = logicalWidth.TryFixed();
          fixedLogicalWidth && fixedLogicalWidth->IsPositive())
      {
        fixedBorderBoxLogicalWidth =
          cell->AdjustBorderBoxLogicalWidthForBoxSizing(*fixedLogicalWidth).ToFloat();
        logicalWidth = style::PreferredSize::Fixed {fixedBorderBoxLogicalWidth};
      }
      else if (logicalWidth.IsCalculated())
      {
        logicalWidth = css::keywords::Auto {};
      }

      uint32 usedSpan = 0;
      while (usedSpan < span && currentColumn < nEffCols)
      {
        auto eSpan = static_cast<float>(_table->SpanOfEffCol(currentColumn));
        // Only set if no col element has already set it.
        if (_width[currentColumn].IsAuto() && !logicalWidth.IsAuto())
        {
          if (auto fixedLogicalWidth = logicalWidth.TryFixed())
          {
            // Zoom was applied when we called AdjustBorderBoxLogicalWidthForBoxSizing
            _width[currentColumn] = style::PreferredSize::Fixed {
              style::Evaluate<float>(*fixedLogicalWidth, style::ZoomFactor {1.0f, 1.0f}) * eSpan
              / static_cast<float>(span)};
          }
          else if (auto percentageLogicalWidth = logicalWidth.TryPercentage())
          {
            _width[currentColumn] = style::PreferredSize::Percentage {percentageLogicalWidth->Value * eSpan
                                                                      / static_cast<float>(span)};
          }
          usedWidth += fixedBorderBoxLogicalWidth * eSpan / static_cast<float>(span);
        }
        usedSpan += static_cast<uint32>(eSpan);
        ++currentColumn;
      }

      // FixedTableLayout doesn't use min/maxPreferredLogicalWidths, but we need to clear the
      // dirty bit on the cell so that we'll correctly mark its ancestors dirty
      // in case we later call SetNeedsPreferredWidthsUpdate() on it later.
      if (cell->NeedsPreferredLogicalWidthsUpdate())
      {
        cell->ClearNeedsPreferredWidthsUpdate();
      }
    }

    return usedWidth;
  }

  void FixedTableLayout::ComputeIntrinsicLogicalWidths(LayoutUnit &minWidth, LayoutUnit &maxWidth,
                                                       TableIntrinsics) noexcept
  {
    minWidth = maxWidth = LayoutUnit(CalcWidthArray());
  }

  void FixedTableLayout::ApplyPreferredLogicalWidthQuirks(LayoutUnit &minWidth,
                                                          LayoutUnit &maxWidth) const noexcept
  {
    const auto &tableLogicalWidth = _table->Style().LogicalWidth();
    if (auto fixedTableLogicalWidth = tableLogicalWidth.TryFixed();
        fixedTableLogicalWidth && fixedTableLogicalWidth->IsPositive())
    {
      minWidth = maxWidth = std::max(
        minWidth, style::Evaluate<LayoutUnit>(*fixedTableLogicalWidth, _table->Style().UsedZoomForLength())
                    - _table->BordersPaddingAndSpacingInRowDirection());
    }

    /*
        <table style="width:100%; background-color:red"><tr><td>
            <table style="background-color:blue"><tr><td>
                <table style="width:100%; background-color:green; table-layout:fixed"><tr><td>
                    Content
                </td></tr></table>
            </td></tr></table>
        </td></tr></table>
    */
    // In this example, the two inner tables should be as large as the outer table.
    // We can achieve this effect by making the max-width of fixed tables with percentage
    // widths be infinite.
    if (tableLogicalWidth.IsPercentOrCalculated() && maxWidth < TableMaxWidth)
    {
      maxWidth = LayoutUnit(TableMaxWidth);
    }
  }

  void FixedTableLayout::Layout() noexcept
  {
    auto tableLogicalWidth =
      (_table->LogicalWidth() - _table->BordersPaddingAndSpacingInRowDirection()).ToFloat();
    auto nEffCols = _table->NumEffCols();

    // FIXME: It is possible to be called without having properly updated our internal representation.
    // This means that our preferred logical widths were not recomputed as expected.
    if (nEffCols != _width.size())
    {
      (void)CalcWidthArray();
      // FIXME: Table layout shouldn't modify our table structure (but does due to columns and column-groups).
      nEffCols = _table->NumEffCols();
    }

    List<float> calcWidth(nEffCols, 0);

    uint32 numAuto = 0;
    uint32 autoSpan = 0;
    float totalFixedWidth = 0;
    float totalPercentWidth = 0;
    float totalPercent = 0;

    // Compute requirements and try to satisfy fixed and percent widths.
    // Percentages are of the table's width, so for example
    // for a table width of 100px with columns (40px, 10%), the 10% compute
    // to 10px here, and will scale up to 20px in the final (80px, 20px).
    for (uint32 i = 0; i < nEffCols; i++)
    {
      if (auto fixedWidth = _width[i].TryFixed())
      {
        // We zoomed the widths inside of CalcWidthArray so here we pass in a zoom
        // factor of 1.0f to avoid double zooming.
        calcWidth[i] = style::Evaluate<float>(*fixedWidth, style::ZoomFactor {1.0f, 1.0f});
        totalFixedWidth += calcWidth[i];
      }
      else if (auto percentageWidth = _width[i].TryPercentage())
      {
        calcWidth[i] = style::Evaluate<float>(*percentageWidth, tableLogicalWidth);
        totalPercentWidth += calcWidth[i];
        totalPercent += percentageWidth->Value;
      }
      else if (_width[i].IsAuto())
      {
        numAuto++;
        autoSpan += _table->SpanOfEffCol(i);
      }
    }

    auto hspacing = _table->HBorderSpacing().ToFloat();
    auto totalWidth = totalFixedWidth + totalPercentWidth;
    if (!numAuto || totalWidth > tableLogicalWidth)
    {
      // If there are no auto columns, or if the total is too wide, take
      // what we have and scale it to fit as necessary.
      if (totalWidth != tableLogicalWidth)
      {
        // Fixed widths only scale up
        if (totalFixedWidth && totalWidth < tableLogicalWidth)
        {
          totalFixedWidth = 0;
          for (uint32 i = 0; i < nEffCols; i++)
          {
            if (_width[i].IsFixed())
            {
              calcWidth[i] = calcWidth[i] * tableLogicalWidth / totalWidth;
              totalFixedWidth += calcWidth[i];
            }
          }
        }
        if (totalPercent)
        {
          totalPercentWidth = 0;
          for (uint32 i = 0; i < nEffCols; i++)
          {
            if (auto percentageWidth = _width[i].TryPercentage())
            {
              calcWidth[i] = percentageWidth->Value * (tableLogicalWidth - totalFixedWidth) / totalPercent;
              totalPercentWidth += calcWidth[i];
            }
          }
        }
        totalWidth = totalFixedWidth + totalPercentWidth;
      }
    }
    else
    {
      // Divide the remaining width among the auto columns.
      krys_debug_assert(autoSpan >= numAuto);
      auto remainingWidth = tableLogicalWidth - totalFixedWidth - totalPercentWidth
                            - hspacing * static_cast<float>(autoSpan - numAuto);
      uint32 lastAuto = 0;
      for (uint32 i = 0; i < nEffCols; i++)
      {
        if (_width[i].IsAuto())
        {
          auto span = _table->SpanOfEffCol(i);
          auto w = remainingWidth * static_cast<float>(span) / static_cast<float>(autoSpan);
          calcWidth[i] = w + hspacing * static_cast<float>(span - 1);
          remainingWidth -= w;
          if (!remainingWidth)
          {
            break;
          }
          lastAuto = i;
          numAuto--;
          krys_debug_assert(autoSpan >= span);
          autoSpan -= span;
        }
      }
      // Last one gets the remainder.
      if (remainingWidth)
      {
        calcWidth[lastAuto] += remainingWidth;
      }
      totalWidth = tableLogicalWidth;
    }

    if (totalWidth < tableLogicalWidth)
    {
      // Spread extra space over columns.
      auto remainingWidth = tableLogicalWidth - totalWidth;
      auto total = nEffCols;
      while (total)
      {
        auto w = remainingWidth / static_cast<float>(total);
        remainingWidth -= w;
        calcWidth[--total] += w;
      }
      if (nEffCols > 0)
      {
        calcWidth[nEffCols - 1] += remainingWidth;
      }
    }

    float pos = 0;
    for (uint32 i = 0; i < nEffCols; i++)
    {
      _table->SetColumnPosition(i, LayoutUnit(pos));
      pos += calcWidth[i] + hspacing;
    }
    auto colPositionsSize = _table->ColumnPositions().size();
    if (colPositionsSize > 0)
    {
      _table->SetColumnPosition(static_cast<uint32>(colPositionsSize - 1), LayoutUnit(pos));
    }
  }
}
