#pragma once

#include "Krystal.Web/Dom/Nodes/Document.hpp"

namespace krys::web
{
  class XMLDocument : public Document
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(XMLDocument);

  protected:
    XMLDocument(dom::DocumentFlags flags) noexcept : Document(flags | dom::DocumentFlag::IsXMLDocument)
    {
    }

  public:
    XMLDocument() noexcept : Document(dom::DocumentFlag::IsXMLDocument)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::XMLDocument)
  KRYS_NODISCARD static bool IsType(const krys::web::Document &document) noexcept
  {
    return document.IsXMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
