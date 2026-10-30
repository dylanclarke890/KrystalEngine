#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderTable.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCaption.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableCol.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderTableSection.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderTreeBuilder::Table::Table(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  RenderElement &RenderTreeBuilder::Table::FindOrCreateParentForChild(RenderTableRow &parent,
                                                                      const RenderObject &child,
                                                                      RenderObject *&beforeChild) noexcept
  {
    if (Is<RenderTableCell>(child))
    {
      return parent;
    }

    if (beforeChild && !beforeChild->IsAnonymous() && beforeChild->Parent() == &parent)
    {
      auto *previousSibling = beforeChild->PreviousSibling();
      if (auto *tableCell = DynamicDowncast<RenderTableCell>(previousSibling);
          tableCell && tableCell->IsAnonymous())
      {
        beforeChild = nullptr;
        return *tableCell;
      }
    }

    auto createAnonymousTableCell = [&](auto &parent) -> RenderTableCell &
    {
      auto newCell = CreateAnonymousTableCellWithStyle(parent.GetDocument(), parent.Style());
      auto &cell = *newCell;
      _builder.Attach(parent, krys::move(newCell), beforeChild);
      beforeChild = nullptr;
      return cell;
    };

    RenderObject *lastChild = beforeChild ? beforeChild : parent.LastCell();
    if (lastChild)
    {
      if (auto *tableCell = DynamicDowncast<RenderTableCell>(*lastChild);
          tableCell && tableCell->IsAnonymous() && !tableCell->IsBeforeOrAfterContent())
      {
        if (beforeChild == lastChild)
        {
          beforeChild = tableCell->FirstChild();
        }
        return *tableCell;
      }

      // Try to find an anonymous container for the child.
      if (auto *lastChildParent = lastChild->Parent())
      {
        if (lastChildParent->IsAnonymous() && !lastChildParent->IsBeforeOrAfterContent())
        {
          // If beforeChild is inside an anonymous COLGROUP, create a cell for the new renderer.
          if (Is<RenderTableCol>(*lastChildParent))
          {
            return createAnonymousTableCell(parent);
          }
          // If beforeChild is inside an anonymous cell, insert into the cell.
          if (!Is<RenderTableCell>(*lastChild))
          {
            return *lastChildParent;
          }
          // If beforeChild is inside an anonymous row, insert into the row.
          if (auto *tableRow = DynamicDowncast<RenderTableRow>(*lastChildParent))
          {
            return createAnonymousTableCell(*tableRow);
          }
        }
      }
    }
    return createAnonymousTableCell(parent);
  }

  RenderElement &RenderTreeBuilder::Table::FindOrCreateParentForChild(RenderTableSection &parent,
                                                                      const RenderObject &child,
                                                                      RenderObject *&beforeChild) noexcept
  {
    if (Is<RenderTableRow>(child))
    {
      return parent;
    }

    RenderObject *lastChild = beforeChild ? beforeChild : parent.LastRow();
    if (auto *tableRow = DynamicDowncast<RenderTableRow>(lastChild);
        tableRow && tableRow->IsAnonymous() && !tableRow->IsBeforeOrAfterContent())
    {
      if (beforeChild == lastChild)
      {
        beforeChild = tableRow->FirstCell();
      }
      return *tableRow;
    }

    if (beforeChild && !beforeChild->IsAnonymous() && beforeChild->Parent() == &parent)
    {
      auto *row = beforeChild->PreviousSibling();
      if (auto *tableRow = DynamicDowncast<RenderTableRow>(row); tableRow && tableRow->IsAnonymous())
      {
        beforeChild = nullptr;
        return *tableRow;
      }
    }

    // If beforeChild is inside an anonymous cell/row, insert into the cell or into
    // the anonymous row containing it, if there is one.
    auto *parentCandidate = lastChild;
    while (parentCandidate && parentCandidate->Parent() && parentCandidate->Parent()->IsAnonymous()
           && !Is<RenderTableRow>(*parentCandidate))
    {
      parentCandidate = parentCandidate->Parent();
    }
    if (auto *tableRow = DynamicDowncast<RenderTableRow>(parentCandidate);
        tableRow && tableRow->IsAnonymous() && !tableRow->IsBeforeOrAfterContent())
    {
      return *tableRow;
    }

    auto newRow = CreateAnonymousTableRowWithStyle(parent.GetDocument(), parent.Style());
    auto &row = *newRow;
    _builder.Attach(parent, krys::move(newRow), beforeChild);
    beforeChild = nullptr;
    return row;
  }

  RenderElement &RenderTreeBuilder::Table::FindOrCreateParentForChild(RenderTable &parent,
                                                                      const RenderObject &child,
                                                                      RenderObject *&beforeChild) noexcept
  {
    if (Is<RenderTableCaption>(child) || Is<RenderTableSection>(child))
    {
      return parent;
    }

    if (auto *tableColumn = DynamicDowncast<RenderTableCol>(child))
    {
      if (!tableColumn->GetElement() || tableColumn->Style().Display() == DisplayType::TableColumnGroup)
      {
        // COLGROUPs and anonymous RenderTableCols (generated wrappers for COLs) are direct children of the
        // table renderer.
        return parent;
      }
      auto newColGroup = CreateRenderer<RenderTableCol>(
        parent.GetDocument(),
        RenderStyle::CreateAnonymousStyleWithDisplay(parent.Style(), DisplayType::TableColumnGroup));
      newColGroup->InitializeStyle();
      auto &colGroup = *newColGroup;
      _builder.Attach(parent, krys::move(newColGroup), beforeChild);
      beforeChild = nullptr;
      return colGroup;
    }

    auto *lastChild = parent.LastChild();
    if (auto *tableSection = DynamicDowncast<RenderTableSection>(lastChild);
        !beforeChild && tableSection && tableSection->IsAnonymous() && !tableSection->IsBeforeContent())
    {
      return *tableSection;
    }

    if (beforeChild && !beforeChild->IsAnonymous() && beforeChild->Parent() == &parent)
    {
      auto *section = beforeChild->PreviousSibling();
      if (auto *tableSection = DynamicDowncast<RenderTableSection>(section);
          tableSection && tableSection->IsAnonymous())
      {
        beforeChild = nullptr;
        return *tableSection;
      }
    }

    auto *parentCandidate = beforeChild;
    while (parentCandidate && parentCandidate->Parent()->IsAnonymous()
           && !Is<RenderTableSection>(*parentCandidate)
           && parentCandidate->Style().Display() != DisplayType::TableCaption
           && parentCandidate->Style().Display() != DisplayType::TableColumnGroup)
    {
      parentCandidate = parentCandidate->Parent();
    }

    if (parentCandidate)
    {
      if (beforeChild && !beforeChild->IsAnonymous() && parentCandidate->Parent() == &parent)
      {
        auto *section = parentCandidate->PreviousSibling();
        if (auto *tableSection = DynamicDowncast<RenderTableSection>(section);
            tableSection && tableSection->IsAnonymous())
        {
          beforeChild = nullptr;
          return *tableSection;
        }
      }

      if (auto *parentTableSection = DynamicDowncast<RenderTableSection>(*parentCandidate);
          parentTableSection && parentTableSection->IsAnonymous()
          && !RenderElement::IsAfterContent(parentTableSection))
      {
        if (beforeChild == parentCandidate)
        {
          beforeChild = parentTableSection->FirstRow();
        }
        return *parentTableSection;
      }
    }

    if (beforeChild && !Is<RenderTableSection>(*beforeChild)
        && beforeChild->Style().Display() != DisplayType::TableCaption
        && beforeChild->Style().Display() != DisplayType::TableColumnGroup)
    {
      beforeChild = nullptr;
    }

    auto newSection = CreateAnonymousTableSectionWithStyle(parent.GetDocument(), parent.Style());
    auto &section = *newSection;
    _builder.Attach(parent, krys::move(newSection), beforeChild);
    beforeChild = nullptr;
    return section;
  }

  void RenderTreeBuilder::Table::Attach(RenderTableRow &parent, RenderPtr<RenderObject> child,
                                        RenderObject *beforeChild) noexcept
  {
    if (beforeChild && beforeChild->Parent() != &parent)
    {
      beforeChild = _builder.SplitAnonymousBoxesAroundChild(parent, *beforeChild);
    }

    auto &newChild = *child;
    krys_debug_assert(!beforeChild || Is<RenderTableCell>(*beforeChild));
    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
    // FIXME: child should always be a RenderTableCell at this point.
    if (auto *renderTableCell = DynamicDowncast<RenderTableCell>(newChild))
    {
      parent.DidInsertTableCell(*renderTableCell, beforeChild);
    }
  }

  void RenderTreeBuilder::Table::Attach(RenderTableSection &parent, RenderPtr<RenderObject> child,
                                        RenderObject *beforeChild) noexcept
  {
    if (beforeChild && beforeChild->Parent() != &parent)
    {
      beforeChild = _builder.SplitAnonymousBoxesAroundChild(parent, *beforeChild);
    }

    // FIXME: child should always be a RenderTableRow at this point.
    if (auto *renderTableRow = DynamicDowncast<RenderTableRow>(child.get()))
    {
      parent.WillInsertTableRow(*renderTableRow, beforeChild);
    }
    krys_debug_assert(!beforeChild || Is<RenderTableRow>(*beforeChild));
    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::Table::Attach(RenderTable &parent, RenderPtr<RenderObject> child,
                                        RenderObject *beforeChild) noexcept
  {
    if (beforeChild && beforeChild->Parent() != &parent)
    {
      beforeChild = _builder.SplitAnonymousBoxesAroundChild(parent, *beforeChild);
    }

    auto &newChild = *child;
    if (auto *renderTableSection = DynamicDowncast<RenderTableSection>(newChild))
    {
      parent.WillInsertTableSection(*renderTableSection, beforeChild);
    }
    else if (auto *renderTableCol = DynamicDowncast<RenderTableCol>(newChild))
    {
      parent.WillInsertTableColumn(*renderTableCol, beforeChild);
    }

    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
  }

  bool RenderTreeBuilder::Table::ChildRequiresTable(const RenderElement &parent,
                                                    const RenderObject &child) noexcept
  {
    if (auto *newTableColumn = DynamicDowncast<RenderTableCol>(child))
    {
      bool isColumnInColumnGroup = newTableColumn->IsTableColumn() && Is<RenderTableCol>(parent);
      return !Is<RenderTable>(parent) && !isColumnInColumnGroup;
    }
    if (Is<RenderTableCaption>(child))
    {
      return !Is<RenderTable>(parent);
    }

    if (Is<RenderTableSection>(child))
    {
      return !Is<RenderTable>(parent);
    }

    if (Is<RenderTableRow>(child))
    {
      return !Is<RenderTableSection>(parent);
    }

    if (Is<RenderTableCell>(child))
    {
      return !Is<RenderTableRow>(parent);
    }

    return false;
  }

  KRYS_NODISCARD static bool CanCollapseNextSibling(const RenderBox &previousSibling,
                                                    const RenderBox &nextSibling) noexcept
  {
    if (!previousSibling.IsAnonymous() || !nextSibling.IsAnonymous())
    {
      return false;
    }
    auto *previousSiblingFirstInFlowChild = previousSibling.FirstInFlowChild();
    auto *nextSiblingFirstInFlowChild = nextSibling.FirstInFlowChild();
    // Do not try to collapse and move inline level boxes over to a container with block level boxes (and vice
    // versa).
    return !previousSiblingFirstInFlowChild || !nextSiblingFirstInFlowChild
           || previousSiblingFirstInFlowChild->IsInline() == nextSiblingFirstInFlowChild->IsInline();
  }

  template <typename Parent, typename Child>
  RenderPtr<RenderObject>
    RenderTreeBuilder::Table::CollapseAndDetachAnonymousNextSibling(Parent *parent, Child *previousSibling,
                                                                    Child *nextSibling) noexcept
  {
    if (!parent || !previousSibling || !nextSibling)
    {
      return {};
    }
    if (!CanCollapseNextSibling(*previousSibling, *nextSibling))
    {
      return {};
    }
    _builder.MoveAllChildren(*nextSibling, *previousSibling, NormalizeAfterInsertion::No);
    previousSibling->SetChildrenInline(!previousSibling->FirstInFlowChild()
                                       || previousSibling->FirstInFlowChild()->IsInline());
    return _builder.Detach(*parent, *nextSibling, WillBeDestroyed::Yes);
  }

  void RenderTreeBuilder::Table::CollapseAndDestroyAnonymousSiblingCells(
    const RenderTableCell &willBeDestroyed) noexcept
  {
    if (auto nextCellToDestroy = CollapseAndDetachAnonymousNextSibling(
          willBeDestroyed.Row(), willBeDestroyed.PreviousCell(), willBeDestroyed.NextCell()))
    {
      Downcast<RenderTableCell>(*nextCellToDestroy)
        .InvalidateLineLayout(RenderBlockFlow::InvalidationReason::InternalMove);
    }
  }

  void RenderTreeBuilder::Table::CollapseAndDestroyAnonymousSiblingRows(
    const RenderTableRow &willBeDestroyed) noexcept
  {
    auto toDestroy = CollapseAndDetachAnonymousNextSibling(
      willBeDestroyed.Section(), willBeDestroyed.PreviousRow(), willBeDestroyed.NextRow());
  }

  RenderPtr<RenderTable>
    RenderTreeBuilder::Table::CreateAnonymousTableWithStyle(dom::Document &document,
                                                            const RenderStyle &style) noexcept
  {
    auto table = CreateRenderer<RenderTable>(
      RenderObject::Type::Table, document,
      RenderStyle::CreateAnonymousStyleWithDisplay(
        style, style.Display() == DisplayType::Inline ? DisplayType::InlineTable : DisplayType::Table));
    table->InitializeStyle();
    return table;
  }

  RenderPtr<RenderTableCell>
    RenderTreeBuilder::Table::CreateAnonymousTableCellWithStyle(dom::Document &document,
                                                                const RenderStyle &style) noexcept
  {
    auto cell = CreateRenderer<RenderTableCell>(
      document, RenderStyle::CreateAnonymousStyleWithDisplay(style, DisplayType::TableCell));
    cell->InitializeStyle();
    return cell;
  }

  RenderPtr<RenderTableRow>
    RenderTreeBuilder::Table::CreateAnonymousTableRowWithStyle(dom::Document &document,
                                                               const RenderStyle &style) noexcept
  {
    auto row = CreateRenderer<RenderTableRow>(
      document, RenderStyle::CreateAnonymousStyleWithDisplay(style, DisplayType::TableRow));
    row->InitializeStyle();
    return row;
  }

  RenderPtr<RenderTableSection>
    RenderTreeBuilder::Table::CreateAnonymousTableSectionWithStyle(dom::Document &document,
                                                                   const RenderStyle &style) noexcept
  {
    auto section = CreateRenderer<RenderTableSection>(
      document, RenderStyle::CreateAnonymousStyleWithDisplay(style, DisplayType::TableRowGroup));
    section->InitializeStyle();
    return section;
  }
}
