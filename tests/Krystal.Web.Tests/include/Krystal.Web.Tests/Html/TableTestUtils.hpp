#pragma once

#include "Krystal.Web/Dom/Collections/HTMLCollection.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Html/Elements/HTMLTableElement.hpp"
#include <string>

namespace krys::web::html::tests
{
  KRYS_NODISCARD inline std::string TableText(DOMStringView text) noexcept
  {
    return std::string(reinterpret_cast<const char *>(text.data()), text.size());
  }

  /// @brief The ids of @p collection's elements, in order.
  KRYS_NODISCARD inline List<std::string> TableIds(const HTMLCollection &collection) noexcept
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
    Ref<HTMLDocument> Document {CreateRef<HTMLDocument>()};
    RefPtr<Element> Body;

    TableDocument() noexcept
    {
      auto html = Document->CreateElement(u8"html").ReleaseValue();
      (void)Document->AppendChild(*html);
      Body = Document->CreateElement(u8"body").ReleaseValue();
      (void)html->AppendChild(*Body);
    }

    HTMLTableElement &Table(DOMStringView markup) const noexcept
    {
      DOMString tableMarkup(u8"<table id=t>");
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
