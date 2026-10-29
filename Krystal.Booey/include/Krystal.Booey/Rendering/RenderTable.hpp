#pragma once

#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/CollapsedBorderValue.hpp"
#include "Krystal.Booey/Rendering/TableLayout.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderTableCaption;
  class RenderTableCell;
  class RenderTableCol;
  class RenderTableSection;

  enum class SkipEmptySections : bool
  {
    No,
    Yes
  };

  /// @brief A table: its sections, columns and captions, its column structure and its collapsed borders.
  class RenderTable : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderTable);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTable);

  public:
    /// @brief An effective column: a run of columns that no cell starts or ends inside.
    struct ColumnStruct
    {
      uint32 Span {1};
    };

  protected:
    mutable List<LayoutUnit> _columnPos;
    mutable List<ColumnStruct> _columns;
    mutable List<WeakPtr<RenderTableCaption>> _captions;
    mutable List<WeakPtr<RenderTableCol>> _columnRenderers;

    // Keyed by address: cleared with the column renderers (InvalidateCachedColumns).
    mutable HashMap<const RenderTableCol *, uint32> _effectiveColumnIndexMap;

    mutable WeakPtr<RenderTableSection> _head;
    mutable WeakPtr<RenderTableSection> _foot;
    mutable WeakPtr<RenderTableSection> _firstBody;

    UniquePtr<TableLayout> _tableLayout;

    /// @brief The distinct collapsed borders, from the lowest precedence to the highest, painted in that
    /// order.
    List<CollapsedBorderValue> _collapsedBorders;
    /// @brief The collapsed border being painted, during the collapsed borders phase.
    const CollapsedBorderValue *_currentBorder {nullptr};
    bool _collapsedBordersValid : 1 {false};
    bool _collapsedEmptyBorderIsPresent : 1 {false};
    mutable bool _hasColElements : 1 {false};
    mutable bool _needsSectionRecalc : 1 {false};
    bool _columnLogicalWidthChanged : 1 {false};
    mutable bool _columnRenderersValid : 1 {false};
    mutable bool _hasCellColspanThatDeterminesTableWidth : 1 {false};

    LayoutUnit _hSpacing;
    LayoutUnit _vSpacing;
    LayoutUnit _borderStart;
    LayoutUnit _borderEnd;
    mutable LayoutUnit _columnOffsetTop {-1};
    mutable LayoutUnit _columnOffsetHeight {-1};

  public:
    RenderTable(Type type, dom::Element &element, RenderStyle &&style) noexcept;
    RenderTable(Type type, dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderTable() noexcept override;

    // Per CSS 3 writing-mode: "The first and second values of the 'border-spacing' property represent spacing
    // between columns and rows respectively, not necessarily the horizontal and vertical spacing
    // respectively".
    KRYS_NODISCARD LayoutUnit HBorderSpacing() const noexcept
    {
      return _hSpacing;
    }
    KRYS_NODISCARD LayoutUnit VBorderSpacing() const noexcept
    {
      return _vSpacing;
    }

    KRYS_NODISCARD bool CollapseBorders() const noexcept;

    KRYS_NODISCARD LayoutUnit BorderStart() const noexcept final
    {
      return _borderStart;
    }
    KRYS_NODISCARD LayoutUnit BorderEnd() const noexcept final
    {
      return _borderEnd;
    }
    KRYS_NODISCARD LayoutUnit BorderBefore() const noexcept final;
    KRYS_NODISCARD LayoutUnit BorderAfter() const noexcept final;

    KRYS_NODISCARD RectEdges<LayoutUnit> BorderWidths() const noexcept final;
    KRYS_NODISCARD LayoutUnit BorderLeft() const noexcept final;
    KRYS_NODISCARD LayoutUnit BorderRight() const noexcept final;
    KRYS_NODISCARD LayoutUnit BorderTop() const noexcept final;
    KRYS_NODISCARD LayoutUnit BorderBottom() const noexcept final;

    KRYS_NODISCARD LayoutUnit OuterBorderBefore() const noexcept;
    KRYS_NODISCARD LayoutUnit OuterBorderAfter() const noexcept;
    KRYS_NODISCARD LayoutUnit OuterBorderStart() const noexcept;
    KRYS_NODISCARD LayoutUnit OuterBorderEnd() const noexcept;

    KRYS_NODISCARD LayoutUnit OuterBorderLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit OuterBorderRight() const noexcept;
    KRYS_NODISCARD LayoutUnit OuterBorderTop() const noexcept;
    KRYS_NODISCARD LayoutUnit OuterBorderBottom() const noexcept;

    KRYS_NODISCARD LayoutUnit CalcBorderStart() const noexcept;
    KRYS_NODISCARD LayoutUnit CalcBorderEnd() const noexcept;
    void RecalcBordersInRowDirection() noexcept;

    void ForceSectionsRecalc() noexcept
    {
      SetNeedsSectionRecalc();
      RecalcSections();
    }

    KRYS_NODISCARD const List<ColumnStruct> &Columns() const noexcept
    {
      return _columns;
    }
    KRYS_NODISCARD const List<LayoutUnit> &ColumnPositions() const noexcept
    {
      return _columnPos;
    }
    void SetColumnPosition(uint32 index, LayoutUnit position) noexcept
    {
      // Note that if our horizontal border-spacing changed, our position will change but not
      // our column's width. In practice, horizontal border-spacing won't change often.
      _columnLogicalWidthChanged |= _columnPos[index] != position;
      _columnPos[index] = position;
    }

    KRYS_NODISCARD RenderTableSection *Header() const noexcept;
    KRYS_NODISCARD RenderTableSection *Footer() const noexcept;
    KRYS_NODISCARD RenderTableSection *FirstBody() const noexcept;

    /// @return The top section, or null if the table has no section.
    KRYS_NODISCARD RenderTableSection *TopSection() const noexcept;
    KRYS_NODISCARD RenderTableSection *BottomSection() const noexcept;

    /// @return The top section with rows, or null if the table has no non-empty section.
    KRYS_NODISCARD RenderTableSection *TopNonEmptySection() const noexcept;
    KRYS_NODISCARD RenderTableSection *BottomNonEmptySection() const noexcept;

    KRYS_NODISCARD uint32 LastColumnIndex() const noexcept
    {
      return NumEffCols() - 1;
    }

    void SplitColumn(uint32 position, uint32 firstSpan) noexcept;
    void AppendColumn(uint32 span) noexcept;
    KRYS_NODISCARD uint32 NumEffCols() const noexcept
    {
      return static_cast<uint32>(_columns.size());
    }
    KRYS_NODISCARD uint32 SpanOfEffCol(uint32 effCol) const noexcept
    {
      return _columns[effCol].Span;
    }

    KRYS_NODISCARD uint32 ColToEffCol(uint32 column) const noexcept
    {
      if (!_hasCellColspanThatDeterminesTableWidth)
      {
        return column;
      }

      uint32 effColumn = 0;
      auto numColumns = NumEffCols();
      for (uint32 c = 0; effColumn < numColumns && c + _columns[effColumn].Span - 1 < column; ++effColumn)
      {
        c += _columns[effColumn].Span;
      }
      return effColumn;
    }

    KRYS_NODISCARD uint32 EffColToCol(uint32 effCol) const noexcept
    {
      if (!_hasCellColspanThatDeterminesTableWidth)
      {
        return effCol;
      }

      uint32 c = 0;
      for (uint32 i = 0; i < effCol; i++)
      {
        c += _columns[i].Span;
      }
      return c;
    }

    KRYS_NODISCARD LayoutUnit BorderSpacingInRowDirection() const noexcept
    {
      if (auto effectiveColumnCount = NumEffCols())
      {
        return static_cast<int32>(effectiveColumnCount + 1) * HBorderSpacing();
      }
      return 0_lu;
    }

    // The collapsing border model dissallows paddings on table, which is why we
    // override those functions.
    // See http://www.w3.org/TR/CSS2/tables.html#collapsing-borders
    KRYS_NODISCARD LayoutUnit PaddingTop() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingBottom() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingLeft() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingRight() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingAfter() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingBefore() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingStart() const noexcept override;
    KRYS_NODISCARD LayoutUnit PaddingEnd() const noexcept override;

    KRYS_NODISCARD LayoutUnit BordersPaddingAndSpacingInRowDirection() const noexcept;

    /// @return The first column or column group.
    KRYS_NODISCARD RenderTableCol *FirstColumn() const noexcept;

    KRYS_NODISCARD RenderTableCol *ColElement(uint32 col, bool *startEdge = nullptr,
                                              bool *endEdge = nullptr) const noexcept
    {
      // The common case is to not have columns, make that case fast.
      if (!_hasColElements)
      {
        return nullptr;
      }
      return SlowColElement(col, startEdge, endEdge);
    }

    KRYS_NODISCARD bool NeedsSectionRecalc() const noexcept
    {
      return _needsSectionRecalc;
    }
    void SetNeedsSectionRecalc() noexcept;

    KRYS_NODISCARD RenderTableSection *
      SectionAbove(const RenderTableSection *section,
                   SkipEmptySections skipEmptySections = SkipEmptySections::No) const noexcept;
    KRYS_NODISCARD RenderTableSection *
      SectionBelow(const RenderTableSection *section,
                   SkipEmptySections skipEmptySections = SkipEmptySections::No) const noexcept;

    KRYS_NODISCARD RenderTableCell *CellAbove(const RenderTableCell *cell) const noexcept;
    KRYS_NODISCARD RenderTableCell *CellBelow(const RenderTableCell *cell) const noexcept;
    KRYS_NODISCARD RenderTableCell *CellBefore(const RenderTableCell *cell) const noexcept;
    KRYS_NODISCARD RenderTableCell *CellAfter(const RenderTableCell *cell) const noexcept;

    KRYS_NODISCARD bool CollapsedBordersAreValid() const noexcept
    {
      return _collapsedBordersValid;
    }
    void InvalidateCollapsedBorders(RenderTableCell *cellWithStyleChange = nullptr) noexcept;
    void InvalidateCollapsedBordersAfterStyleChangeIfNeeded(
      const RenderStyle &oldStyle, const RenderStyle &newStyle,
      RenderTableCell *cellWithStyleChange = nullptr) noexcept;
    void SetCollapsedEmptyBorderIsPresent() noexcept
    {
      _collapsedEmptyBorderIsPresent = true;
    }

    KRYS_NODISCARD bool HasSections() const noexcept;

    using CollapsedBorderValues = List<CollapsedBorderValue>;

    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;
    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction action) noexcept override;
    void PaintObject(PaintInfo &, const LayoutPoint &) noexcept override;
    void PaintMask(PaintInfo &, const LayoutPoint &) noexcept override;
    void PaintBoxDecorations(PaintInfo &, const LayoutPoint &) noexcept override;
    void AdjustBorderBoxRectForPainting(LayoutRect &) noexcept override;

    /// @brief The collapsed border cells paint in the current pass of the collapsed borders phase.
    KRYS_NODISCARD const CollapsedBorderValue *CurrentBorderValue() const noexcept
    {
      return _currentBorder;
    }
    /// @brief Collects the cells' distinct collapsed borders, sorted by precedence, if they're out of date.
    void RecalcCollapsedBorders() noexcept;

    void RecalcSectionsIfNeeded() const noexcept
    {
      if (_needsSectionRecalc)
      {
        RecalcSections();
      }
    }

    void AddCaption(RenderTableCaption &caption) noexcept;
    void RemoveCaption(RenderTableCaption &oldCaption) noexcept;
    void AddColumn(const RenderTableCol *column) noexcept;
    void InvalidateColumns() noexcept;

    KRYS_NODISCARD LayoutUnit OffsetTopForColumn(const RenderTableCol &column) const noexcept;
    KRYS_NODISCARD LayoutUnit OffsetLeftForColumn(const RenderTableCol &column) const noexcept;
    KRYS_NODISCARD LayoutUnit OffsetWidthForColumn(const RenderTableCol &column) const noexcept;
    KRYS_NODISCARD LayoutUnit OffsetHeightForColumn(const RenderTableCol &column) const noexcept;

    void WillInsertTableColumn(RenderTableCol &child, RenderObject *beforeChild) noexcept;
    void WillInsertTableSection(RenderTableSection &child, RenderObject *beforeChild) noexcept;

    KRYS_NODISCARD LayoutUnit SumCaptionsLogicalHeight() const noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderTable";
    }

  protected:
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept final;
    void SimplifiedNormalFlowLayout() noexcept final;

    void Layout() noexcept final;
    void ComputeIntrinsicLogicalWidths(LayoutUnit &minWidth, LayoutUnit &maxWidth,
                                       TableIntrinsics intrinsics) const noexcept;
    void ComputeIntrinsicLogicalWidths(LayoutUnit &minWidth, LayoutUnit &maxWidth) const noexcept final;
    void ComputeIntrinsicKeywordLogicalWidths(LayoutUnit &minWidth,
                                              LayoutUnit &maxWidth) const noexcept final;
    void ComputePreferredLogicalWidths() noexcept override;

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept override;
    KRYS_NODISCARD Maybe<LayoutUnit> LastLineBaseline() const noexcept override;

    KRYS_NODISCARD RenderTableCol *SlowColElement(uint32 col, bool *startEdge, bool *endEdge) const noexcept;

    void UpdateColumnCache() const noexcept;
    void InvalidateCachedColumns() noexcept;
    void InvalidateCachedColumnOffsets() noexcept;

    void UpdateLogicalWidth() noexcept final;

    template <typename TSize>
    KRYS_NODISCARD LayoutUnit ConvertStyleLogicalWidthToComputedWidth(const TSize &styleLogicalWidth,
                                                                      LayoutUnit availableWidth) noexcept;
    template <typename TSize>
    KRYS_NODISCARD LayoutUnit
      ConvertStyleLogicalHeightToComputedHeight(const TSize &styleLogicalHeight) noexcept;

    KRYS_NODISCARD LayoutRect
      OverflowClipRect(const LayoutPoint &location,
                       OverlayScrollbarSizeRelevancy relevancy =
                         OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize) const noexcept final;
    KRYS_NODISCARD LayoutRect OverflowClipRectForChildLayers(
      const LayoutPoint &location, OverlayScrollbarSizeRelevancy relevancy) const noexcept override
    {
      return RenderBox::OverflowClipRect(location, relevancy);
    }

    void AddOverflowFromInFlowChildren(EnumSet<ComputeOverflowOptions> options = {}) noexcept final;

    void RecalcSections() const noexcept;

    enum class BottomCaptionLayoutPhase : bool
    {
      No,
      Yes
    };
    void LayoutCaptions(
      BottomCaptionLayoutPhase bottomCaptionLayoutPhase = BottomCaptionLayoutPhase::No) noexcept;
    void LayoutCaption(RenderTableCaption &caption) noexcept;

    void DistributeExtraLogicalHeight(LayoutUnit extraLogicalHeight) noexcept;

    KRYS_NODISCARD uint32 EffectiveIndexOfColumn(const RenderTableCol &column) const noexcept;

    KRYS_NODISCARD bool IsHTMLTableElement() const noexcept;

    KRYS_NODISCARD bool HasCellColspanThatDeterminesTableWidth() const noexcept
    {
      for (uint32 c = 0; c < NumEffCols(); c++)
      {
        if (_columns[c].Span > 1)
        {
          return true;
        }
      }
      return false;
    }

    KRYS_NODISCARD bool ShouldResetLogicalHeightBeforeLayout() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTable)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTable();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
