#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Style/CollapsedBorderValue.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::html
{
  class HTMLTableCellElement;
}

namespace krys::boo
{
  class RenderTable;
  class RenderTableCol;
  class RenderTableRow;
  class RenderTableSection;
  enum class CollapsedBorderSide : uint8;
  struct BorderValue;

  enum class IncludeBorderColor : bool
  {
    No,
    Yes
  };

  /// @brief A table cell: its place in the grid, its spans, its collapsed borders and its layout.
  // FIXME: Painting (paint, paintCollapsedBorders, paintBackgroundsBehindCell, paintBoxDecorations,
  // paintMask, the collapsed borders collected for painting), repaints (localRectsForRepaint,
  // computeVisibleRectsInContainer) and sticky positioning (frameRectForStickyPositioning), once there are
  // those.
  class RenderTableCell final : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderTableCell);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTableCell);

  public:
    // These is limited by the size of RenderTableCell::_column bitfield.
    constexpr static uint32 UnsetColumnIndex = 0x1FFFFFF;
    constexpr static uint32 MaxColumnIndex = 0x1FFFFFE; // 33554430

  private:
    uint32 _column : 25 {UnsetColumnIndex};
    uint32 _cellWidthChanged : 1 {false};
    uint32 _hasColSpan : 1 {false};
    uint32 _hasRowSpan : 1 {false};
    mutable uint32 _hasEmptyCollapsedBeforeBorder : 1 {false};
    mutable uint32 _hasEmptyCollapsedAfterBorder : 1 {false};
    mutable uint32 _hasEmptyCollapsedStartBorder : 1 {false};
    mutable uint32 _hasEmptyCollapsedEndBorder : 1 {false};
    bool _isComputingPreferredSize {false};
    LayoutUnit _intrinsicPaddingBefore {0};
    LayoutUnit _intrinsicPaddingAfter {0};
    mutable Maybe<LayoutUnit> _orthogonalCellContentIntrinsicHeight;

  public:
    RenderTableCell(dom::Element &element, RenderStyle &&style) noexcept;
    RenderTableCell(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderTableCell() noexcept override;

    KRYS_NODISCARD uint32 ColSpan() const noexcept
    {
      if (!_hasColSpan)
      {
        return 1;
      }
      return ParseColSpanFromDOM();
    }
    KRYS_NODISCARD uint32 RowSpan() const noexcept
    {
      if (!_hasRowSpan)
      {
        return 1;
      }
      return ParseRowSpanFromDOM();
    }

    /// @brief Called when the element's colspan or rowspan changes.
    void ColSpanOrRowSpanChanged() noexcept;

    void SetCol(uint32 column) noexcept
    {
      if (column > MaxColumnIndex) KRYS_UNLIKELY
      {
        column = MaxColumnIndex;
      }
      _column = column;
    }
    KRYS_NODISCARD uint32 Col() const noexcept
    {
      krys_debug_assert(_column != UnsetColumnIndex);
      return _column;
    }

    KRYS_NODISCARD RenderTableCell *NextCell() const noexcept;
    KRYS_NODISCARD RenderTableCell *PreviousCell() const noexcept;

    KRYS_NODISCARD RenderTableRow *Row() const noexcept;
    KRYS_NODISCARD RenderTableSection *Section() const noexcept;
    KRYS_NODISCARD RenderTable *Table() const noexcept;

    KRYS_NODISCARD uint32 RowIndex() const noexcept;

    KRYS_NODISCARD Pair<style::PreferredSize, style::ZoomFactor> StyleOrColLogicalWidth() const noexcept;

    KRYS_NODISCARD LayoutUnit LogicalHeightForRowSizing() const noexcept;

    KRYS_NODISCARD LayoutUnit MinLogicalWidthForColumnSizing() noexcept;
    KRYS_NODISCARD LayoutUnit MaxLogicalWidthForColumnSizing() noexcept;

    void SetCellLogicalWidth(LayoutUnit logicalWidthInTableDirection) noexcept;

    KRYS_NODISCARD RectEdges<LayoutUnit> BorderWidths() const noexcept override;
    KRYS_NODISCARD LayoutUnit BorderLeft() const noexcept override;
    KRYS_NODISCARD LayoutUnit BorderRight() const noexcept override;
    KRYS_NODISCARD LayoutUnit BorderTop() const noexcept override;
    KRYS_NODISCARD LayoutUnit BorderBottom() const noexcept override;
    KRYS_NODISCARD LayoutUnit BorderStart() const noexcept override;
    KRYS_NODISCARD LayoutUnit BorderEnd() const noexcept override;
    KRYS_NODISCARD LayoutUnit BorderBefore() const noexcept override;
    KRYS_NODISCARD LayoutUnit BorderAfter() const noexcept override;

    KRYS_NODISCARD LayoutSize
      OffsetFromContainer(const RenderElement &container, const LayoutPoint &point,
                          bool *offsetDependsOnPoint = nullptr) const noexcept override;

    void Layout() noexcept override;

    KRYS_NODISCARD LayoutUnit CellBaselinePosition() const noexcept;
    KRYS_NODISCARD bool IsBaselineAligned() const noexcept;

    bool ComputeIntrinsicPadding(LayoutUnit heightConstraint) noexcept;
    void ClearIntrinsicPadding() noexcept
    {
      SetIntrinsicPadding(0_lu, 0_lu);
    }

    KRYS_NODISCARD LayoutUnit IntrinsicPaddingBefore() const noexcept
    {
      return _intrinsicPaddingBefore;
    }
    KRYS_NODISCARD LayoutUnit IntrinsicPaddingAfter() const noexcept
    {
      return _intrinsicPaddingAfter;
    }

    KRYS_NODISCARD RectEdges<LayoutUnit> Padding() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingTop() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingBottom() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingLeft() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingRight() const noexcept override;

    // FIXME: For now we just assume the cell has the same block flow direction as the table. It's likely
    // we'll create an extra anonymous RenderBlock to handle mixing directionality anyway, in which case we
    // can lock the block flow directionality of the cells to the table's directionality.
    KRYS_NODISCARD LayoutUnit PaddingBefore() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingAfter() const noexcept override;

    void SetOverridingLogicalHeightFromRowHeight(LayoutUnit rowHeight) noexcept;

    void ScrollbarsChanged(bool horizontalScrollbarChanged, bool verticalScrollbarChanged) noexcept override;

    KRYS_NODISCARD bool CellWidthChanged() const noexcept
    {
      return _cellWidthChanged;
    }
    void SetCellWidthChanged(bool b = true) noexcept
    {
      _cellWidthChanged = b;
    }

    /// @brief The table's writing mode, which table layout always uses.
    KRYS_NODISCARD WritingMode TableWritingMode() const noexcept;

    KRYS_NODISCARD const BorderValue &BorderAdjoiningTableStart() const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningTableEnd() const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningCellBefore(const RenderTableCell &cell) const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAdjoiningCellAfter(const RenderTableCell &cell) const noexcept;

#if KRYS_DEBUG
    KRYS_NODISCARD bool IsFirstOrLastCellInRow() const noexcept;
#endif

    void InvalidateHasEmptyCollapsedBorders() noexcept
    {
      _hasEmptyCollapsedBeforeBorder = false;
      _hasEmptyCollapsedAfterBorder = false;
      _hasEmptyCollapsedStartBorder = false;
      _hasEmptyCollapsedEndBorder = false;
    }
    void SetHasEmptyCollapsedBorder(CollapsedBorderSide side, bool empty) const noexcept;

    KRYS_NODISCARD bool IsOrthogonal() const noexcept;
    KRYS_NODISCARD bool IsComputingPreferredSize() const noexcept
    {
      return _isComputingPreferredSize;
    }

    KRYS_NODISCARD CollapsedBorderValue
      CollapsedStartBorder(IncludeBorderColor includeColor = IncludeBorderColor::Yes) const noexcept;
    KRYS_NODISCARD CollapsedBorderValue
      CollapsedEndBorder(IncludeBorderColor includeColor = IncludeBorderColor::Yes) const noexcept;
    KRYS_NODISCARD CollapsedBorderValue
      CollapsedBeforeBorder(IncludeBorderColor includeColor = IncludeBorderColor::Yes) const noexcept;
    KRYS_NODISCARD CollapsedBorderValue
      CollapsedAfterBorder(IncludeBorderColor includeColor = IncludeBorderColor::Yes) const noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return (IsAnonymous() || IsPseudoElement()) ? u8"RenderTableCell (anonymous)" : u8"RenderTableCell";
    }

  protected:
    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;

  private:
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;
    void ComputePreferredLogicalWidths() noexcept override;
    void WillBeRemovedFromTree() noexcept override;
    void UpdateLogicalWidth() noexcept override;

    KRYS_NODISCARD LayoutUnit BorderHalfLeft(bool outer) const noexcept;
    KRYS_NODISCARD LayoutUnit BorderHalfRight(bool outer) const noexcept;
    KRYS_NODISCARD LayoutUnit BorderHalfTop(bool outer) const noexcept;
    KRYS_NODISCARD LayoutUnit BorderHalfBottom(bool outer) const noexcept;

    KRYS_NODISCARD LayoutUnit BorderHalfStart(bool outer) const noexcept;
    KRYS_NODISCARD LayoutUnit BorderHalfEnd(bool outer) const noexcept;
    KRYS_NODISCARD LayoutUnit BorderHalfBefore(bool outer) const noexcept;
    KRYS_NODISCARD LayoutUnit BorderHalfAfter(bool outer) const noexcept;

    void SetIntrinsicPaddingBefore(LayoutUnit p) noexcept
    {
      _intrinsicPaddingBefore = p;
    }
    void SetIntrinsicPaddingAfter(LayoutUnit p) noexcept
    {
      _intrinsicPaddingAfter = p;
    }
    void SetIntrinsicPadding(LayoutUnit before, LayoutUnit after) noexcept
    {
      SetIntrinsicPaddingBefore(before);
      SetIntrinsicPaddingAfter(after);
    }

    KRYS_NODISCARD bool HasStartBorderAdjoiningTable() const noexcept;
    KRYS_NODISCARD bool HasEndBorderAdjoiningTable() const noexcept;

    KRYS_NODISCARD CollapsedBorderValue
      ComputeCollapsedStartBorder(IncludeBorderColor includeColor) const noexcept;
    KRYS_NODISCARD CollapsedBorderValue
      ComputeCollapsedEndBorder(IncludeBorderColor includeColor) const noexcept;
    KRYS_NODISCARD CollapsedBorderValue
      ComputeCollapsedBeforeBorder(IncludeBorderColor includeColor) const noexcept;
    KRYS_NODISCARD CollapsedBorderValue
      ComputeCollapsedAfterBorder(IncludeBorderColor includeColor) const noexcept;

    KRYS_NODISCARD style::PreferredSize
      LogicalWidthFromColumns(RenderTableCol *firstColForThisCell,
                              const style::PreferredSize &widthFromStyle) const noexcept;

    KRYS_NODISCARD CollapsedBorderValue EmptyBorder() const noexcept;

    KRYS_NODISCARD const html::HTMLTableCellElement *TableCellElement() const noexcept;
    void UpdateColAndRowSpanFlags() noexcept;
    KRYS_NODISCARD uint32 ParseRowSpanFromDOM() const noexcept;
    KRYS_NODISCARD uint32 ParseColSpanFromDOM() const noexcept;

    KRYS_NODISCARD bool HasLineIfEmpty() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTableCell)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTableCell();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
