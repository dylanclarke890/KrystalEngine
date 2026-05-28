#pragma once

#include "Krystal.HTML/DOM/Document.hpp"

namespace Krys::HTML
{
  class XMLDocument : public Document
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(XMLDocument);

  protected:
    XMLDocument(DocumentFlags flags) noexcept : Document(flags | DocumentFlags::IsXMLDocument)
    {
    }

  public:
    XMLDocument() noexcept : Document(DocumentFlags::IsXMLDocument)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::XMLDocument)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Document &document) noexcept
  {
    return document.IsXMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
