/*
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Elements/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLTableSectionElement::HTMLTableSectionElement(dom::Document &document) noexcept
      : HTMLTablePartElement(document, HTMLElementInterface::TableSection)
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
      *this, [](const dom::Element &element) { return AsTableElement<HTMLTableRowElement>(&element); },
      dom::LiveCollectionInvalidation::TreeOnly, dom::LiveCollectionTraversal::Children);
    _rows = CreateWeakPtr(rows.get());
    return rows;
  }

  dom::ExceptionOr<Ref<HTMLTableRowElement>> HTMLTableSectionElement::InsertRow(int32 index) noexcept
  {
    // 1. If index is less than −1 or greater than the number of elements in the rows collection, throw
    //    an "IndexSizeError" DOMException.
    if (index < -1)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }
    auto children = Rows();
    auto numRows = static_cast<int32>(children->Length());
    if (index > numRows)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    // 2. Let table row be the result of creating an element given this element's node document, "tr", and
    //    the HTML namespace.
    auto row = CreateTableElement<HTMLTableRowElement>(NodeDocument(), u8"tr");

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

  dom::ExceptionOr<void> HTMLTableSectionElement::DeleteRow(int32 index) noexcept
  {
    CEReactionsScope ceReactions;

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
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }
    if (auto result = RemoveChild(*children->Item(index)); result.HasException())
    {
      return result.ReleaseException();
    }
    return {};
  }

#pragma endregion

#pragma region HTMLTableSectionElement Obsolete members

  dom::DOMString HTMLTableSectionElement::Align() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableSectionElement::Align(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLTableSectionElement::Char() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableSectionElement::Char(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"char", krys::move(value));
  }

  dom::DOMString HTMLTableSectionElement::CharOff() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableSectionElement::CharOff(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"charoff", krys::move(value));
  }

  dom::DOMString HTMLTableSectionElement::VAlign() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableSectionElement::VAlign(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"valign", krys::move(value));
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
