#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableSection.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  RenderTableRow::RenderTableRow(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBox(Type::TableRow, element, krys::move(style))
  {
    SetInline(false);
    krys_debug_assert(IsRenderTableRow());
  }

  RenderTableRow::RenderTableRow(dom::Document &document, RenderStyle &&style) noexcept
      : RenderBox(Type::TableRow, document, krys::move(style))
  {
    SetInline(false);
    krys_debug_assert(IsRenderTableRow());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableRow::~RenderTableRow() noexcept = default;

  RenderTableRow *RenderTableRow::NextRow() const noexcept
  {
    return Downcast<RenderTableRow>(NextSibling());
  }

  RenderTableRow *RenderTableRow::PreviousRow() const noexcept
  {
    return Downcast<RenderTableRow>(PreviousSibling());
  }

  RenderTableCell *RenderTableRow::FirstCell() const noexcept
  {
    return Downcast<RenderTableCell>(FirstChild());
  }

  RenderTableCell *RenderTableRow::LastCell() const noexcept
  {
    return Downcast<RenderTableCell>(LastChild());
  }

  RenderTable *RenderTableRow::Table() const noexcept
  {
    auto *section = Section();
    if (!section)
    {
      return nullptr;
    }
    return Downcast<RenderTable>(section->Parent());
  }

  RenderTableSection *RenderTableRow::Section() const noexcept
  {
    return Downcast<RenderTableSection>(static_cast<RenderObject *>(Parent()));
  }

  void RenderTableRow::SetRowIndex(uint32 rowIndex) noexcept
  {
    if (rowIndex > MaxRowIndex) KRYS_UNLIKELY
    {
      krys_assert_msg(false, "Too many table rows.");
    }
    _rowIndex = rowIndex;
  }

  void RenderTableRow::WillBeRemovedFromTree() noexcept
  {
    RenderBox::WillBeRemovedFromTree();

    Section()->SetNeedsCellRecalc();
  }

  static bool BorderWidthChanged(const RenderStyle *oldStyle, const RenderStyle *newStyle) noexcept
  {
    return oldStyle->BorderLeftWidth() != newStyle->BorderLeftWidth()
           || oldStyle->BorderTopWidth() != newStyle->BorderTopWidth()
           || oldStyle->BorderRightWidth() != newStyle->BorderRightWidth()
           || oldStyle->BorderBottomWidth() != newStyle->BorderBottomWidth();
  }

  void RenderTableRow::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    krys_debug_assert(Style().Display() == DisplayType::TableRow);

    RenderBox::StyleDidChange(diff, oldStyle);
    PropagateStyleToAnonymousChildren(StylePropagationType::AllChildren);

    if (Section() && oldStyle && Style().LogicalHeight() != oldStyle->LogicalHeight())
    {
      Section()->RowLogicalHeightChanged(RowIndex());
    }

    // If border was changed, notify table.
    if (auto *table = Table())
    {
      if (oldStyle)
      {
        table->InvalidateCollapsedBordersAfterStyleChangeIfNeeded(*oldStyle, Style());
      }

      if (oldStyle && diff == style::DifferenceResult::Layout && NeedsLayout() && table->CollapseBorders()
          && BorderWidthChanged(oldStyle, &Style()))
      {
        // If the border width changes on a row, we need to make sure the cells in the row know to lay out
        // again. This only happens when borders are collapsed, since they end up affecting the border sides
        // of the cell itself.
        auto propagageNeedsLayoutOnBorderSizeChange = [&](RenderTableRow &row)
        {
          for (auto *cell = row.FirstCell(); cell; cell = cell->NextCell())
          {
            cell->SetNeedsLayoutAndPreferredWidthsUpdate();
          }
        };
        propagageNeedsLayoutOnBorderSizeChange(*this);
        if (auto *previousRow = PreviousRow())
        {
          propagageNeedsLayoutOnBorderSizeChange(*previousRow);
        }
        if (auto *nextRow = NextRow())
        {
          propagageNeedsLayoutOnBorderSizeChange(*nextRow);
        }
      }
    }
  }

  const BorderValue &RenderTableRow::BorderAdjoiningTableStart() const noexcept
  {
    return Style().BorderStart(Table()->GetWritingMode());
  }

  const BorderValue &RenderTableRow::BorderAdjoiningTableEnd() const noexcept
  {
    return Style().BorderEnd(Table()->GetWritingMode());
  }

  const BorderValue &
    RenderTableRow::BorderAdjoiningStartCell(KRYS_MAYBE_UNUSED const RenderTableCell &cell) const noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(cell.IsFirstOrLastCellInRow());
#endif
    // FIXME: https://webkit.org/b/79272 - Add support for mixed directionality at the cell level.
    return Style().BorderStart(Table()->GetWritingMode());
  }

  const BorderValue &
    RenderTableRow::BorderAdjoiningEndCell(KRYS_MAYBE_UNUSED const RenderTableCell &cell) const noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(cell.IsFirstOrLastCellInRow());
#endif
    // FIXME: https://webkit.org/b/79272 - Add support for mixed directionality at the cell level.
    return Style().BorderEnd(Table()->GetWritingMode());
  }

  void RenderTableRow::DidInsertTableCell(RenderTableCell &child, RenderObject *beforeChild) noexcept
  {
    // Generated content can result in us having a null section so make sure to null check our parent.
    if (auto *section = Section())
    {
      section->AddCell(&child, this);
      if (beforeChild || NextRow())
      {
        section->SetNeedsCellRecalc();
      }
    }

    if (auto *table = Table())
    {
      table->InvalidateCollapsedBorders();
    }
  }

  void RenderTableRow::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());

    // Table rows do not add translation.
    // There are no reflections (-webkit-box-reflect was dropped).
    LayoutStateMaintainer statePusher(*this, LayoutSize(),
                                      IsTransformed() || GetWritingMode().IsBlockFlipped());

    // FIXME: Lay out cells whose page offset changed (isPaginated, pageLogicalHeightChanged), once there's
    // pagination.
    for (auto *cell = FirstCell(); cell; cell = cell->NextCell())
    {
      if (cell->NeedsLayout())
      {
        cell->Layout();
      }
    }

    ClearOverflow();
    AddVisualEffectOverflow();
    // FIXME: Repaint the cells when the row itself needed layout (checkForRepaintDuringLayout), once there's
    // painting.

    // RenderTableSection::LayoutRows will set our logical height and width later, so it calls
    // UpdateLayerTransform().
    ClearNeedsLayout();
  }
}
