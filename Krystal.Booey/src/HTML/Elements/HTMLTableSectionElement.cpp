#include "Krystal.Booey/HTML/Elements/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo
{
  HTMLTableSectionElement::HTMLTableSectionElement(Document &document) noexcept
      : HTMLTablePartElement(document, html::HTMLElementInterface::TableSection)
  {
  }

#pragma region HTMLTableSectionElement

  Ref<dom::HTMLCollection> HTMLTableSectionElement::Rows() noexcept
  {
    if (auto *rows = _rows.get())
    {
      return ShareRef(*rows);
    }

    Ref<dom::HTMLCollection> rows = CreateRef<dom::LiveHTMLCollection>(
      *this, [](const Element &element) { return html::AsTableElement<HTMLTableRowElement>(&element); },
      dom::LiveCollectionInvalidation::TreeOnly, dom::LiveCollectionTraversal::Children);
    _rows = CreateWeakPtr(rows.get());
    return rows;
  }

  ExceptionOr<Ref<HTMLTableRowElement>> HTMLTableSectionElement::InsertRow(int32 index) noexcept
  {
    // 1. If index is less than −1 or greater than the number of elements in the rows collection, throw
    //    an "IndexSizeError" DOMException.
    if (index < -1)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }
    auto children = Rows();
    auto numRows = static_cast<int32>(children->Length());
    if (index > numRows)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    // 2. Let table row be the result of creating an element given this element's node document, "tr", and
    //    the HTML namespace.
    auto row = html::CreateTableElement<HTMLTableRowElement>(NodeDocument(), u8"tr");

    // 3. If index is −1 or equal to the number of items in the rows collection, then append table row to
    //    this element.
    // 4. Otherwise, insert table row as a child of this element, immediately before the indexth element in
    //    the rows collection.
    auto result =
      numRows == index || index == -1 ? AppendChild(*row) : InsertBefore(*row, children->Item(index).get());
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    return row;
  }

  ExceptionOr<void> HTMLTableSectionElement::DeleteRow(int32 index) noexcept
  {
    html::CEReactionsScope ceReactions;

    // 1. If index is less than −1 or greater than or equal to the number of elements in the rows
    //    collection, then throw an "IndexSizeError" DOMException.
    // 2. If index is −1, then remove the last element in the rows collection from this element, or do
    //    nothing if the rows collection is empty.
    // 3. Otherwise, remove the indexth element in the rows collection from this element.
    auto children = Rows();
    auto numRows = static_cast<int32>(children->Length());
    if (index == -1)
    {
      if (!numRows)
      {
        return {};
      }
      index = numRows - 1;
    }
    if (index < 0 || index >= numRows)
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

#pragma region HTMLTableSectionElement Obsolete members

  DOMString HTMLTableSectionElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableSectionElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

  DOMString HTMLTableSectionElement::Char() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableSectionElement::Char(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"char", krys::move(value));
  }

  DOMString HTMLTableSectionElement::CharOff() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableSectionElement::CharOff(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"charoff", krys::move(value));
  }

  DOMString HTMLTableSectionElement::VAlign() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableSectionElement::VAlign(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"valign", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  const css::MutableStyleProperties *
    HTMLTableSectionElement::AdditionalPresentationalHintStyle() const noexcept
  {
    const auto *table = FindParentTable();
    if (!table)
    {
      return nullptr;
    }
    return table->AdditionalGroupStyle(true);
  }

#pragma endregion
}
