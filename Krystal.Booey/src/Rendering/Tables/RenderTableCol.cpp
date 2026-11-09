/*
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include "Krystal.Booey/Rendering/Tables/RenderTableCol.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableColElement.hpp"
#include "Krystal.Booey/Rendering/Iterators/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTable.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableSection.hpp"

namespace krys::boo
{
  RenderTableCol::RenderTableCol(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBox(Type::TableCol, element, krys::move(style))
  {
    // init RenderObject attributes
    SetInline(true); // our object is not Inline
    UpdateFromElement();
    krys_debug_assert(IsRenderTableCol());
  }

  RenderTableCol::RenderTableCol(dom::Document &document, RenderStyle &&style) noexcept
      : RenderBox(Type::TableCol, document, krys::move(style))
  {
    SetInline(true);
    krys_debug_assert(IsRenderTableCol());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableCol::~RenderTableCol() noexcept = default;

  void RenderTableCol::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBox::StyleDidChange(diff, oldStyle);
    auto *table = Table();
    if (!table)
    {
      return;
    }

    // If border was changed, notify table.
    if (!oldStyle)
    {
      return;
    }

    table->InvalidateCollapsedBordersAfterStyleChangeIfNeeded(*oldStyle, Style());
    if (oldStyle->Width() != Style().Width())
    {
      table->RecalcSectionsIfNeeded();
      for (auto &section : ChildrenOfType<RenderTableSection>(*table))
      {
        auto nEffCols = table->NumEffCols();
        for (uint32 j = 0; j < nEffCols; j++)
        {
          auto rowCount = section.NumRows();
          for (uint32 i = 0; i < rowCount; i++)
          {
            auto *cell = section.PrimaryCellAt(i, j);
            if (!cell)
            {
              continue;
            }
            cell->SetNeedsPreferredWidthsUpdate();
          }
        }
      }
    }
  }

  void RenderTableCol::UpdateFromElement() noexcept
  {
    krys_debug_assert(GetElement());
    auto oldSpan = _span;
    auto *htmlElement = DynamicDowncast<html::HTMLElement>(GetElement());
    if (auto *tableColElement =
          htmlElement ? DynamicDowncast<html::HTMLTableColElement>(*htmlElement) : nullptr)
    {
      _span = tableColElement->Span();
    }
    else
    {
      _span = 1;
    }

    if (_span != oldSpan && Parent())
    {
      if (HasInitializedStyle())
      {
        SetNeedsLayoutAndPreferredWidthsUpdate();
      }
      if (auto *table = Table())
      {
        table->InvalidateColumns();
      }
    }
  }

  void RenderTableCol::InsertedIntoTree() noexcept
  {
    RenderBox::InsertedIntoTree();
    Table()->AddColumn(this);
  }

  void RenderTableCol::WillBeRemovedFromTree() noexcept
  {
    RenderBox::WillBeRemovedFromTree();
    if (auto *table = Table())
    {
      // We only need to invalidate the column cache when only individual columns are being removed (as
      // opposed to when the entire table is being collapsed).
      table->InvalidateColumns();
    }
  }

  bool RenderTableCol::IsTableColumn() const noexcept
  {
    return Style().Display() == DisplayType::TableColumn;
  }

  bool RenderTableCol::IsTableColumnGroup() const noexcept
  {
    return Style().Display() == DisplayType::TableColumnGroup;
  }

  bool RenderTableCol::IsChildAllowed(const RenderObject &child, const RenderStyle &style) const noexcept
  {
    // We cannot use isTableColumn here as style() may return 0.
    return style.Display() == DisplayType::TableColumn && child.IsRenderTableCol();
  }

  bool RenderTableCol::CanHaveChildren() const noexcept
  {
    // Cols cannot have children. This is actually necessary to fix a bug
    // with libraries.uc.edu, which makes a <p> be a table-column.
    return IsTableColumnGroup();
  }

  void RenderTableCol::ClearNeedsPreferredLogicalWidthsUpdate() noexcept
  {
    ClearNeedsPreferredWidthsUpdate();

    for (auto &child : ChildrenOfType<RenderObject>(*this))
    {
      child.ClearNeedsPreferredWidthsUpdate();
    }
  }

  RenderTable *RenderTableCol::Table() const noexcept
  {
    auto *table = Parent();
    if (table && !Is<RenderTable>(*table))
    {
      table = table->Parent();
    }
    return DynamicDowncast<RenderTable>(table);
  }

  RenderTableCol *RenderTableCol::EnclosingColumnGroup() const noexcept
  {
    auto *parentColumnGroup = DynamicDowncast<RenderTableCol>(*Parent());
    if (!parentColumnGroup)
    {
      return nullptr;
    }

    krys_debug_assert(parentColumnGroup->IsTableColumnGroup());
    krys_debug_assert(IsTableColumn());
    return parentColumnGroup;
  }

  RenderTableCol *RenderTableCol::NextColumn() const noexcept
  {
    // If |this| is a column-group, the next column is the colgroup's first child column.
    if (auto *firstChild = FirstChild())
    {
      return Downcast<RenderTableCol>(firstChild);
    }

    // Otherwise it's the next column along.
    auto *next = NextSibling();

    // Failing that, the child is the last column in a column-group, so the next column is the next
    // column/column-group after its column-group.
    if (!next && Is<RenderTableCol>(*Parent()))
    {
      next = Parent()->NextSibling();
    }

    for (; next; next = next->NextSibling())
    {
      if (auto *column = DynamicDowncast<RenderTableCol>(*next))
      {
        return column;
      }
    }

    return nullptr;
  }

  const BorderValue &RenderTableCol::BorderAdjoiningCellStartBorder() const noexcept
  {
    return Style().BorderStart(Table()->GetWritingMode());
  }

  const BorderValue &RenderTableCol::BorderAdjoiningCellEndBorder() const noexcept
  {
    return Style().BorderEnd(Table()->GetWritingMode());
  }

  const BorderValue &
    RenderTableCol::BorderAdjoiningCellBefore(KRYS_MAYBE_UNUSED const RenderTableCell &cell) const noexcept
  {
    auto *table = Table();
    krys_debug_assert(table->ColElement(cell.Col() + cell.ColSpan()) == this);
    return Style().BorderStart(table->GetWritingMode());
  }

  const BorderValue &
    RenderTableCol::BorderAdjoiningCellAfter(KRYS_MAYBE_UNUSED const RenderTableCell &cell) const noexcept
  {
    auto *table = Table();
    krys_debug_assert(table->ColElement(cell.Col() - 1) == this);
    return Style().BorderEnd(table->GetWritingMode());
  }

  LayoutUnit RenderTableCol::OffsetLeft() const noexcept
  {
    return Table()->OffsetLeftForColumn(*this);
  }

  LayoutUnit RenderTableCol::OffsetTop() const noexcept
  {
    return Table()->OffsetTopForColumn(*this);
  }

  LayoutUnit RenderTableCol::OffsetWidth() const noexcept
  {
    return Table()->OffsetWidthForColumn(*this);
  }

  LayoutUnit RenderTableCol::OffsetHeight() const noexcept
  {
    return Table()->OffsetHeightForColumn(*this);
  }

  LayoutRect RenderTableCol::ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                 VisibleRectContext context) const noexcept
  {
    // For now, just repaint the whole table.
    // FIXME: Find a better way to do this, e.g., need to repaint all the cells that we
    // might have propagated a background color or borders into.
    // FIXME: check for repaintContainer each time here?

    auto *parentTable = Table();
    if (!parentTable)
    {
      return {};
    }

    return parentTable->ClippedOverflowRect(repaintContainer, context);
  }

  auto RenderTableCol::RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                     RepaintOutlineBounds) const noexcept -> RepaintRects
  {
    // Ignore RepaintOutlineBounds because it doesn't make sense to use the table's outline bounds to repaint
    // a column.
    return RepaintRects {ClippedOverflowRect(repaintContainer, VisibleRectContextForRepaint())};
  }

  void RenderTableCol::ImageChanged(WrappedImagePtr, const IntRect *) noexcept
  {
    // FIXME: Repaint only the rect the image paints in.
    if (!Parent())
    {
      return;
    }
    Repaint();
  }
}
