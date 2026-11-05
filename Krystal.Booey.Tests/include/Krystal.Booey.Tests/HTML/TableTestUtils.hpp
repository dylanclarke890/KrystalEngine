#pragma once

#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include <string>

namespace krys::boo::html::tests
{
  KRYS_NODISCARD inline std::string TableText(dom::DOMStringView text) noexcept
  {
    return std::string(reinterpret_cast<const char *>(text.data()), text.size());
  }

  /// @brief The ids of @p collection's elements, in order.
  KRYS_NODISCARD inline List<std::string> TableIds(const dom::HTMLCollection &collection) noexcept
  {
    List<std::string> ids;
    for (size_t i = 0; i < collection.Length(); ++i)
    {
      ids.push_back(TableText(collection.Item(i)->Id()));
    }
    return ids;
  }

  /// @brief A document with a body holding a table, whose markup the test sets.
  struct TableDocument
  {
    Ref<dom::HTMLDocument> Document {CreateRef<dom::HTMLDocument>()};
    RefPtr<dom::Element> Body;

    TableDocument() noexcept
    {
      auto html = Document->CreateElement(dom::DOMStringAtom(u8"html")).ReleaseValue();
      (void)Document->AppendChild(*html);
      Body = Document->CreateElement(dom::DOMStringAtom(u8"body")).ReleaseValue();
      (void)html->AppendChild(*Body);
    }

    HTMLTableElement &Table(dom::DOMStringView markup) noexcept
    {
      dom::DOMString tableMarkup(u8"<table id=t>");
      tableMarkup += markup;
      tableMarkup += u8"</table>";
      (void)Body->InnerHTML(tableMarkup);
      return Downcast<HTMLTableElement>(Downcast<HTMLElement>(*Body->FirstElementChild()));
    }

    KRYS_NODISCARD std::string Markup() const noexcept
    {
      return TableText(Body->FirstElementChild()->InnerHTML().ReleaseValue());
    }
  };
}
