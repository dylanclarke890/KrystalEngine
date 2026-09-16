#pragma once

#include "Krystal.Booey/DOM/Document.hpp"

namespace krys::boo::dom
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::XMLDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Document &document) noexcept
  {
    return document.IsXMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
