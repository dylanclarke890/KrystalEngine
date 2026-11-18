#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Collections/HTMLTableRowsCollection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableCellElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableSectionElement.hpp"

namespace krys::boo
{
  HTMLTableRowElement::HTMLTableRowElement(Document &document) noexcept
      : HTMLTablePartElement(document, html::HTMLElementInterface::TableRow)
  {
  }

#pragma region HTMLTableRowElement

  namespace
  {
    // The row's index among its section's or table's rows, counted with next.
    template <typename TNext>
    KRYS_NODISCARD int32 IndexOfRow(const HTMLTableRowElement &row, HTMLTableRowElement *first,
                                    TNext next) noexcept
    {
      int32 index = 0;
      for (auto *current = first; current; current = next(current))
      {
        if (current == &row)
        {
          return index;
        }
        ++index;
      }
      return -1;
    }

    KRYS_NODISCARD HTMLTableElement *TableOfRow(const HTMLTableRowElement &row) noexcept
    {
      auto *parent = row.ParentNode();
      if (auto *table = html::AsTableElement<HTMLTableElement>(parent))
      {
        return const_cast<HTMLTableElement *>(table);
      }
      if (html::AsTableElement<HTMLTableSectionElement>(parent))
      {
        if (auto *table = html::AsTableElement<HTMLTableElement>(parent->ParentNode()))
        {
          return const_cast<HTMLTableElement *>(table);
        }
      }
      return nullptr;
    }
  }

  int32 HTMLTableRowElement::RowIndex() const noexcept
  {
    auto *table = TableOfRow(*this);
    if (!table)
    {
      return -1;
    }
    return IndexOfRow(*this, html::HTMLTableRowsCollection::RowAfter(*table, nullptr),
                      [table](HTMLTableRowElement *row)
                      { return html::HTMLTableRowsCollection::RowAfter(*table, row); });
  }

  int32 HTMLTableRowElement::SectionRowIndex() const noexcept
  {
    auto *parent = ParentNode();
    if (html::AsTableElement<HTMLTableSectionElement>(parent))
    {
      int32 index = 0;
      for (auto *sibling = PreviousSibling(); sibling; sibling = sibling->PreviousSibling())
      {
        if (html::AsTableElement<HTMLTableRowElement>(sibling))
        {
          ++index;
        }
      }
      return index;
    }
    if (auto *table = html::AsTableElement<HTMLTableElement>(parent))
    {
      auto &mutableTable = const_cast<HTMLTableElement &>(*table);
      return IndexOfRow(*this, html::HTMLTableRowsCollection::RowAfter(mutableTable, nullptr),
                        [&mutableTable](HTMLTableRowElement *row)
                        { return html::HTMLTableRowsCollection::RowAfter(mutableTable, row); });
    }
    return -1;
  }

  Ref<dom::HTMLCollection> HTMLTableRowElement::Cells() noexcept
  {
    if (auto *cells = _cells.get())
    {
      return ShareRef(*cells);
    }

    Ref<dom::HTMLCollection> cells = CreateRef<dom::LiveHTMLCollection>(
      *this, [](const Element &element) { return html::AsTableElement<HTMLTableCellElement>(&element); },
      dom::LiveCollectionInvalidation::TreeOnly, dom::LiveCollectionTraversal::Children);
    _cells = CreateWeakPtr(cells.get());
    return cells;
  }

  ExceptionOr<Ref<HTMLTableCellElement>> HTMLTableRowElement::InsertCell(int32 index) noexcept
  {
    // 1. If index is less than −1 or greater than the number of elements in the cells collection, throw
    //    an "IndexSizeError" DOMException.
    if (index < -1)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }
    auto children = Cells();
    auto numCells = static_cast<int32>(children->Length());
    if (index > numCells)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    // 2. Let table cell be the result of creating an element given this element's node document, "td", and
    //    the HTML namespace.
    auto cell = html::CreateTableElement<HTMLTableCellElement>(NodeDocument(), u8"td");

    // 3. If index is −1 or equal to the number of items in the cells collection, then append table cell to
    //    this element.
    // 4. Otherwise, insert table cell as a child of this element, immediately before the indexth element in
    //    the cells collection.
    auto result = numCells == index || index == -1 ? AppendChild(*cell)
                                                   : InsertBefore(*cell, children->Item(index).get());
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    return cell;
  }

  ExceptionOr<void> HTMLTableRowElement::DeleteCell(int32 index) noexcept
  {
    html::CEReactionsScope ceReactions;

    // 1. If index is less than −1 or greater than or equal to the number of elements in the cells
    //    collection, then throw an "IndexSizeError" DOMException.
    // 2. If index is −1, then remove the last element in the cells collection from this element, or do
    //    nothing if the cells collection is empty.
    // 3. Otherwise, remove the indexth element in the cells collection from this element.
    auto children = Cells();
    auto numCells = static_cast<int32>(children->Length());
    if (index == -1)
    {
      if (!numCells)
      {
        return {};
      }
      index = numCells - 1;
    }
    if (index < 0 || index >= numCells)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }
    if (auto result = RemoveChild(*children->Item(index)); result.HasException())
    {
      return result.ReleaseException();
    }
    return {};
  }

#pragma endregion

#pragma region HTMLTableRowElement Obsolete members

  DOMString HTMLTableRowElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableRowElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

  DOMString HTMLTableRowElement::Char() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableRowElement::Char(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"char", krys::move(value));
  }

  DOMString HTMLTableRowElement::CharOff() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableRowElement::CharOff(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"charoff", krys::move(value));
  }

  DOMString HTMLTableRowElement::VAlign() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableRowElement::VAlign(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"valign", krys::move(value));
  }

  DOMString HTMLTableRowElement::BgColor() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLTableRowElement::BgColor(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"bgcolor", krys::move(value));
  }

#pragma endregion
}
