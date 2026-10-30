#include "Krystal.Booey/HTML/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableRowElement.hpp"
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
      *this, [this](const dom::Element &element)
      { return element.ParentNode() == this && AsTableElement<HTMLTableRowElement>(&element); });
    _rows = CreateWeakPtr(rows.get());
    return rows;
  }

  dom::ExceptionOr<Ref<HTMLTableRowElement>> HTMLTableSectionElement::InsertRow(int32 index) noexcept
  {
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

    auto row = CreateTableElement<HTMLTableRowElement>(NodeDocument(), u8"tr");
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
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableSectionElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLTableSectionElement::Char() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableSectionElement::Char(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"char", krys::move(value));
  }

  dom::DOMString HTMLTableSectionElement::CharOff() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableSectionElement::CharOff(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"charoff", krys::move(value));
  }

  dom::DOMString HTMLTableSectionElement::VAlign() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableSectionElement::VAlign(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"valign", krys::move(value));
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
